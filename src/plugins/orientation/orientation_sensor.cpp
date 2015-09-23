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
#include <common.h>
#include <sf_common.h>
#include <orientation_sensor.h>
#include <sensor_plugin_loader.h>
#include <orientation_filter.h>
#include <cvirtual_sensor_config.h>

#define SENSOR_NAME "ORIENTATION_SENSOR"
#define SENSOR_TYPE_ORIENTATION		"ORIENTATION"

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
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

orientation_sensor::orientation_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
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
	register_supported_event(ORIENTATION_RAW_DATA_EVENT);

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_AZIMUTH_ROTATION_COMPENSATION, &m_azimuth_rotation_compensation)) {
		ERR("[AZIMUTH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_azimuth_rotation_compensation = %d", m_azimuth_rotation_compensation);

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_PITCH_ROTATION_COMPENSATION, &m_pitch_rotation_compensation)) {
		ERR("[PITCH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_pitch_rotation_compensation = %d", m_pitch_rotation_compensation);

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_ROLL_ROTATION_COMPENSATION, &m_roll_rotation_compensation)) {
		ERR("[ROLL_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_roll_rotation_compensation = %d", m_roll_rotation_compensation);

	m_interval = m_default_sampling_time * MS_TO_US;
}

orientation_sensor::~orientation_sensor()
{
	INFO("orientation_sensor is destroyed!\n");
}

bool orientation_sensor::init(void)
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

sensor_type_t orientation_sensor::get_type(void)
{
	return ORIENTATION_SENSOR;
}

bool orientation_sensor::on_start(void)
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
	m_fusion_sensor->register_supported_event(FUSION_ORIENTATION_ENABLED);
	m_fusion_sensor->add_client(FUSION_EVENT);
	m_fusion_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_fusion_sensor->start();

	activate();
	return true;
}

bool orientation_sensor::on_stop(void)
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
	m_fusion_sensor->unregister_supported_event(FUSION_ORIENTATION_ENABLED);
	m_fusion_sensor->stop();

	deactivate();
	return true;
}

bool orientation_sensor::add_interval(int client_id, unsigned int interval)
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

bool orientation_sensor::delete_interval(int client_id)
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

void orientation_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t orientation_event;
	unsigned long long diff_time;
	float azimuth_offset;

	if (event.event_type == FUSION_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		quaternion<float> quat(event.data.values[0], event.data.values[1],
				event.data.values[2], event.data.values[3]);

		euler_angles<float> euler = quat2euler(quat);

		if(m_raw_data_unit == "DEGREES") {
			euler = rad2deg(euler);
			azimuth_offset = AZIMUTH_OFFSET_DEGREES;
		}
		else {
			azimuth_offset = AZIMUTH_OFFSET_RADIANS;
		}

		euler.m_ang.m_vec[0] *= m_pitch_rotation_compensation;
		euler.m_ang.m_vec[1] *= m_roll_rotation_compensation;
		euler.m_ang.m_vec[2] *= m_azimuth_rotation_compensation;

		m_time = get_timestamp();
		orientation_event.sensor_id = get_id();
		orientation_event.event_type = ORIENTATION_RAW_DATA_EVENT;
		orientation_event.data.accuracy = event.data.accuracy;
		orientation_event.data.timestamp = m_time;
		orientation_event.data.value_count = 3;
		orientation_event.data.values[1] = euler.m_ang.m_vec[0];
		orientation_event.data.values[2] = euler.m_ang.m_vec[1];
		if (euler.m_ang.m_vec[2] >= 0)
			orientation_event.data.values[0] = euler.m_ang.m_vec[2];
		else
			orientation_event.data.values[0] = euler.m_ang.m_vec[2] + azimuth_offset;

		push(orientation_event);
	}

	return;
}

int orientation_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t fusion_data;
	float azimuth_offset;

	if (event_type != ORIENTATION_RAW_DATA_EVENT)
		return -1;

	m_fusion_sensor->get_sensor_data(FUSION_ORIENTATION_ENABLED, fusion_data);

	quaternion<float> quat(fusion_data.values[0], fusion_data.values[1],
			fusion_data.values[2], fusion_data.values[3]);

	euler_angles<float> euler = quat2euler(quat);

	if(m_raw_data_unit == "DEGREES") {
		euler = rad2deg(euler);
		azimuth_offset = AZIMUTH_OFFSET_DEGREES;
	}
	else {
		azimuth_offset = AZIMUTH_OFFSET_RADIANS;
	}

	data.accuracy = fusion_data.accuracy;
	data.timestamp = get_timestamp();
	data.value_count = 3;
	data.values[1] = euler.m_ang.m_vec[0];
	data.values[2] = euler.m_ang.m_vec[1];
	if (euler.m_ang.m_vec[2] >= 0)
		data.values[0] = euler.m_ang.m_vec[2];
	else
		data.values[0] = euler.m_ang.m_vec[2] + azimuth_offset;

	data.values[1] *= m_pitch_rotation_compensation;
	data.values[2] *= m_roll_rotation_compensation;
	data.values[0] *= m_azimuth_rotation_compensation;

	return 0;
}

bool orientation_sensor::get_properties(sensor_properties_s &properties)
{
	if(m_raw_data_unit == "DEGREES") {
		properties.min_range = -180;
		properties.max_range = 360;
	}
	else {
		properties.min_range = -PI;
		properties.max_range = 2 * PI;
	}
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
	orientation_sensor *sensor;

	try {
		sensor = new(std::nothrow) orientation_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
