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
#include <sf_common.h>
#include <linear_accel_sensor.h>
#include <sensor_plugin_loader.h>
#include <cvirtual_sensor_config.h>

using std::string;
using std::vector;

#define SENSOR_NAME "LINEAR_ACCEL_SENSOR"
#define SENSOR_TYPE_LINEAR_ACCEL	"LINEAR_ACCEL"
#define SENSOR_TYPE_GRAVITY		"GRAVITY"
#define SENSOR_TYPE_ORIENTATION		"ORIENTATION"

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"
#define ELEMENT_ACCEL_STATIC_BIAS								"ACCEL_STATIC_BIAS"
#define ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION			"ACCEL_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_ACCEL_SCALE										"ACCEL_SCALE"
#define ELEMENT_LINEAR_ACCEL_SIGN_COMPENSATION					"LINEAR_ACCEL_SIGN_COMPENSATION"
#define ELEMENT_ORIENTATION_DATA_UNIT							"RAW_DATA_UNIT"
#define ELEMENT_GRAVITY_SIGN_COMPENSATION						"GRAVITY_SIGN_COMPENSATION"
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

#define INITIAL_VALUE -1
#define GRAVITY 9.80665
#define DEVIATION 0.1

#define PI 3.141593
#define AZIMUTH_OFFSET_DEGREES 360
#define AZIMUTH_OFFSET_RADIANS (2 * PI)

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define ACCELEROMETER_ENABLED 0x01
#define GRAVITY_ENABLED 0x02
#define LINEAR_ACCEL_ENABLED 3

linear_accel_sensor::linear_accel_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_fusion_sensor(NULL)
, m_time(0)
{
	cvirtual_sensor_config &config = cvirtual_sensor_config::get_instance();

	m_name = string(SENSOR_NAME);
	m_enable_linear_accel = 0;
	register_supported_event(LINEAR_ACCEL_RAW_DATA_EVENT);

	sensor_hal *fusion_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_FUSION);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;


	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_ORIENTATION, ELEMENT_ORIENTATION_DATA_UNIT, m_orientation_data_unit)) {
		ERR("[ORIENTATION_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_orientation_data_unit = %s", m_orientation_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_ACCEL_STATIC_BIAS, m_accel_static_bias, 3)) {
		ERR("[ACCEL_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION, m_accel_rotation_direction_compensation, 3)) {
		ERR("[ACCEL_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_accel_rotation_direction_compensation = (%d, %d, %d)", m_accel_rotation_direction_compensation[0], m_accel_rotation_direction_compensation[1], m_accel_rotation_direction_compensation[2]);


	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_ACCEL_SCALE, &m_accel_scale)) {
		ERR("[ACCEL_SCALE] is empty\n");
		throw ENXIO;
	}

	INFO("m_accel_scale = %f", m_accel_scale);

	if (!config.get(SENSOR_TYPE_GRAVITY, ELEMENT_GRAVITY_SIGN_COMPENSATION, m_gravity_sign_compensation, 3)) {
		ERR("[GRAVITY_SIGN_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_gravity_sign_compensation = (%d, %d, %d)", m_gravity_sign_compensation[0], m_gravity_sign_compensation[1], m_gravity_sign_compensation[2]);

	if (!config.get(SENSOR_TYPE_LINEAR_ACCEL, ELEMENT_LINEAR_ACCEL_SIGN_COMPENSATION, m_linear_accel_sign_compensation, 3)) {
		ERR("[LINEAR_ACCEL_SIGN_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_linear_accel_sign_compensation = (%d, %d, %d)", m_linear_accel_sign_compensation[0], m_linear_accel_sign_compensation[1], m_linear_accel_sign_compensation[2]);

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

linear_accel_sensor::~linear_accel_sensor()
{
	INFO("linear_accel_sensor is destroyed!\n");
}

bool linear_accel_sensor::init()
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

void linear_accel_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(LINEAR_ACCEL_SENSOR);
}

bool linear_accel_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_client(ACCELEROMETER_RAW_DATA_EVENT);
	m_accel_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_accel_sensor->start();

	if (!m_hardware_fusion) {
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

bool linear_accel_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);
	m_accel_sensor->delete_client(ACCELEROMETER_RAW_DATA_EVENT);
	m_accel_sensor->delete_interval((intptr_t)this, false);
	m_accel_sensor->stop();

	if (!m_hardware_fusion) {
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

bool linear_accel_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);
	m_accel_sensor->add_interval(client_id, interval, false);

	if (!m_hardware_fusion) {
		m_gyro_sensor->add_interval(client_id, interval, false);
		m_magnetic_sensor->add_interval(client_id, interval, false);
	}

	m_fusion_sensor->add_interval(client_id, interval, false);

	return sensor_base::add_interval(client_id, interval, false);
}

bool linear_accel_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);
	m_accel_sensor->delete_interval(client_id, false);

	if (!m_hardware_fusion) {
		m_gyro_sensor->delete_interval(client_id, false);
		m_magnetic_sensor->delete_interval(client_id, false);
	}

	m_fusion_sensor->delete_interval(client_id, false);

	return sensor_base::delete_interval(client_id, false);
}

sensor_data_t linear_accel_sensor::calculate_gravity(sensor_data_t data)
{
	sensor_data_t gravity_data;
	float pitch, roll, azimuth;
	float azimuth_offset;

	quaternion<float> quat(data.values[0], data.values[1],
			data.values[2], data.values[3]);

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


	if ((roll >= (M_PI/2)-DEVIATION && roll <= (M_PI/2)+DEVIATION) ||
			(roll >= -(M_PI/2)-DEVIATION && roll <= -(M_PI/2)+DEVIATION)) {
		gravity_data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll) * cos(azimuth);
		gravity_data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(azimuth);
		gravity_data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll);
	} else if ((pitch >= (M_PI/2)-DEVIATION && pitch <= (M_PI/2)+DEVIATION) ||
			(pitch >= -(M_PI/2)-DEVIATION && pitch <= -(M_PI/2)+DEVIATION)) {
		gravity_data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(azimuth);
		gravity_data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch) * cos(azimuth);
		gravity_data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(pitch);
	} else {
		gravity_data.values[0] = m_gravity_sign_compensation[0] * GRAVITY * sin(roll);
		gravity_data.values[1] = m_gravity_sign_compensation[1] * GRAVITY * sin(pitch);
		gravity_data.values[2] = m_gravity_sign_compensation[2] * GRAVITY * cos(roll) * cos(pitch);
	}
	gravity_data.value_count = 3;
	gravity_data.timestamp = m_time;
	gravity_data.accuracy = SENSOR_ACCURACY_GOOD;

	return gravity_data;
}

void linear_accel_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t lin_accel_event;
	sensor_data_t gravity_data;

	unsigned long long diff_time;

	if (event.event_type == ACCELEROMETER_RAW_DATA_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		m_accel.m_data.m_vec[0] = m_accel_rotation_direction_compensation[0] * (event.data.values[0] - m_accel_static_bias[0]) / m_accel_scale;
		m_accel.m_data.m_vec[1] = m_accel_rotation_direction_compensation[1] * (event.data.values[1] - m_accel_static_bias[1]) / m_accel_scale;
		m_accel.m_data.m_vec[2] = m_accel_rotation_direction_compensation[2] * (event.data.values[2] - m_accel_static_bias[2]) / m_accel_scale;

		m_accel.m_time_stamp = event.data.timestamp;

		m_enable_linear_accel |= ACCELEROMETER_ENABLED;
	}
	else if (event.event_type == FUSION_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		gravity_data = calculate_gravity(event.data);

		m_enable_linear_accel |= GRAVITY_ENABLED;
	}

	if (m_enable_linear_accel == LINEAR_ACCEL_ENABLED) {
		m_enable_linear_accel = 0;

		m_time = get_timestamp();
		lin_accel_event.sensor_id = get_id();
		lin_accel_event.event_type = LINEAR_ACCEL_RAW_DATA_EVENT;
		lin_accel_event.data.value_count = 3;
		lin_accel_event.data.timestamp = m_time;
		lin_accel_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		lin_accel_event.data.values[0] = m_linear_accel_sign_compensation[0] * (m_accel.m_data.m_vec[0] - gravity_data.values[0]);
		lin_accel_event.data.values[1] = m_linear_accel_sign_compensation[1] * (m_accel.m_data.m_vec[1] - gravity_data.values[1]);
		lin_accel_event.data.values[2] = m_linear_accel_sign_compensation[2] * (m_accel.m_data.m_vec[2] - gravity_data.values[2]);
		push(lin_accel_event);
	}

	return;
}

int linear_accel_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t gravity_data, accel_data, fusion_data;
	m_fusion_sensor->get_sensor_data(FUSION_ORIENTATION_ENABLED, fusion_data);
	m_accel_sensor->get_sensor_data(ACCELEROMETER_RAW_DATA_EVENT, accel_data);

	gravity_data = calculate_gravity(fusion_data);

	accel_data.values[0] = m_accel_rotation_direction_compensation[0] * (accel_data.values[0] - m_accel_static_bias[0]) / m_accel_scale;
	accel_data.values[1] = m_accel_rotation_direction_compensation[1] * (accel_data.values[1] - m_accel_static_bias[1]) / m_accel_scale;
	accel_data.values[2] = m_accel_rotation_direction_compensation[2] * (accel_data.values[2] - m_accel_static_bias[2]) / m_accel_scale;

	if (event_type != LINEAR_ACCEL_RAW_DATA_EVENT)
		return -1;

	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = get_timestamp();
	data.values[0] = m_linear_accel_sign_compensation[0] * (accel_data.values[0] - gravity_data.values[0]);
	data.values[1] = m_linear_accel_sign_compensation[1] * (accel_data.values[1] - gravity_data.values[1]);
	data.values[2] = m_linear_accel_sign_compensation[2] * (accel_data.values[2] - gravity_data.values[2]);
	data.value_count = 3;
	return 0;
}

bool linear_accel_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	m_accel_sensor->get_properties(ACCELEROMETER_SENSOR, properties);
	properties.name = "Linear Acceleration Sensor";
	properties.vendor = m_vendor;
	properties.resolution = 0.000001;

	return true;
}

extern "C" sensor_module* create(void)
{
	linear_accel_sensor *sensor;

	try {
		sensor = new(std::nothrow) linear_accel_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
