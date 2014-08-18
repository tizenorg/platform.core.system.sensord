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
#include <sensor.h>
#include <common.h>
#include <sf_common.h>
#include <orientation_sensor.h>
#include <sensor_plugin_loader.h>
#include <orientation_filter.h>

#define SENSOR_NAME "Orientation"

void copy_sensor_data(sensor_data<float> &data_out, sensor_data_t &data_in)
{
	data_out.m_data.m_vec[0] = data_in.values[0];
	data_out.m_data.m_vec[1] = data_in.values[1];
	data_out.m_data.m_vec[2] = data_in.values[2];
	data_out.m_time_stamp = data_in.timestamp;
}

orientation_sensor::orientation_sensor()
:m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_roll(-1)
, m_pitch(-1)
, m_yaw(-1)
, m_timestamp(-1)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME);
	register_supported_event(ORIENTATION_EVENT_CALIBRATION_NEEDED);
}

orientation_sensor::~orientation_sensor()
{
}

bool orientation_sensor::init(void)
{
	m_accel_sensor = sensor_plugin_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_gyro_sensor = sensor_plugin_loader::get_instance().get_sensor(GYROSCOPE_SENSOR);
	m_magnetic_sensor = sensor_plugin_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	if (!m_accel_sensor || !m_gyro_sensor || !m_magnetic_sensor) {
		ERR("Fail to load sensors,  accel: 0x%x, gyro: 0x%x, mag: 0x%x",
			m_accel_sensor, m_gyro_sensor, m_magnetic_sensor);
		return false;
	}

	INFO("%s is created!", sensor_base::get_name());
	return true;
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
	return true;
}

bool orientation_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_interval(client_id, interval, true);
	m_gyro_sensor->add_interval(client_id, interval, true);
	m_magnetic_sensor->add_interval(client_id, interval, true);
	return true;
}

bool orientation_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->delete_interval(client_id, true);
	m_gyro_sensor->delete_interval(client_id, true);
	m_magnetic_sensor->delete_interval(client_id, true);
	return true;
}

bool orientation_sensor::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNDEFINED_UNIT;
	properties.sensor_min_range = 0;
	properties.sensor_max_range = 1;
	properties.sensor_resolution = 1;
	strncpy(properties.sensor_vendor, "Samsung", MAX_KEY_LENGTH);
	strncpy(properties.sensor_name, SENSOR_NAME, MAX_KEY_LENGTH);
	return true;
}

void orientation_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_data<float> accel;
	sensor_data<float> gyro;
	sensor_data<float> magnetic;

	sensor_data_t accel_data;
	sensor_data_t gyro_data;
	sensor_data_t mag_data;

	sensor_event_t orientation_event;
	unsigned long long timestamp = get_timestamp();

	if (event.event_type == ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME) {

		m_accel_sensor->get_sensor_data(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, accel_data);
		m_gyro_sensor->get_sensor_data(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, gyro_data);
		m_magnetic_sensor->get_sensor_data(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, mag_data);

		copy_sensor_data(accel, accel_data);
		copy_sensor_data(gyro, gyro_data);
		copy_sensor_data(magnetic, mag_data);

		orientation.get_orientation(accel, gyro, magnetic);

		orientation_event.data.data_accuracy = SENSOR_ACCURACY_GOOD;
		orientation_event.data.data_unit_idx = SENSOR_UNIT_DEGREE;
		orientation_event.event_type = ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
		orientation_event.data.timestamp = timestamp;
		orientation_event.data.values_num = 3;
		orientation_event.data.values[0] = orientation.m_orientation.m_ang.m_vec[2];
		orientation_event.data.values[1] = orientation.m_orientation.m_ang.m_vec[1];
		orientation_event.data.values[2] = orientation.m_orientation.m_ang.m_vec[0];
		outs.push_back(orientation_event);

		AUTOLOCK(m_mutex);
		m_roll = orientation.m_orientation.m_ang.m_vec[0];
		m_pitch = orientation.m_orientation.m_ang.m_vec[1];
		m_yaw = orientation.m_orientation.m_ang.m_vec[2];
		m_timestamp = timestamp;

		return;
	}
}

int orientation_sensor::get_sensor_data(const unsigned int data_id, sensor_data_t &data)
{
	sensor_data<float> accel;
	sensor_data<float> gyro;
	sensor_data<float> magnetic;

	sensor_data_t accel_data;
	sensor_data_t gyro_data;
	sensor_data_t mag_data;

	if (data_id != ORIENTATION_BASE_DATA_SET)
		return -1;

	m_accel_sensor->get_sensor_data(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, accel_data);
	m_gyro_sensor->get_sensor_data(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, gyro_data);
	m_magnetic_sensor->get_sensor_data(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, mag_data);

	copy_sensor_data(accel, accel_data);
	copy_sensor_data(gyro, gyro_data);
	copy_sensor_data(magnetic, mag_data);

	orientation.get_orientation(accel, gyro, magnetic);

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_DEGREE;
	data.time_stamp = get_timestamp();
	data.values[0] = orientation.m_orientation.m_ang.m_vec[2];
	data.values[1] = orientation.m_orientation.m_ang.m_vec[1];
	data.values[2] = orientation.m_orientation.m_ang.m_vec[0];
	data.values_num = 3;
	return 0;
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
