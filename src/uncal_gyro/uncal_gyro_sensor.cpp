/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <common.h>
#include <sf_common.h>
#include <uncal_gyro_sensor.h>
#include <sensor_plugin_loader.h>
#include <orientation_filter.h>
#include <cvirtual_sensor_config.h>

#define SENSOR_NAME "UNCAL_GYROSCOPE_SENSOR"
#define SENSOR_TYPE_UNCAL_GYRO		"UNCAL_GYROSCOPE"

#define GYROSCOPE_ENABLED 0x01
#define GYRO_BIAS_ENABLED 0x02
#define UNCAL_GYRO_BIAS_ENABLED 3

#define INITIAL_VALUE -1

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define PI 3.141593
#define AZIMUTH_OFFSET_DEGREES 360
#define AZIMUTH_OFFSET_RADIANS (2 * PI)

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"

uncal_gyro_sensor::uncal_gyro_sensor()
: m_accel_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_gyro_sensor(NULL)
, m_fusion_sensor(NULL)
, m_time(0)
{
	cvirtual_sensor_config &config = cvirtual_sensor_config::get_instance();

	sensor_hal *fusion_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(FUSION_SENSOR);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;

	m_name = string(SENSOR_NAME);
	register_supported_event(UNCAL_GYRO_RAW_DATA_EVENT);

	if (!config.get(SENSOR_TYPE_UNCAL_GYRO, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_UNCAL_GYRO, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_UNCAL_GYRO, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	m_interval = m_default_sampling_time * MS_TO_US;
}

uncal_gyro_sensor::~uncal_gyro_sensor()
{
	INFO("uncal_gyro_sensor is destroyed!\n");
}

bool uncal_gyro_sensor::init(void)
{
	m_accel_sensor = sensor_plugin_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_gyro_sensor = sensor_plugin_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);
	m_magnetic_sensor = sensor_plugin_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	m_fusion_sensor = sensor_plugin_loader::get_instance().get_sensor(FUSION_SENSOR);

	if (!m_accel_sensor || !m_gyro_sensor || !m_magnetic_sensor || !m_fusion_sensor) {
		ERR("Failed to load sensors,  accel: 0x%x, gyro: 0x%x, mag: 0x%x, fusion: 0x%x",
			m_accel_sensor, m_gyro_sensor, m_magnetic_sensor, m_fusion_sensor);
		return false;
	}

	INFO("%s is created!\n", sensor_base::get_name());

	return true;
}

sensor_type_t uncal_gyro_sensor::get_type(void)
{
	return UNCAL_GYROSCOPE_SENSOR;
}

bool uncal_gyro_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->add_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_accel_sensor->start();
		m_gyro_sensor->add_client(GYROSCOPE_RAW_DATA_EVENT);
		m_gyro_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_gyro_sensor->start();
		m_magnetic_sensor->add_client(GEOMAGNETIC_RAW_DATA_EVENT);
		m_magnetic_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_magnetic_sensor->start();
	}

	m_fusion_sensor->register_supported_event(FUSION_UNCAL_GYRO_EVENT);
	m_fusion_sensor->register_supported_event(FUSION_UNCAL_GYRO_ENABLED);
	m_fusion_sensor->add_client(FUSION_UNCAL_GYRO_EVENT);
	m_fusion_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_fusion_sensor->start();

	activate();
	return true;
}

bool uncal_gyro_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->delete_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->delete_interval((intptr_t)this, false);
		m_accel_sensor->stop();
		m_gyro_sensor->delete_client(GYROSCOPE_RAW_DATA_EVENT);
		m_gyro_sensor->delete_interval((intptr_t)this, false);
		m_gyro_sensor->stop();
		m_magnetic_sensor->delete_client(GEOMAGNETIC_RAW_DATA_EVENT);
		m_magnetic_sensor->delete_interval((intptr_t)this, false);
		m_magnetic_sensor->stop();
	}

	m_fusion_sensor->delete_client(FUSION_UNCAL_GYRO_EVENT);
	m_fusion_sensor->delete_interval((intptr_t)this, false);
	m_fusion_sensor->unregister_supported_event(FUSION_UNCAL_GYRO_EVENT);
	m_fusion_sensor->unregister_supported_event(FUSION_UNCAL_GYRO_ENABLED);
	m_fusion_sensor->stop();

	deactivate();
	return true;
}

bool uncal_gyro_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->add_interval(client_id, interval, false);
		m_gyro_sensor->add_interval(client_id, interval, false);
		m_magnetic_sensor->add_interval(client_id, interval, false);
	}

	m_fusion_sensor->add_interval(client_id, interval, false);

	return sensor_base::add_interval(client_id, interval, false);
}

bool uncal_gyro_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->delete_interval(client_id, false);
		m_gyro_sensor->delete_interval(client_id, false);
		m_magnetic_sensor->delete_interval(client_id, false);
	}

	m_fusion_sensor->delete_interval(client_id, false);

	return sensor_base::delete_interval(client_id, false);
}

void uncal_gyro_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t uncal_gyro_event;
	unsigned long long diff_time;
	float azimuth_offset;

	if (event.event_type == GYROSCOPE_RAW_DATA_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		m_gyro.m_data.m_vec[0] = event.data.values[0];
		m_gyro.m_data.m_vec[1] = event.data.values[1];
		m_gyro.m_data.m_vec[2] = event.data.values[2];

		m_gyro.m_time_stamp = event.data.timestamp;

		m_enable_uncal_gyro |= GYROSCOPE_ENABLED;
	}

	if (event.event_type == FUSION_UNCAL_GYRO_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		m_fusion.m_data.m_vec[0] = event.data.values[0];
		m_fusion.m_data.m_vec[1] = event.data.values[1];
		m_fusion.m_data.m_vec[2] = event.data.values[2];

		m_fusion.m_time_stamp = event.data.timestamp;

		m_enable_uncal_gyro |= GYRO_BIAS_ENABLED;
	}

	if (m_enable_uncal_gyro == UNCAL_GYRO_BIAS_ENABLED) {
		m_enable_uncal_gyro = 0;

		m_time = get_timestamp();
		uncal_gyro_event.sensor_id = get_id();
		uncal_gyro_event.event_type = UNCAL_GYRO_RAW_DATA_EVENT;
		uncal_gyro_event.data.value_count = 6;
		uncal_gyro_event.data.timestamp = m_time;
		uncal_gyro_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		uncal_gyro_event.data.values[0] = m_gyro.m_data.m_vec[0];
		uncal_gyro_event.data.values[1] = m_gyro.m_data.m_vec[1];
		uncal_gyro_event.data.values[2] = m_gyro.m_data.m_vec[2];

		uncal_gyro_event.data.values[3] = m_fusion.m_data.m_vec[0];
		uncal_gyro_event.data.values[4] = m_fusion.m_data.m_vec[1];
		uncal_gyro_event.data.values[5] = m_fusion.m_data.m_vec[2];

		push(uncal_gyro_event);
	}

	return;
}

int uncal_gyro_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t fusion_data, gyro_data;

	if (event_type != UNCAL_GYRO_RAW_DATA_EVENT)
		return -1;

	m_fusion_sensor->get_sensor_data(FUSION_UNCAL_GYRO_ENABLED, fusion_data);
	m_gyro_sensor->get_sensor_data(GYROSCOPE_RAW_DATA_EVENT, gyro_data);

	data.accuracy = fusion_data.accuracy;
	data.timestamp = get_timestamp();
	data.value_count = 6;
	data.values[0] = gyro_data.values[0];
	data.values[1] = gyro_data.values[1];
	data.values[2] = gyro_data.values[2];
	data.values[3] = fusion_data.values[0];
	data.values[4] = fusion_data.values[1];
	data.values[5] = fusion_data.values[2];

	return 0;
}

bool uncal_gyro_sensor::get_properties(sensor_properties_s &properties)
{
	properties.resolution = 0.000001;
	properties.vendor = m_vendor;
	properties.name = SENSOR_NAME;
	properties.min_interval = 1;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;

	return true;
}

extern "C" sensor_module* create(void)
{
	uncal_gyro_sensor *sensor;

	try {
		sensor = new(std::nothrow) uncal_gyro_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
