/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <sensor_logs.h>
#include <rv_sensor.h>
#include <sensor_loader.h>
#include <orientation_filter.h>
#include <virtual_sensor_config.h>

using std::string;
using std::vector;

#define SENSOR_NAME "RV_SENSOR"
#define SENSOR_TYPE_RV		"ROTATION_VECTOR"

#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define INITIAL_VALUE -1

#define MS_TO_US 1000

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"

rv_sensor::rv_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_fusion_sensor(NULL)
, m_time(0)
{
	virtual_sensor_config &config = virtual_sensor_config::get_instance();

	// Will check if fusion_sensor is in the list of hal sensors.
	sensor_hal *fusion_sensor_hal = sensor_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_FUSION);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;

	m_name = string(SENSOR_NAME);
	register_supported_event(ROTATION_VECTOR_RAW_DATA_EVENT);
	m_enable_orientation = 0;

	if (!config.get(SENSOR_TYPE_RV, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_RV, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	m_interval = m_default_sampling_time * MS_TO_US;
}

rv_sensor::~rv_sensor()
{
	INFO("rv_sensor is destroyed!\n");
}

bool rv_sensor::init()
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_gyro_sensor = sensor_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);
	m_magnetic_sensor = sensor_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	m_fusion_sensor = sensor_loader::get_instance().get_sensor(FUSION_SENSOR);

	if (!m_accel_sensor || !m_gyro_sensor || !m_magnetic_sensor || !m_fusion_sensor) {
		ERR("Failed to load sensors,  accel: 0x%x, gyro: 0x%x, mag: 0x%x, fusion: 0x%x",
			m_accel_sensor, m_gyro_sensor, m_magnetic_sensor, m_fusion_sensor);
		return false;
	}

	INFO("%s is created!\n", sensor_base::get_name());

	return true;
}

void rv_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(ROTATION_VECTOR_SENSOR);
}

bool rv_sensor::on_start(void)
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

	m_fusion_sensor->register_supported_event(FUSION_EVENT);
	m_fusion_sensor->register_supported_event(FUSION_ROTATION_VECTOR_ENABLED);
	m_fusion_sensor->add_client(FUSION_EVENT);
	m_fusion_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_fusion_sensor->start();

	activate();
	return true;
}

bool rv_sensor::on_stop(void)
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

	m_fusion_sensor->delete_client(FUSION_EVENT);
	m_fusion_sensor->delete_interval((intptr_t)this, false);
	m_fusion_sensor->unregister_supported_event(FUSION_EVENT);
	m_fusion_sensor->unregister_supported_event(FUSION_ROTATION_VECTOR_ENABLED);
	m_fusion_sensor->stop();

	deactivate();
	return true;
}

bool rv_sensor::add_interval(int client_id, unsigned int interval)
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

bool rv_sensor::delete_interval(int client_id)
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

void rv_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	unsigned long long diff_time;

	sensor_event_t rv_event;

	if (event.event_type == FUSION_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		m_time = get_timestamp();
		rv_event.sensor_id = get_id();
		rv_event.event_type = ROTATION_VECTOR_RAW_DATA_EVENT;
		rv_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		rv_event.data.timestamp = m_time;
		rv_event.data.value_count = 4;
		rv_event.data.values[0] = event.data.values[1];
		rv_event.data.values[1] = event.data.values[2];
		rv_event.data.values[2] = event.data.values[3];
		rv_event.data.values[3] = event.data.values[0];

		push(rv_event);
	}

	return;
}

int rv_sensor::get_sensor_data(unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t fusion_data;

	if (event_type != ROTATION_VECTOR_RAW_DATA_EVENT)
		return -1;

	m_fusion_sensor->get_sensor_data(FUSION_ROTATION_VECTOR_ENABLED, fusion_data);

	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = get_timestamp();
	data.value_count = 4;
	data.values[0] = fusion_data.values[1];
	data.values[1] = fusion_data.values[2];
	data.values[2] = fusion_data.values[3];
	data.values[3] = fusion_data.values[0];

	return 0;
}

bool rv_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	properties.vendor = m_vendor;
	properties.name = SENSOR_NAME;
	properties.min_range = -1;
	properties.max_range = 1;
	properties.resolution = 0.000001;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	properties.min_interval = 1;

	return true;
}

