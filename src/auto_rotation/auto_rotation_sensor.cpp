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
#include <auto_rotation_sensor.h>
#include <sensor_plugin_loader.h>
#include <cvirtual_sensor_config.h>

#include <auto_rotation_alg_emul.h>

#define INITIAL_VALUE -1
#define GRAVITY 9.80665

#define DEVIATION 0.1

#define PI 3.141593
#define AZIMUTH_OFFSET_DEGREES 360
#define AZIMUTH_OFFSET_RADIANS (2 * PI)

#define SENSOR_NAME "AUTO_ROTATION_SENSOR"
#define SENSOR_TYPE_AUTO_ROTATION		"AUTO_ROTATION"
#define SENSOR_TYPE_ORIENTATION		"ORIENTATION"

#define ACCELEROMETER_ENABLED 0x01
#define GYROSCOPE_ENABLED 0x02
#define GEOMAGNETIC_ENABLED 0x04
#define TILT_ENABLED 1
#define GAMING_RV_ENABLED 3
#define GEOMAGNETIC_RV_ENABLED 5
#define ORIENTATION_ENABLED 7
#define ROTATION_VECTOR_ENABLED 7

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

auto_rotation_sensor::auto_rotation_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_fusion_sensor(NULL)
, m_time(0)
, m_alg(NULL)
{
	cvirtual_sensor_config &config = cvirtual_sensor_config::get_instance();

	sensor_hal *fusion_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(FUSION_SENSOR);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;

	m_enable_fusion = 0;
	sensor_hal *accel_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(ACCELEROMETER_SENSOR);
	sensor_hal *gyro_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(GYROSCOPE_SENSOR);
	sensor_hal *magnetic_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(GEOMAGNETIC_SENSOR);

	if (accel_sensor_hal)
		m_enable_fusion |= ACCELEROMETER_ENABLED;
	if (gyro_sensor_hal)
		m_enable_fusion |= GYROSCOPE_ENABLED;
	if (magnetic_sensor_hal)
		m_enable_fusion |= GEOMAGNETIC_ENABLED;

	INFO ("m_enable_fusion = %d", m_enable_fusion);

	m_name = std::string(SENSOR_NAME);
	register_supported_event(AUTO_ROTATION_CHANGE_STATE_EVENT);

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_AZIMUTH_ROTATION_COMPENSATION, &m_azimuth_rotation_compensation)) {
		ERR("[AZIMUTH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_azimuth_rotation_compensation = %d", m_azimuth_rotation_compensation);

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_PITCH_ROTATION_COMPENSATION, &m_pitch_rotation_compensation)) {
		ERR("[PITCH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_pitch_rotation_compensation = %d", m_pitch_rotation_compensation);

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_ROLL_ROTATION_COMPENSATION, &m_roll_rotation_compensation)) {
		ERR("[ROLL_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_roll_rotation_compensation = %d", m_roll_rotation_compensation);

	m_interval = m_default_sampling_time * MS_TO_US;

	m_rotation_x = AUTO_ROTATION_DEGREE_UNKNOWN;
	m_rotation_y = AUTO_ROTATION_DEGREE_UNKNOWN;
	m_rotation_z = AUTO_ROTATION_DEGREE_UNKNOWN;

}

auto_rotation_sensor::~auto_rotation_sensor()
{
	delete m_alg;

	INFO("auto_rotation_sensor is destroyed!\n");
}

auto_rotation_alg *auto_rotation_sensor::get_alg()
{
	return new auto_rotation_alg_emul();
}

bool auto_rotation_sensor::init()
{
	m_accel_sensor = sensor_plugin_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (m_enable_fusion & GYROSCOPE_ENABLED)
		m_gyro_sensor = sensor_plugin_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);
	if (m_enable_fusion & GEOMAGNETIC_ENABLED)
		m_magnetic_sensor = sensor_plugin_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	m_fusion_sensor = sensor_plugin_loader::get_instance().get_sensor(FUSION_SENSOR);

	if (!m_accel_sensor || !m_fusion_sensor) {
		ERR("Failed to load sensors,  accel: 0x%x, gyro: 0x%x, mag: 0x%x, fusion: 0x%x",
			m_accel_sensor, m_gyro_sensor, m_magnetic_sensor, m_fusion_sensor);
		return false;
	}

	m_alg = get_alg();

	if (!m_alg) {
		ERR("Not supported AUTO ROTATION sensor");
		return false;
	}

	if (!m_alg->open())
		return false;

	INFO("%s is created!", sensor_base::get_name());
	return true;
}

sensor_type_t auto_rotation_sensor::get_type(void)
{
	return AUTO_ROTATION_SENSOR;
}

bool auto_rotation_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->add_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
		m_accel_sensor->start();
		if (m_enable_fusion & GYROSCOPE_ENABLED) {
			m_gyro_sensor->add_client(GYROSCOPE_RAW_DATA_EVENT);
			m_gyro_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
			m_gyro_sensor->start();
		}
		if (m_enable_fusion & GEOMAGNETIC_ENABLED) {
			m_magnetic_sensor->add_client(GEOMAGNETIC_RAW_DATA_EVENT);
			m_magnetic_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
			m_magnetic_sensor->start();
		}
	}

	m_fusion_sensor->register_supported_event(FUSION_EVENT);
	if (m_enable_fusion == ORIENTATION_ENABLED)
		m_fusion_sensor->register_supported_event(FUSION_ORIENTATION_ENABLED);
	else if (m_enable_fusion == GAMING_RV_ENABLED)
		m_fusion_sensor->register_supported_event(FUSION_GAMING_ROTATION_VECTOR_ENABLED);
	else if (m_enable_fusion == GEOMAGNETIC_RV_ENABLED)
		m_fusion_sensor->register_supported_event(FUSION_GEOMAGNETIC_ROTATION_VECTOR_ENABLED);
	else if (m_enable_fusion == TILT_ENABLED)
		m_fusion_sensor->register_supported_event(FUSION_TILT_ENABLED);

	m_fusion_sensor->add_client(FUSION_EVENT);
	m_fusion_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_fusion_sensor->start();

	m_alg->start();

	activate();
	return true;
}

bool auto_rotation_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_hardware_fusion) {
		m_accel_sensor->delete_client(ACCELEROMETER_RAW_DATA_EVENT);
		m_accel_sensor->delete_interval((intptr_t)this, false);
		m_accel_sensor->stop();
		if (m_enable_fusion & GYROSCOPE_ENABLED) {
			m_gyro_sensor->delete_client(GYROSCOPE_RAW_DATA_EVENT);
			m_gyro_sensor->delete_interval((intptr_t)this, false);
			m_gyro_sensor->stop();
		}
		if (m_enable_fusion & GEOMAGNETIC_ENABLED) {
			m_magnetic_sensor->delete_client(GEOMAGNETIC_RAW_DATA_EVENT);
			m_magnetic_sensor->delete_interval((intptr_t)this, false);
			m_magnetic_sensor->stop();
		}
	}

	m_fusion_sensor->delete_client(FUSION_EVENT);
	m_fusion_sensor->delete_interval((intptr_t)this, false);
	m_fusion_sensor->unregister_supported_event(FUSION_EVENT);
	if (m_enable_fusion == ORIENTATION_ENABLED)
		m_fusion_sensor->unregister_supported_event(FUSION_ORIENTATION_ENABLED);
	else if (m_enable_fusion == GAMING_RV_ENABLED)
		m_fusion_sensor->unregister_supported_event(FUSION_GAMING_ROTATION_VECTOR_ENABLED);
	else if (m_enable_fusion == GEOMAGNETIC_RV_ENABLED)
		m_fusion_sensor->unregister_supported_event(FUSION_GEOMAGNETIC_ROTATION_VECTOR_ENABLED);
	else if (m_enable_fusion == TILT_ENABLED)
		m_fusion_sensor->unregister_supported_event(FUSION_TILT_ENABLED);

	m_fusion_sensor->stop();

	deactivate();
	return true;
}

bool auto_rotation_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);
	if (!m_hardware_fusion) {
		m_accel_sensor->add_interval(client_id, interval, false);
		if (m_enable_fusion & GYROSCOPE_ENABLED)
			m_gyro_sensor->add_interval(client_id, interval, false);
		if (m_enable_fusion & GEOMAGNETIC_ENABLED)
			m_magnetic_sensor->add_interval(client_id, interval, false);
	}

	m_fusion_sensor->add_interval(client_id, interval, false);

	return sensor_base::add_interval(client_id, interval, false);
}

bool auto_rotation_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);
	if (!m_hardware_fusion) {
		m_accel_sensor->delete_interval(client_id, false);
		if (m_enable_fusion & GYROSCOPE_ENABLED)
			m_gyro_sensor->delete_interval(client_id, false);
		if (m_enable_fusion & GEOMAGNETIC_ENABLED)
			m_magnetic_sensor->delete_interval(client_id, false);
	}

	m_fusion_sensor->delete_interval(client_id, false);

	return sensor_base::delete_interval(client_id, false);
}

void auto_rotation_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t auto_rotation_event;
	float pitch, roll, azimuth;
	unsigned long long diff_time;
	float azimuth_offset;

	if (event.event_type == FUSION_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		quaternion<float> quat(event.data.values[0], event.data.values[1],
				event.data.values[2], event.data.values[3]);

		euler_angles<float> euler = quat2euler(quat);

		euler = rad2deg(euler);
		azimuth_offset = AZIMUTH_OFFSET_DEGREES;

		euler.m_ang.m_vec[0] *= m_pitch_rotation_compensation;
		euler.m_ang.m_vec[1] *= m_roll_rotation_compensation;
		euler.m_ang.m_vec[2] *= m_azimuth_rotation_compensation;


		int rotation_x, rotation_y, rotation_z;
		bool changed1, changed2, changed3;
		float acc[3];
		acc[1] = euler.m_ang.m_vec[0];
		acc[2] = euler.m_ang.m_vec[1];

		if (euler.m_ang.m_vec[2] >= 0)
			acc[0] = euler.m_ang.m_vec[2];
		else
			acc[0] = euler.m_ang.m_vec[2] + azimuth_offset;

		changed2 = m_alg->get_rotation(acc[1]+180, m_rotation_y, rotation_y);
		changed3 = m_alg->get_rotation(acc[2]+90, m_rotation_z, rotation_z);
		m_rotation_y = rotation_y;
		m_rotation_z = rotation_z;

		if (m_enable_fusion == ORIENTATION_ENABLED || m_enable_fusion == GEOMAGNETIC_RV_ENABLED
				|| m_enable_fusion == GAMING_RV_ENABLED) {
			changed1 = m_alg->get_rotation(acc[0], m_rotation_x, rotation_x);
			m_rotation_x = rotation_x;
		}

		m_alg->correct_rotation(rotation_x, rotation_y, rotation_z);

		m_time = get_timestamp();
		auto_rotation_event.sensor_id = get_id();
		auto_rotation_event.event_type = AUTO_ROTATION_CHANGE_STATE_EVENT;
		auto_rotation_event.data.value_count = 1;
		auto_rotation_event.data.timestamp = m_time;
		auto_rotation_event.data.accuracy = SENSOR_ACCURACY_GOOD;

		INFO ("m_enable_fusion = %d", m_enable_fusion);
		INFO ("rotation_values %d %d %d", rotation_x, rotation_y, rotation_z);

		if (m_enable_fusion == ORIENTATION_ENABLED || m_enable_fusion == GEOMAGNETIC_RV_ENABLED
			|| m_enable_fusion == GAMING_RV_ENABLED) {
			if (changed1) {
				auto_rotation_event.data.values[0] = rotation_x;
				push(auto_rotation_event);
			}
		}
		if (changed2) {
			auto_rotation_event.data.values[0] = rotation_y;
			push(auto_rotation_event);
		}
		if (changed3) {
			auto_rotation_event.data.values[0] = rotation_z;
			push(auto_rotation_event);
		}

	}

	return;
}

int auto_rotation_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t fusion_data;
	float azimuth_offset;
	float pitch, roll, azimuth;

	if (event_type != AUTO_ROTATION_CHANGE_STATE_EVENT)
		return -1;

	m_fusion_sensor->get_sensor_data(FUSION_ORIENTATION_ENABLED, fusion_data);

	quaternion<float> quat(fusion_data.values[0], fusion_data.values[1],
			fusion_data.values[2], fusion_data.values[3]);

	euler_angles<float> euler = quat2euler(quat);

	euler = rad2deg(euler);
	azimuth_offset = AZIMUTH_OFFSET_DEGREES;

	euler.m_ang.m_vec[0] *= m_pitch_rotation_compensation;
	euler.m_ang.m_vec[1] *= m_roll_rotation_compensation;
	euler.m_ang.m_vec[2] *= m_azimuth_rotation_compensation;

	int rotation_x, rotation_y, rotation_z;
	bool changed1, changed2, changed3;
	float acc[3];
	acc[1] = euler.m_ang.m_vec[0];
	acc[2] = euler.m_ang.m_vec[1];

	if (euler.m_ang.m_vec[2] >= 0)
		acc[0] = euler.m_ang.m_vec[2];
	else
		acc[0] = euler.m_ang.m_vec[2] + azimuth_offset;

	changed2 = m_alg->get_rotation(acc[1]+180, m_rotation_y, rotation_y);
	changed3 = m_alg->get_rotation(acc[2]+90, m_rotation_z, rotation_z);
	m_rotation_y = rotation_y;
	m_rotation_z = rotation_z;

	if (m_enable_fusion == ORIENTATION_ENABLED || m_enable_fusion == GEOMAGNETIC_RV_ENABLED
		|| m_enable_fusion == GAMING_RV_ENABLED) {
		changed1 = m_alg->get_rotation(acc[0], m_rotation_x, rotation_x);
		m_rotation_x = rotation_x;
	}

	m_alg->correct_rotation(rotation_x, rotation_y, rotation_z);

	data.timestamp = get_timestamp();
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.value_count = 1;
	if (m_enable_fusion == ORIENTATION_ENABLED || m_enable_fusion == GEOMAGNETIC_RV_ENABLED
		|| m_enable_fusion == GAMING_RV_ENABLED) {
		if (changed1) {
			data.values[0] = rotation_x;
		}
	}
	if (changed2) {
		data.values[0] = rotation_y;
	}
	if (changed3) {
		data.values[0] = rotation_z;
	}

	return 0;
}

bool auto_rotation_sensor::get_properties(sensor_properties_s &properties)
{
	properties.min_range = AUTO_ROTATION_DEGREE_UNKNOWN;
	properties.max_range = AUTO_ROTATION_DEGREE_270;
	properties.vendor = m_vendor;
	properties.name = m_name;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	properties.min_interval = 1;

	return true;
}

extern "C" sensor_module* create(void)
{
	auto_rotation_sensor *sensor;

	try {
		sensor = new(std::nothrow) auto_rotation_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
