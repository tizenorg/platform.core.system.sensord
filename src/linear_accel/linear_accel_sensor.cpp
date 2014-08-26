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
#include <virtual_sensor.h>
#include <linear_accel_sensor.h>
#include <sensor_plugin_loader.h>

#define SENSOR_NAME "LINEAR_ACCEL_SENSOR"

#define INITIAL_VALUE -1
#define INITIAL_TIME 0
#define GRAVITY 9.80665

linear_accel_sensor::linear_accel_sensor()
: m_gravity_sensor(NULL)
, m_x(INITIAL_VALUE)
, m_y(INITIAL_VALUE)
, m_z(INITIAL_VALUE)
, m_time(INITIAL_TIME)
{
	m_name = string(SENSOR_NAME);
	register_supported_event(LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME);
}

linear_accel_sensor::~linear_accel_sensor()
{
	INFO("linear_accel_sensor is destroyed!");
}

bool linear_accel_sensor::init()
{
	m_gravity_sensor = sensor_plugin_loader::get_instance().get_sensor(GRAVITY_SENSOR);
	m_accel_sensor = sensor_plugin_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor || !m_gravity_sensor) {
		ERR("Failed to load sensors,  accel: 0x%x, gravity: 0x%x",
			m_accel_sensor, m_gravity_sensor);
		return false;
	}

	INFO("%s is created!", sensor_base::get_name());
	return true;
}

sensor_type_t linear_accel_sensor::get_type(void)
{
	return LINEAR_ACCEL_SENSOR;
}

bool linear_accel_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);
	m_gravity_sensor->add_client(GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_gravity_sensor->start();
	activate();
	return true;
}

bool linear_accel_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);
	m_gravity_sensor->delete_client(GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_gravity_sensor->stop();
	deactivate();
	return true;
}

bool linear_accel_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);
	m_gravity_sensor->add_interval(client_id, interval, true);
	return sensor_base::add_interval(client_id, interval, true);
}

bool linear_accel_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);
	m_gravity_sensor->delete_interval(client_id, true);
	return sensor_base::delete_interval(client_id, true);
}

void linear_accel_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	vector<sensor_event_t> gravity_event;
	sensor_event_t lin_accel_event;
	((virtual_sensor *)m_gravity_sensor)->synthesize(event, gravity_event);

	if (!gravity_event.empty() && event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		AUTOLOCK(m_value_mutex);

		lin_accel_event.data.values_num = 3;
		lin_accel_event.data.timestamp = gravity_event[0].data.timestamp;
		lin_accel_event.event_type = LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
		lin_accel_event.data.data_accuracy = SENSOR_ACCURACY_GOOD;
		lin_accel_event.data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
		lin_accel_event.data.values[0] = event.data.values[0] - gravity_event[0].data.values[0];
		lin_accel_event.data.values[1] = event.data.values[1] - gravity_event[0].data.values[1];
		lin_accel_event.data.values[2] = event.data.values[2] - gravity_event[0].data.values[2];
		outs.push_back(lin_accel_event);
	}

	return;
}

int linear_accel_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t gravity_data, accel_data;
	((virtual_sensor *)m_gravity_sensor)->get_sensor_data(GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME, gravity_data);
	m_accel_sensor->get_sensor_data(ACCELEROMETER_BASE_DATA_SET, accel_data);

	if (event_type != LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME)
		return -1;

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
	data.timestamp = gravity_data.timestamp;
	data.values[0] = accel_data.values[0] - gravity_data.values[0];
	data.values[1] = accel_data.values[1] - gravity_data.values[1];
	data.values[2] = accel_data.values[2] - gravity_data.values[2];
	data.values_num = 3;
	return 0;
}

bool linear_accel_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	m_gravity_sensor->get_properties(type, properties);

	if (type != LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME)
		return true;

	strncpy(properties.sensor_name, "Linear Accelerometer Sensor", MAX_KEY_LENGTH);
	return true;
}

extern "C" void *create(void)
{
	linear_accel_sensor *inst;

	try {
		inst = new linear_accel_sensor();
	} catch (int err) {
		ERR("Failed to create linear_accel_sensor class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (linear_accel_sensor *)inst;
}
