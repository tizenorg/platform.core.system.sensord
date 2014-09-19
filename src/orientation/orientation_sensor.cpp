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

#define SENSOR_NAME "ORIENTATION_SENSOR"

#define ACCELEROMETER_ENABLED 0x01
#define GYROSCOPE_ENABLED 0x02
#define GEOMAGNETIC_ENABLED 0x04
#define ORIENTATION_ENABLED 7

#define INITIAL_VALUE -1
#define INITIAL_TIME 0

// Below Defines const variables to be input from sensor config files once code is stabilized
#define SAMPLING_TIME 100
#define MS_TO_US 1000

float bias_accel[] = {0.098586, 0.18385, (10.084 - GRAVITY)};
float bias_gyro[] = {-5.3539, 0.24325, 2.3391};
float bias_magnetic[] = {0, 0, 0};
int sign_accel[] = {+1, +1, +1};
int sign_gyro[] = {+1, +1, +1};
int sign_magnetic[] = {+1, +1, +1};
float scale_accel = 1;
float scale_gyro = 575;
float scale_magnetic = 1;

int pitch_phase_compensation = -1;
int roll_phase_compensation = -1;
int yaw_phase_compensation = -1;
int magnetic_alignment_factor = -1;

void pre_process_data(sensor_data<float> &data_out, sensor_data_t &data_in, float *bias, int *sign, float scale)
{
	data_out.m_data.m_vec[0] = sign[0] * (data_in.values[0] - bias[0]) / scale;
	data_out.m_data.m_vec[1] = sign[1] * (data_in.values[1] - bias[1]) / scale;
	data_out.m_data.m_vec[2] = sign[2] * (data_in.values[2] - bias[2]) / scale;

	data_out.m_time_stamp = data_in.timestamp;
}

orientation_sensor::orientation_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_roll(INITIAL_VALUE)
, m_pitch(INITIAL_VALUE)
, m_yaw(INITIAL_VALUE)
{
	m_name = string(SENSOR_NAME);
	register_supported_event(ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_interval = SAMPLING_TIME * MS_TO_US;
	m_timestamp = get_timestamp();
	m_enable_orientation = 0;
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

	if (!m_accel_sensor || !m_gyro_sensor || !m_magnetic_sensor) {
		ERR("Failed to load sensors,  accel: 0x%x, gyro: 0x%x, mag: 0x%x",
			m_accel_sensor, m_gyro_sensor, m_magnetic_sensor);
		return false;
	}

	INFO("%s is created!", sensor_base::get_name());
	return true;
}


sensor_type_t orientation_sensor::get_type(void)
{
	return ORIENTATION_SENSOR;
}

bool orientation_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_client(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_accel_sensor->start();
	m_gyro_sensor->add_client(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_gyro_sensor->start();
	m_magnetic_sensor->add_client(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_magnetic_sensor->start();

	activate();
	return true;
}

bool orientation_sensor::on_stop(void)
{
	m_accel_sensor->delete_client(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_accel_sensor->stop();
	m_gyro_sensor->delete_client(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_gyro_sensor->stop();
	m_magnetic_sensor->delete_client(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_magnetic_sensor->stop();

	deactivate();
	return true;
}

bool orientation_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_interval(client_id, interval, true);
	m_gyro_sensor->add_interval(client_id, interval, true);
	m_magnetic_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, true);
}

bool orientation_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->delete_interval(client_id, true);
	m_gyro_sensor->delete_interval(client_id, true);
	m_magnetic_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, true);
}

void orientation_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	const float MIN_DELIVERY_DIFF_FACTOR = 0.75f;
	unsigned long long diff_time;

	sensor_data<float> accel;
	sensor_data<float> gyro;
	sensor_data<float> magnetic;

	sensor_data_t accel_data;
	sensor_data_t gyro_data;
	sensor_data_t mag_data;

	sensor_event_t orientation_event;
	euler_angles<float> euler_orientation;

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		diff_time = event.data.timestamp - m_timestamp;

		if (m_timestamp && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(accel, accel_data, bias_accel, sign_accel, scale_accel);

		m_enable_orientation |= ACCELEROMETER_ENABLED;
	}
	else if (event.event_type == GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME) {
		diff_time = event.data.timestamp - m_timestamp;

		if (m_timestamp && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(gyro, gyro_data, bias_gyro, sign_gyro, scale_gyro);

		m_enable_orientation |= GYROSCOPE_ENABLED;
	}
	else if (event.event_type == GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME) {
		diff_time = event.data.timestamp - m_timestamp;

		if (m_timestamp && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(magnetic, magnetic_data, bias_magnetic, sign_magnetic, scale_magnetic);

		m_enable_orientation |= GEOMAGNETIC_ENABLED;
	}

	if (m_enable_orientation == ORIENTATION_ENABLED) {
		m_enable_orientation = 0;
		m_timestamp = get_timestamp();

		m_orientation.m_pitch_phase_compensation = pitch_phase_compensation;
		m_orientation.m_roll_phase_compensation = roll_phase_compensation;
		m_orientation.m_yaw_phase_compensation = yaw_phase_compensation;
		m_orientation.m_magnetic_alignment_factor = magnetic_alignment_factor;

		euler_orientation = m_orientation.get_orientation(accel, gyro, magnetic);

		orientation_event.data.data_accuracy = SENSOR_ACCURACY_GOOD;
		orientation_event.data.data_unit_idx = SENSOR_UNIT_DEGREE;
		orientation_event.event_type = ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
		orientation_event.data.timestamp = m_timestamp;
		orientation_event.data.values_num = 3;
		orientation_event.data.values[0] = euler_orientation.m_ang.m_vec[0];
		orientation_event.data.values[1] = euler_orientation.m_ang.m_vec[1];
		orientation_event.data.values[2] = euler_orientation.m_ang.m_vec[2];

		outs.push_back(orientation_event);
	}

	return;
}

int orientation_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data<float> accel;
	sensor_data<float> gyro;
	sensor_data<float> magnetic;

	sensor_data_t accel_data;
	sensor_data_t gyro_data;
	sensor_data_t magnetic_data;

	euler_angles<float> euler_orientation;

	if (event_type != ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME)
		return -1;

	m_accel_sensor->get_sensor_data(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, accel_data);
	m_gyro_sensor->get_sensor_data(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, gyro_data);
	m_magnetic_sensor->get_sensor_data(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, magnetic_data);

	pre_process_data(accel, accel_data, bias_accel, sign_accel, scale_accel);
	pre_process_data(gyro, gyro_data, bias_gyro, sign_gyro, scale_gyro);
	pre_process_data(magnetic, magnetic_data, bias_magnetic, sign_magnetic, scale_magnetic);

	m_orientation.m_pitch_phase_compensation = pitch_phase_compensation;
	m_orientation.m_roll_phase_compensation = roll_phase_compensation;
	m_orientation.m_yaw_phase_compensation = yaw_phase_compensation;
	m_orientation.m_magnetic_alignment_factor = magnetic_alignment_factor;

	euler_orientation = m_orientation.get_orientation(accel, gyro, magnetic);

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_DEGREE;
	data.timestamp = get_timestamp();
	data.values[0] = euler_orientation.m_ang.m_vec[0];
	data.values[1] = euler_orientation.m_ang.m_vec[1];
	data.values[2] = euler_orientation.m_ang.m_vec[2];
	data.values_num = 3;

	return 0;
}

bool orientation_sensor::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_DEGREE;
	properties.sensor_min_range = -180;
	properties.sensor_max_range = 180;
	properties.sensor_resolution = 1;

	strncpy(properties.sensor_vendor, "Samsung", MAX_KEY_LENGTH);
	strncpy(properties.sensor_name, SENSOR_NAME, MAX_KEY_LENGTH);

	return true;
}

extern "C" void *create(void)
{
	orientation_sensor *inst;

	try {
		inst = new orientation_sensor();
	} catch (int err) {
		ERR("orientation_sensor class create fail , errno : %d , errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (orientation_sensor *)inst;
}
