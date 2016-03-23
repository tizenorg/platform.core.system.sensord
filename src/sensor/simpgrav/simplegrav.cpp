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

#include <sensor_log.h>
#include <sensor_types.h>

#include <sensor_common.h>
#include <virtual_sensor.h>
#include <simpgrav_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SIMPGRAV_SENSOR"

#define SIMPGRAV 9.80665

#define PHASE_ACCEL_READY 0x01
#define PHASE_GYRO_READY 0x02
#define PHASE_FUSION_READY 0x03
#define US_PER_SEC 1000000
#define TAU_LOW 0.4
#define TAU_MID 0.75
#define TAU_HIGH 0.99

#define NORM(x, y, z) sqrt((x)*(x) + (y)*(y) + (z)*(z))

simpgrav_sensor::simpgrav_sensor()
: m_accel_sensor(NULL)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_accuracy(-1)
, m_time(0)
{
}

simpgrav_sensor::~simpgrav_sensor()
{
	_I("simpgrav_sensor is destroyed!\n");
}

bool simpgrav_sensor::init()
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);

	if (!m_accel_sensor) {
		_E("cannot load accelerometer sensor_hal[%s]", get_name());
		return false;
	}

	_I("%s (%s) is created!\n", get_name());
	return true;
}

sensor_type_t simpgrav_sensor::get_type(void)
{
	return SIMPGRAV_SENSOR;
}

unsigned int simpgrav_sensor::get_event_type(void)
{
	return SIMPGRAV_EVENT_RAW_DATA_REPORT_ON_TIME;
}

const char* simpgrav_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool simpgrav_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name("simpgrav Sensor");
	info.set_vendor("Samsung Electronics");
	info.set_min_range(-19.6);
	info.set_max_range(19.6);
	info.set_resolution(0.01);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void simpgrav_sensor::synthesize(const sensor_event_t& event)
{
	synthesize_lowpass(event);
}

void simpgrav_sensor::synthesize_lowpass(const sensor_event_t& event)
{
	sensor_event_t *simpgrav_event;
	float x, y, z, norm, alpha, tau, err;

	norm = NORM(event.data->values[0], event.data->values[1], event.data->values[2]);
	//x = event.data->values[0] / norm * SIMPGRAV;
	//y = event.data->values[1] / norm * SIMPGRAV;
	//z = event.data->values[2] / norm * SIMPGRAV;
	x = event.data->values[0];
	y = event.data->values[1];
	z = event.data->values[2];

	if (m_time > 0) {
		err = fabs(norm - SIMPGRAV) / SIMPGRAV;
		tau = (err < 0.1 ? TAU_LOW : err > 0.9 ? TAU_HIGH : TAU_MID);
		alpha = tau / (tau + (float)(event.data->timestamp - m_time) / US_PER_SEC);
		x = alpha * m_x + (1 - alpha) * x;
		y = alpha * m_y + (1 - alpha) * y;
		z = alpha * m_z + (1 - alpha) * z;
		norm = NORM(x, y, z);
		x = x / norm * SIMPGRAV;
		y = y / norm * SIMPGRAV;
		z = z / norm * SIMPGRAV;
	}

	simpgrav_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!simpgrav_event) {
		_E("Failed to allocate memory");
		return;
	}
	simpgrav_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!simpgrav_event->data) {
		_E("Failed to allocate memory");
		return;
	}

	simpgrav_event->sensor_id = get_id();
	simpgrav_event->event_type = SIMPGRAV_EVENT_RAW_DATA_REPORT_ON_TIME;
	simpgrav_event->data_length = sizeof(sensor_data_t);
	simpgrav_event->data->accuracy = event.data->accuracy;
	simpgrav_event->data->timestamp = event.data->timestamp;
	simpgrav_event->data->value_count = 3;
	simpgrav_event->data->values[0] = x;
	simpgrav_event->data->values[1] = y;
	simpgrav_event->data->values[2] = z;
	push(simpgrav_event);

	m_time = event.data->timestamp;
	m_x = x;
	m_y = y;
	m_z = z;
	m_accuracy = event.data->accuracy;
}

int simpgrav_sensor::get_data(sensor_data_t **data, int *length)
{
	sensor_data_t *sensor_data;
	sensor_data = (sensor_data_t *)malloc(sizeof(sensor_data_t));

	sensor_data->accuracy = m_accuracy;
	sensor_data->timestamp = m_time;
	sensor_data->value_count = 3;
	sensor_data->values[0] = m_x;
	sensor_data->values[1] = m_y;
	sensor_data->values[2] = m_z;

	*data = sensor_data;
	*length = sizeof(sensor_data_t);

	return 0;
}

bool simpgrav_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool simpgrav_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool simpgrav_sensor::on_start(void)
{
	if (m_accel_sensor)
		m_accel_sensor->start();

	m_time = 0;

	return activate();
}

bool simpgrav_sensor::on_stop(void)
{
	if (m_accel_sensor)
		m_accel_sensor->stop();

	m_time = 0;

	return deactivate();
}

bool simpgrav_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool simpgrav_sensor::delete_interval(int client_id, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}

bool simpgrav_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}
