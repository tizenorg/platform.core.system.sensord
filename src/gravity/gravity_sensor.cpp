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

#include <sensor_base.h>
#include <virtual_sensor.h>
#include <gravity_sensor.h>
#include <sensor_plugin_loader.h>

#define TIME_CONSTANT  0.18
#define GRAVITY 9.80665

#define SENSOR_NAME "GRAVITY_SENSOR"

gravity_sensor::gravity_sensor()
: m_accel_sensor(NULL)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_time(0)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME);
}

gravity_sensor::~gravity_sensor()
{
	INFO("gravity_sensor is destroyed!\n");
}

bool gravity_sensor::init()
{
	m_accel_sensor = sensor_plugin_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		ERR("cannot load accel sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	INFO("%s is created!\n", sensor_base::get_name());

	return true;
}

sensor_type_t gravity_sensor::get_type(void)
{
	return GRAVITY_SENSOR;
}

bool gravity_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->add_client(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_accel_sensor->start();

	activate();
	return true;
}

bool gravity_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	m_accel_sensor->delete_client(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_accel_sensor->stop();

	deactivate();
	return true;
}

bool gravity_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	m_accel_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, true);
}

bool gravity_sensor::delete_interval(int client_id, bool is_processor)
{
	m_accel_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, true);
}

void gravity_sensor::synthesize(const sensor_event_t& event, vector<sensor_event_t> &outs)
{
	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		float x, y, z;

		calibrate_gravity(event, x, y, z);

		sensor_event_t event;

		event.event_type = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
		event.data.data_accuracy = SENSOR_ACCURACY_GOOD;
		event.data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
		event.data.timestamp = m_time;
		event.data.values_num = 3;
		event.data.values[0] = x;
		event.data.values[1] = y;
		event.data.values[2] = z;
		outs.push_back(event);

		AUTOLOCK(m_value_mutex);
		m_x = x;
		m_y = y;
		m_z = z;

		return;
	}

}

int gravity_sensor::get_sensor_data(const unsigned int data_id, sensor_data_t &data)
{
	if (data_id != GRAVITY_BASE_DATA_SET)
		return -1;

	AUTOLOCK(m_value_mutex);

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
	data.time_stamp = m_time;
	data.values_num = 3;
	data.values[0] = m_x;
	data.values[1] = m_y;
	data.values[2] = m_z;

	return 0;
}

bool gravity_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	m_accel_sensor->get_properties(type, properties);

	if (type != GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME)
		return true;

	properties.sensor_min_range = properties.sensor_min_range / GRAVITY;
	properties.sensor_max_range = properties.sensor_max_range / GRAVITY;
	properties.sensor_resolution = properties.sensor_resolution / GRAVITY;
	strncpy(properties.sensor_name, "Gravity Sensor", MAX_KEY_LENGTH);

	return true;
}

void gravity_sensor::calibrate_gravity(const sensor_event_t &raw, float &x, float &y, float &z)
{
	unsigned long long timestamp;
	float dt;
	float alpha;
	float last_x = 0, last_y = 0, last_z = 0;

	{
		AUTOLOCK(m_value_mutex);
		last_x = m_x;
		last_y = m_y;
		last_z = m_z;
	}

	timestamp = get_timestamp();
	dt = (timestamp - m_time) / 1000000.0f;
	m_time = timestamp;

	alpha = TIME_CONSTANT / (TIME_CONSTANT + dt);

	if (dt > 1.0)
		alpha = 0.0;

	x = (alpha * last_x) + ((1 - alpha) * raw.data.values[0]/GRAVITY);
	y = (alpha * last_y) + ((1 - alpha) * raw.data.values[1]/GRAVITY);
	z = (alpha * last_z) + ((1 - alpha) * raw.data.values[2]/GRAVITY);
}

extern "C" void *create(void)
{
	gravity_sensor *inst;

	try {
		inst = new gravity_sensor();
	} catch (int ErrNo) {
		ERR("gravity_sensor class create fail , errno : %d , errstr : %s\n", errno, strerror(errno));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (gravity_sensor*)inst;;
}
