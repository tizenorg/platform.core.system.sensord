/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <virtual_sensor.h>
#include <gesture_movement_sensor.h>
#include <sensor_loader.h>
#include <virtual_sensor_config.h>
#include <gesture_movement_alg.h>

using std::bind1st;
using std::mem_fun;
using std::string;
using std::vector;

#define SENSOR_NAME						"GESTURE_MOVEMENT_SENSOR"
#define SENSOR_TYPE_GESTURE_MOVEMENT	"GESTURE_MOVEMENT"

#define MS_TO_US 1000

#define ELEMENT_NAME					"NAME"
#define ELEMENT_VENDOR					"VENDOR"
#define ELEMENT_RAW_DATA_UNIT			"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME	"DEFAULT_SAMPLING_TIME"

gesture_movement_sensor::gesture_movement_sensor()
: m_accel_sensor(NULL)
, m_alg(NULL)
, m_gesture_movement(GESTURE_MOVEMENT_NONE)
, m_interval(1)
, m_gesture_movement_time(1) // gesture_movement state is valid from initial state, so set gesture_movement time to non-zero value
, m_default_sampling_time(1)
{
	virtual_sensor_config &config = virtual_sensor_config::get_instance();

	if (!config.get(SENSOR_TYPE_GESTURE_MOVEMENT, ELEMENT_VENDOR, m_vendor)) {
		_E("[VENDOR] is empty\n");
		throw ENXIO;
	}

	_I("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_GESTURE_MOVEMENT, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		_E("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	_I("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_GESTURE_MOVEMENT, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		_E("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	_I("m_default_sampling_time = %d", m_default_sampling_time);

	m_interval = m_default_sampling_time * MS_TO_US;
}

gesture_movement_sensor::~gesture_movement_sensor()
{
	delete m_alg;

	_I("gesture_movement_sensor is destroyed!\n");
}

bool gesture_movement_sensor::init(void)
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		_E("cannot load accel sensor_hal from %s", get_name());
		return false;
	}

	m_alg = get_alg();

	if (!m_alg) {
		_E("Not supported GESTURE MOVEMENT sensor");
		return false;
	}

	_I("%s is created!\n", get_name());

	return true;
}

sensor_type_t gesture_movement_sensor::get_type(void)
{
	return GESTURE_MOVEMENT_SENSOR;
}

unsigned int gesture_movement_sensor::get_event_type(void)
{
	return (GESTURE_MOVEMENT_SENSOR << 16) | 0x0001;
}

const char* gesture_movement_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool gesture_movement_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name("Gesture Movement Sensor");
	info.set_vendor("Samsung Electronics");
	info.set_min_range(GESTURE_MOVEMENT_NONE);
	info.set_max_range(GESTURE_MOVEMENT_DETECTION);
	info.set_resolution(1);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void gesture_movement_sensor::synthesize(const sensor_event_t& event)
{
	if (event.event_type == ACCELEROMETER_RAW_DATA_EVENT) {
		int gesture_movement;
		float acc[3];

		acc[0] = event.data->values[0];
		acc[1] = event.data->values[1];
		acc[2] = event.data->values[2];

		if (!m_alg->get_gesture_movement(acc, event.data->timestamp, m_gesture_movement, gesture_movement))
			return;

		m_gesture_movement = gesture_movement;
		m_gesture_movement_time = event.data->timestamp;

		sensor_event_t *gesture_movement_event;
		sensor_data_t *gesture_movement_data;
		int data_length;
		int remains;

		gesture_movement_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
		remains = get_data(&gesture_movement_data, &data_length);

		if (remains < 0)
			return;

		gesture_movement_event->sensor_id = get_id();
		gesture_movement_event->event_type = GESTURE_MOVEMENT_CHANGE_STATE_EVENT;
		gesture_movement_event->data_length = data_length;
		gesture_movement_event->data = gesture_movement_data;

		push(gesture_movement_event);

//		_D("Gesture Movement: %d, ACC[0]: %f, ACC[1]: %f, ACC[2]: %f", gesture_movement, event.data->values[0], event.data->values[1], event.data->values[2]);
		_E("@@@@@ Gesture Movement: %d, ACC[0]: %f, ACC[1]: %f, ACC[2]: %f", gesture_movement, event.data->values[0], event.data->values[1], event.data->values[2]);
	} else {
		return;
	}
}

int gesture_movement_sensor::get_data(sensor_data_t **data, int *length)
{
	/* if It is batch sensor, remains can be 2+ */
	int remains = 1;

	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_gesture_movement_time;
	sensor_data->values[0] = m_gesture_movement;
	sensor_data->value_count = 1;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return --remains;
}

bool gesture_movement_sensor::set_interval(unsigned long interval)
{
	return false;
}

bool gesture_movement_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool gesture_movement_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool gesture_movement_sensor::on_start(void)
{
	m_gesture_movement = GESTURE_MOVEMENT_NONE;

	m_accel_sensor->add_interval((intptr_t)this , (m_interval/MS_TO_US), true);
	m_accel_sensor->start();

	return activate();
}

bool gesture_movement_sensor::on_stop(void)
{
	m_accel_sensor->delete_interval((intptr_t)this , true);
	m_accel_sensor->stop();

	return deactivate();
}

gesture_movement_alg *gesture_movement_sensor::get_alg()
{
	gesture_movement_alg *alg = new(std::nothrow) gesture_movement_alg();
	retvm_if(!alg, NULL, "Failed to allocate memory");

	return alg;
}

