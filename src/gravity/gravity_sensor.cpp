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
#include <gravity_sensor.h>
#include <sensor_plugin_loader.h>
#include <cvirtual_sensor_config.h>

#define INITIAL_VALUE -1
#define GRAVITY 9.80665

#define DEVIATION 0.1

#define PI 3.141593
#define AZIMUTH_OFFSET_DEGREES 360
#define AZIMUTH_OFFSET_RADIANS (2 * PI)

#define SENSOR_NAME "GRAVITY_SENSOR"
#define SENSOR_TYPE_GRAVITY		"GRAVITY"
#define SENSOR_TYPE_ORIENTATION		"ORIENTATION"

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"
#define ELEMENT_GRAVITY_SIGN_COMPENSATION						"GRAVITY_SIGN_COMPENSATION"
#define ELEMENT_ORIENTATION_DATA_UNIT							"RAW_DATA_UNIT"
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

gravity_sensor::gravity_sensor()
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

	m_name = std::string(SENSOR_NAME);
	register_supported_event(GRAVITY_RAW_DATA_EVENT);

	if (!config.get(SENSOR_TYPE_GRAVITY, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_ORIENTATION_DATA_UNIT, m_orientation_data_unit)) {
		ERR("[ORIENTATION_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_orientation_data_unit = %s", m_orientation_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_GRAVITY, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_GRAVITY, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_GRAVITY, ELEMENT_GRAVITY_SIGN_COMPENSATION, m_gravity_sign_compensation, 3)) {
		ERR("[GRAVITY_SIGN_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_gravity_sign_compensation = (%d, %d, %d)", m_gravity_sign_compensation[0], m_gravity_sign_compensation[1], m_gravity_sign_compensation[2]);

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

gravity_sensor::~gravity_sensor()
{
	INFO("gravity_sensor is destroyed!\n");
}

bool gravity_sensor::init()
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

	INFO("%s is created!", sensor_base::get_name());
	return true;
}

sensor_type_t gravity_sensor::get_type(void)
{
	return GRAVITY_SENSOR;
}

bool gravity_sensor::on_start(void)
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

bool gravity_sensor::on_stop(void)
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

bool gravity_sensor::add_interval(int client_id, unsigned int interval)
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

bool gravity_sensor::delete_interval(int client_id)
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

void gravity_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t gravity_event;
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

		if(m_orientation_data_unit == "DEGREES") {
			euler = rad2deg(euler);
			azimuth_offset = AZIMUTH_OFFSET_DEGREES;
		}
		else {
			azimuth_offset = AZIMUTH_OFFSET_RADIANS;
		}

		euler.m_ang.m_vec[0] *= m_pitch_rotation_compensation;
		euler.m_ang.m_vec[1] *= m_roll_rotation_compensation;
		euler.m_ang.m_vec[2] *= m_azimuth_rotation_compensation;

		pitch = euler.m_ang.m_vec[0];
		roll = euler.m_ang.m_vec[1];
		if (euler.m_ang.m_vec[2] >= 0)
			azimuth = euler.m_ang.m_vec[2];
		else
			azimuth = euler.m_ang.m_vec[2] + azimuth_offset;

		if(m_orientation_data_unit == "DEGREES") {
			azimuth *= DEG2RAD;
			pitch *= DEG2RAD;
			roll *= DEG2RAD;
		}

		m_time = get_timestamp();
		gravity_event.sensor_id = get_id();
		gravity_event.event_type = GRAVITY_RAW_DATA_EVENT;


		if ((roll >= (M_PI/2)-DEVIATION && roll <= (M_PI/2)+DEVIATION) ||
				(roll >= -(M_PI/2)-DEVIATION && roll <= -(M_PI/2)+DEVIATION)) {
			gravity_event.data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll) * cos(azimuth);
			gravity_event.data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(azimuth);
			gravity_event.data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll);
		} else if ((pitch >= (M_PI/2)-DEVIATION && pitch <= (M_PI/2)+DEVIATION) ||
				(pitch >= -(M_PI/2)-DEVIATION && pitch <= -(M_PI/2)+DEVIATION)) {
			gravity_event.data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(azimuth);
			gravity_event.data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch) * cos(azimuth);
			gravity_event.data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(pitch);
		} else {
			gravity_event.data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll);
			gravity_event.data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch);
			gravity_event.data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll) * cos(pitch);
		}
		gravity_event.data.value_count = 3;
		gravity_event.data.timestamp = m_time;
		gravity_event.data.accuracy = SENSOR_ACCURACY_GOOD;

		push(gravity_event);
	}

	return;
}

int gravity_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t fusion_data;
	float azimuth_offset;
	float pitch, roll, azimuth;

	if (event_type != GRAVITY_RAW_DATA_EVENT)
		return -1;

	m_fusion_sensor->get_sensor_data(FUSION_ORIENTATION_ENABLED, fusion_data);

	quaternion<float> quat(fusion_data.values[0], fusion_data.values[1],
			fusion_data.values[2], fusion_data.values[3]);

	euler_angles<float> euler = quat2euler(quat);

	if(m_orientation_data_unit == "DEGREES") {
		euler = rad2deg(euler);
		azimuth_offset = AZIMUTH_OFFSET_DEGREES;
	}
	else {
		azimuth_offset = AZIMUTH_OFFSET_RADIANS;
	}

	pitch = euler.m_ang.m_vec[0];
	roll = euler.m_ang.m_vec[1];

	if (euler.m_ang.m_vec[2] >= 0)
		azimuth = euler.m_ang.m_vec[2];
	else
		azimuth = euler.m_ang.m_vec[2] + azimuth_offset;

	if(m_orientation_data_unit == "DEGREES") {
		azimuth *= DEG2RAD;
		pitch *= DEG2RAD;
		roll *= DEG2RAD;
	}

	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = get_timestamp();
	if ((roll >= (M_PI/2)-DEVIATION && roll <= (M_PI/2)+DEVIATION) ||
			(roll >= -(M_PI/2)-DEVIATION && roll <= -(M_PI/2)+DEVIATION)) {
		data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll) * cos(azimuth);
		data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(azimuth);
		data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll);
	} else if ((pitch >= (M_PI/2)-DEVIATION && pitch <= (M_PI/2)+DEVIATION) ||
			(pitch >= -(M_PI/2)-DEVIATION && pitch <= -(M_PI/2)+DEVIATION)) {
		data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(azimuth);
		data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch) * cos(azimuth);
		data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(pitch);
	} else {
		data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll);
		data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch);
		data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll) * cos(pitch);
	}
	data.value_count = 3;

	return 0;
}

bool gravity_sensor::get_properties(sensor_properties_s &properties)
{
	properties.min_range = -GRAVITY;
	properties.max_range = GRAVITY;
	properties.resolution = 0.000001;
	properties.vendor = m_vendor;
	properties.name = SENSOR_NAME;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	properties.min_interval = 1;

	return true;
}

extern "C" sensor_module* create(void)
{
	gravity_sensor *sensor;

	try {
		sensor = new(std::nothrow) gravity_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
