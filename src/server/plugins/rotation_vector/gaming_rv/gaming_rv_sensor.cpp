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
#include <sensor_log.h>
#include <gaming_rv_sensor.h>
#include <sensor_loader.h>
#include <orientation_filter.h>
#include <virtual_sensor_config.h>

using std::string;
using std::vector;

#define SENSOR_NAME "GAMING_RV_SENSOR"
#define SENSOR_TYPE_GAMING_RV "GAMING_ROTATION_VECTOR"

#define ACCELEROMETER_ENABLED 0x01
#define GYROSCOPE_ENABLED 0x02
#define GAMING_RV_ENABLED 3

#define INITIAL_VALUE -1

#define MS_TO_US 1000

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"
#define ELEMENT_ACCEL_STATIC_BIAS								"ACCEL_STATIC_BIAS"
#define ELEMENT_GYRO_STATIC_BIAS								"GYRO_STATIC_BIAS"
#define ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION			"ACCEL_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_GYRO_ROTATION_DIRECTION_COMPENSATION			"GYRO_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_ACCEL_SCALE										"ACCEL_SCALE"
#define ELEMENT_GYRO_SCALE										"GYRO_SCALE"

gaming_rv_sensor::gaming_rv_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_accuracy(-1)
, m_time(0)
{
	virtual_sensor_config &config = virtual_sensor_config::get_instance();

	sensor_hal *fusion_sensor_hal = sensor_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_FUSION);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;

	_I("m_hardware_fusion = %d", m_hardware_fusion);

	m_name = string(SENSOR_NAME);
	register_supported_event(GAMING_RV_RAW_DATA_EVENT);
	m_enable_gaming_rv = 0;

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_VENDOR, m_vendor)) {
		_E("[VENDOR] is empty\n");
		throw ENXIO;
	}

	_I("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		_E("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	_I("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_ACCEL_STATIC_BIAS, m_accel_static_bias, 3)) {
		_E("[ACCEL_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	_I("m_accel_static_bias = (%f, %f, %f)", m_accel_static_bias[0], m_accel_static_bias[1], m_accel_static_bias[2]);

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_GYRO_STATIC_BIAS, m_gyro_static_bias,3)) {
		_E("[GYRO_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	_I("m_gyro_static_bias = (%f, %f, %f)", m_gyro_static_bias[0], m_gyro_static_bias[1], m_gyro_static_bias[2]);

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION, m_accel_rotation_direction_compensation, 3)) {
		_E("[ACCEL_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	_I("m_accel_rotation_direction_compensation = (%d, %d, %d)", m_accel_rotation_direction_compensation[0], m_accel_rotation_direction_compensation[1], m_accel_rotation_direction_compensation[2]);

	if (!config.get(SENSOR_TYPE_GAMING_RV, ELEMENT_GYRO_ROTATION_DIRECTION_COMPENSATION, m_gyro_rotation_direction_compensation, 3)) {
		_E("[GYRO_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	_I("m_gyro_rotation_direction_compensation = (%d, %d, %d)", m_gyro_rotation_direction_compensation[0], m_gyro_rotation_direction_compensation[1], m_gyro_rotation_direction_compensation[2]);

	m_interval = m_default_sampling_time * MS_TO_US;
}

gaming_rv_sensor::~gaming_rv_sensor()
{
	_I("gaming_rv_sensor is destroyed!\n");
}

bool gaming_rv_sensor::init()
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_gyro_sensor = sensor_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);

	if (!m_accel_sensor || !m_gyro_sensor) {
		_E("Failed to load sensors,  accel: 0x%x, gyro: 0x%x",
			m_accel_sensor, m_gyro_sensor);
		return false;
	}

	_I("%s is created!\n", sensor_base::get_name());

	return true;
}

void gaming_rv_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(GAMING_RV_SENSOR);
}

bool gaming_rv_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->add_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_accel_sensor->start();
		m_gyro_sensor->add_client(GYROSCOPE_RAW_DATA_EVENT);
		m_gyro_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_gyro_sensor->start();
	}

	activate();
	return true;
}

bool gaming_rv_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->delete_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->delete_interval((intptr_t)this, false);
		m_accel_sensor->stop();
		m_gyro_sensor->delete_client(GYROSCOPE_RAW_DATA_EVENT);
		m_gyro_sensor->delete_interval((intptr_t)this, false);
		m_gyro_sensor->stop();
	}

	deactivate();
	return true;
}

bool gaming_rv_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->add_interval(client_id, interval, false);
		m_gyro_sensor->add_interval(client_id, interval, false);
	}

	return sensor_base::add_interval(client_id, interval, false);
}

bool gaming_rv_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->delete_interval(client_id, false);
		m_gyro_sensor->delete_interval(client_id, false);
	}

	return sensor_base::delete_interval(client_id, false);
}

void gaming_rv_sensor::synthesize(const sensor_event_t& event, vector<sensor_event_t> &outs)
{
	const float MIN_DELIVERY_DIFF_FACTOR = 0.75f;
	unsigned long long diff_time;

	sensor_event_t rv_event;
	quaternion<float> quaternion_gaming_rv;

	if (event.event_type == ACCELEROMETER_RAW_DATA_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(m_accel, event.data.values, m_accel_static_bias, m_accel_rotation_direction_compensation, ACCEL_SCALE);

		m_accel.m_time_stamp = event.data.timestamp;

		m_enable_gaming_rv |= ACCELEROMETER_ENABLED;
	} else if (event.event_type == GYROSCOPE_RAW_DATA_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(m_gyro, event.data.values, m_gyro_static_bias, m_gyro_rotation_direction_compensation, GYRO_SCALE);

		m_gyro.m_time_stamp = event.data.timestamp;

		m_enable_gaming_rv |= GYROSCOPE_ENABLED;
	}

	if (m_enable_gaming_rv == GAMING_RV_ENABLED) {
		m_enable_gaming_rv = 0;

		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, NULL);

		quaternion_gaming_rv = m_orientation_filter.m_quat_gaming_rv;

		m_time = get_timestamp();
		rv_event.sensor_id = get_id();
		rv_event.event_type = GAMING_RV_RAW_DATA_EVENT;
		rv_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		rv_event.data.timestamp = m_time;
		rv_event.data.value_count = 4;
		rv_event.data.values[0] = quaternion_gaming_rv.m_quat.m_vec[1];
		rv_event.data.values[1] = quaternion_gaming_rv.m_quat.m_vec[2];
		rv_event.data.values[2] = quaternion_gaming_rv.m_quat.m_vec[3];
		rv_event.data.values[3] = quaternion_gaming_rv.m_quat.m_vec[0];

		push(rv_event);
	}

	return;
}

int gaming_rv_sensor::get_sensor_data(unsigned int event_type, sensor_data_t &data)
{
	sensor_data<float> accel;
	sensor_data<float> gyro;

	sensor_data_t accel_data;
	sensor_data_t gyro_data;

	quaternion<float> quaternion_gaming_rv;

	if (event_type != GAMING_RV_RAW_DATA_EVENT)
		return -1;

	m_accel_sensor->get_sensor_data(ACCELEROMETER_RAW_DATA_EVENT, accel_data);
	m_gyro_sensor->get_sensor_data(GYROSCOPE_RAW_DATA_EVENT, gyro_data);

	pre_process_data(accel, accel_data.values, m_accel_static_bias, m_accel_rotation_direction_compensation, ACCEL_SCALE);
	pre_process_data(gyro, gyro_data.values, m_gyro_static_bias, m_gyro_rotation_direction_compensation, GYRO_SCALE);
	accel.m_time_stamp = accel_data.timestamp;
	gyro.m_time_stamp = gyro_data.timestamp;

	m_orientation_filter_poll.get_device_orientation(&m_accel, &m_gyro, NULL);

	quaternion_gaming_rv = m_orientation_filter_poll.m_quat_gaming_rv;

	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = get_timestamp();
	data.value_count = 4;
	data.values[0] = quaternion_gaming_rv.m_quat.m_vec[1];
	data.values[1] = quaternion_gaming_rv.m_quat.m_vec[2];
	data.values[2] = quaternion_gaming_rv.m_quat.m_vec[3];
	data.values[3] = quaternion_gaming_rv.m_quat.m_vec[0];

	return 0;
}

bool gaming_rv_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
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

