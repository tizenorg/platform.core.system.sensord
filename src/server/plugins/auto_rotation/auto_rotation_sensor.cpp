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

#include <virtual_sensor.h>
#include <auto_rotation_sensor.h>
#include <sensor_loader.h>
#include <virtual_sensor_config.h>
#include <auto_rotation_alg.h>
#include <auto_rotation_alg_emul.h>

using std::bind1st;
using std::mem_fun;
using std::string;
using std::vector;

#define SENSOR_NAME						"AUTO_ROTATION_SENSOR"
#define SENSOR_TYPE_AUTO_ROTATION		"AUTO_ROTATION"

#define MS_TO_US 1000

#define ELEMENT_NAME					"NAME"
#define ELEMENT_VENDOR					"VENDOR"
#define ELEMENT_RAW_DATA_UNIT			"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME	"DEFAULT_SAMPLING_TIME"

#define AUTO_ROTATION_LIB				"/usr/lib/sensord/libauto_rotation_sensor.so"

auto_rotation_sensor::auto_rotation_sensor()
: m_accel_sensor(NULL)
, m_alg(NULL)
, m_rotation(0)
, m_interval(1)
, m_rotation_time(1) // rotation state is valid from initial state, so set rotation time to non-zero value
, m_default_sampling_time(1)
{
	virtual_sensor_config &config = virtual_sensor_config::get_instance();

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_VENDOR, m_vendor)) {
		_E("[VENDOR] is empty\n");
		throw ENXIO;
	}

	_I("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		_E("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	_I("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_AUTO_ROTATION, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		_E("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	_I("m_default_sampling_time = %d", m_default_sampling_time);

	m_interval = m_default_sampling_time * MS_TO_US;
}

auto_rotation_sensor::~auto_rotation_sensor()
{
	delete m_alg;

	_I("auto_rotation_sensor is destroyed!\n");
}

bool auto_rotation_sensor::init(void)
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		_E("cannot load accel sensor_hal from %s", get_name());
		return false;
	}

	m_alg = get_alg();

	if (!m_alg) {
		_E("Not supported AUTO ROTATION sensor");
		return false;
	}

	if (!m_alg->open())
		return false;

	_I("%s is created!\n", get_name());

	return true;
}

sensor_type_t auto_rotation_sensor::get_type(void)
{
	return AUTO_ROTATION_SENSOR;
}

unsigned int auto_rotation_sensor::get_event_type(void)
{
	return (AUTO_ROTATION_SENSOR << 16) | 0x0001;
}

const char* auto_rotation_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool auto_rotation_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name("Auto Rotation Sensor");
	info.set_vendor("Samsung Electronics");
	info.set_min_range(AUTO_ROTATION_DEGREE_UNKNOWN);
	info.set_max_range(AUTO_ROTATION_DEGREE_270);
	info.set_resolution(1);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void auto_rotation_sensor::synthesize(const sensor_event_t& event)
{
	if (event.event_type != ACCELEROMETER_RAW_DATA_EVENT)
		return;

	int rotation;
	float acc[3];
	acc[0] = event.data->values[0];
	acc[1] = event.data->values[1];
	acc[2] = event.data->values[2];

	if (!m_alg->get_rotation(acc, event.data->timestamp, m_rotation, rotation))
		return;

	m_rotation = rotation;
	m_rotation_time = event.data->timestamp;

	sensor_event_t *rotation_event;
	sensor_data_t *rotation_data;
	int data_length;
	int remains;

	rotation_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	remains = get_data(&rotation_data, &data_length);

	if (remains < 0)
		return;

	rotation_event->sensor_id = get_id();
	rotation_event->event_type = AUTO_ROTATION_CHANGE_STATE_EVENT;
	rotation_event->data_length = data_length;
	rotation_event->data = rotation_data;

	push(rotation_event);

	_D("Rotation: %d, ACC[0]: %f, ACC[1]: %f, ACC[2]: %f", rotation, event.data->values[0], event.data->values[1], event.data->values[2]);
	return;
}

int auto_rotation_sensor::get_data(sensor_data_t **data, int *length)
{
	/* if It is batch sensor, remains can be 2+ */
	int remains = 1;

	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = SENSOR_ACCURACY_GOOD;
	sensor_data->timestamp = m_rotation_time;
	sensor_data->values[0] = m_rotation;
	sensor_data->value_count = 1;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return --remains;
}

bool auto_rotation_sensor::set_interval(unsigned long interval)
{
	return false;
}

bool auto_rotation_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool auto_rotation_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool auto_rotation_sensor::on_start(void)
{
	m_rotation = AUTO_ROTATION_DEGREE_UNKNOWN;

	m_alg->start();

	m_accel_sensor->add_interval((intptr_t)this , (m_interval/MS_TO_US), true);
	m_accel_sensor->start();

	return activate();
}

bool auto_rotation_sensor::on_stop(void)
{
	m_accel_sensor->delete_interval((intptr_t)this , true);
	m_accel_sensor->stop();

	return deactivate();
}

bool auto_rotation_sensor::check_lib(void)
{
	if (access(AUTO_ROTATION_LIB, F_OK) < 0)
		return false;

	return true;
}

auto_rotation_alg *auto_rotation_sensor::get_alg()
{
	auto_rotation_alg *alg = new(std::nothrow) auto_rotation_alg_emul();
	retvm_if(!alg, NULL, "Failed to allocate memory");

	return alg;
}

