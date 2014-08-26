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
#include <gravity_sensor.h>
#include <sensor_plugin_loader.h>

#define INITIAL_VALUE -1
#define INITIAL_TIME 0
#define GRAVITY 9.80665

#define SENSOR_NAME "GRAVITY_SENSOR"

gravity_sensor::gravity_sensor()
: m_orientation_sensor(NULL)
, m_x(INITIAL_VALUE)
, m_y(INITIAL_VALUE)
, m_z(INITIAL_VALUE)
, m_timestamp(INITIAL_TIME)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME);
}

gravity_sensor::~gravity_sensor()
{
	INFO("gravity_sensor is destroyed!");
}

bool gravity_sensor::init()
{
	m_orientation_sensor = sensor_plugin_loader::get_instance().get_sensor(ORIENTATION_SENSOR);

	if (!m_orientation_sensor) {
		ERR("Failed to load orientation sensor: 0x%x", m_orientation_sensor);
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

	m_orientation_sensor->add_client(ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_orientation_sensor->start();

	activate();
	return true;
}

bool gravity_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	m_orientation_sensor->delete_client(ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME);
	m_orientation_sensor->stop();

	deactivate();
	return true;
}

bool gravity_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);
	m_orientation_sensor->add_interval(client_id , interval, true);

	return sensor_base::add_interval(client_id, interval, true);
}

bool gravity_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);
	m_orientation_sensor->delete_interval(client_id , true);

	return sensor_base::delete_interval(client_id, true);
}

void gravity_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t gravity_event;
	vector<sensor_event_t> orientation_event;
	((virtual_sensor *)m_orientation_sensor)->synthesize(event, orientation_event);

	if (!orientation_event.empty()) {
		AUTOLOCK(m_mutex);

		m_timestamp = orientation_event[0].data.timestamp;
		gravity_event.data.values[0] = GRAVITY * sin(orientation_event[0].data.values[1]);
		gravity_event.data.values[1] = GRAVITY * sin(orientation_event[0].data.values[0]);
		gravity_event.data.values[2] = GRAVITY * cos(orientation_event[0].data.values[1] *
										orientation_event[0].data.values[0]);
		gravity_event.data.values_num = 3;
		gravity_event.data.timestamp = m_timestamp;
		gravity_event.data.data_accuracy = SENSOR_ACCURACY_GOOD;
		gravity_event.data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;

		outs.push_back(gravity_event);
		return;
	}
}

int gravity_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	sensor_data_t orientation_data;

	if (event_type != GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME)
		return -1;

	m_orientation_sensor->get_sensor_data(ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME, orientation_data);

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
	data.timestamp = orientation_data.timestamp;
	data.values[0] = GRAVITY * sin(orientation_data.values[1]);
	data.values[1] = GRAVITY * sin(orientation_data.values[0]);
	data.values[2] = GRAVITY * cos(orientation_data.values[1] * orientation_data.values[0]);
	data.values_num = 3;

	return 0;
}

bool gravity_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_DEGREE;
	properties.sensor_min_range = -GRAVITY;
	properties.sensor_max_range = GRAVITY;
	properties.sensor_resolution = 1;
	strncpy(properties.sensor_vendor, "Samsung", MAX_KEY_LENGTH);
	strncpy(properties.sensor_name, SENSOR_NAME, MAX_KEY_LENGTH);

	return true;
}

extern "C" void *create(void)
{
	gravity_sensor *inst;

	try {
		inst = new gravity_sensor();
	} catch (int err) {
		ERR("Failed to create gravity_sensor class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (gravity_sensor *)inst;
}
