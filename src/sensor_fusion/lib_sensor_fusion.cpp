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
#include <lib_sensor_fusion.h>
#include <sensor_plugin_loader.h>

#define SENSOR_NAME "Sensor Fusion"

lib_sensor_fusion::lib_sensor_fusion()
{
	m_name = string(SENSOR_NAME);
}

lib_sensor_fusion::~lib_sensor_fusion()
{
}

bool lib_sensor_fusion::init(void)
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

bool lib_sensor_fusion::on_start(void)
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

bool lib_sensor_fusion::on_stop(void)
{
	m_accel_sensor->delete_client(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_accel_sensor->stop();
	m_gyro_sensor->delete_client(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_gyro_sensor->stop();
	m_magnetic_sensor->delete_client(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_magnetic_sensor->stop();
	return true;
}

bool lib_sensor_fusion::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_interval(client_id, interval, true);
	m_gyro_sensor->add_interval(client_id, interval, true);
	m_magnetic_sensor->add_interval(client_id, interval, true);
	return true;
}

bool lib_sensor_fusion::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->delete_interval(client_id, true);
	m_gyro_sensor->delete_interval(client_id, true);
	m_magnetic_sensor->delete_interval(client_id, true);
	return true;
}

bool lib_sensor_fusion::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNDEFINED_UNIT;
	properties.sensor_min_range = 0;
	properties.sensor_max_range = 1;
	properties.sensor_resolution = 1;
	strncpy(properties.sensor_vendor, "Samsung", MAX_KEY_LENGTH);
	strncpy(properties.sensor_name, SENSOR_NAME, MAX_KEY_LENGTH);
	return true;
}

void lib_sensor_fusion::fuse(const sensor_event_t &event)
{
	return;
}

bool lib_sensor_fusion::get_rotation_matrix(arr33_t &rot)
{
	return true;
}

bool lib_sensor_fusion::get_attitude(float &x, float &y, float &z, float &w)
{
	return true;
}

bool lib_sensor_fusion::get_gyro_bias(float &x, float &y, float &z)
{
	return true;
}

bool lib_sensor_fusion::get_rotation_vector(float &x, float &y, float &z, float &w, float &heading_accuracy)
{
	return true;
}

bool lib_sensor_fusion::get_linear_acceleration(float &x, float &y, float &z)
{
	return true;
}

bool lib_sensor_fusion::get_gravity(float &x, float &y, float &z)
{
	return true;
}

bool lib_sensor_fusion::get_rotation_vector_6axis(float &x, float &y, float &z, float &w, float &heading_accuracy)
{
	return true;
}

bool lib_sensor_fusion::get_geomagnetic_rotation_vector(float &x, float &y, float &z, float &w)
{
	return true;
}

bool lib_sensor_fusion::get_orientation(float &azimuth, float &pitch, float &roll)
{
	return true;
}

extern "C" void *create(void)
{
	lib_sensor_fusion *inst;

	try {
		inst = new lib_sensor_fusion();
	} catch (int err) {
		ERR("lib_sensor_fusion class create fail , errno : %d , errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (lib_sensor_fusion *)inst;
}
