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

#include <sensor_log.h>
#include <sensor_types.h>

#include <sensor_common.h>
#include <virtual_sensor.h>
#include <rotation_vector_sensor.h>
#include <sensor_loader.h>
#include <fusion_util.h>

#define SENSOR_NAME "SENSOR_ROTATION_VECTOR"

#define NORM(x, y, z) sqrt((x)*(x) + (y)*(y) + (z)*(z))

#define STATE_ACCEL 0x1
#define STATE_MAGNETIC 0x2

rotation_vector_sensor::rotation_vector_sensor()
: m_accel_sensor(NULL)
, m_mag_sensor(NULL)
, m_x(-1)
, m_y(-1)
, m_z(-1)
, m_w(-1)
, m_time(0)
, m_state(0)
{
}

rotation_vector_sensor::~rotation_vector_sensor()
{
	_I("%s is destroyed!", SENSOR_NAME);
}

bool rotation_vector_sensor::init()
{
	m_accel_sensor = sensor_loader::get_instance().get_sensor(ACCELEROMETER_SENSOR);
	m_mag_sensor = sensor_loader::get_instance().get_sensor(GEOMAGNETIC_SENSOR);

	if (!m_accel_sensor || !m_mag_sensor) {
		_E("cannot load sensors[%s]", SENSOR_NAME);
		return false;
	}

	_I("%s is created!", SENSOR_NAME);
	return true;
}

sensor_type_t rotation_vector_sensor::get_type(void)
{
	return ROTATION_VECTOR_SENSOR;
}

unsigned int rotation_vector_sensor::get_event_type(void)
{
	return CONVERT_TYPE_EVENT(ROTATION_VECTOR_SENSOR);
}

const char* rotation_vector_sensor::get_name(void)
{
	return SENSOR_NAME;
}

bool rotation_vector_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC);
	info.set_name(get_name());
	info.set_vendor("Samsung Electronics");
	info.set_min_range(0);
	info.set_max_range(1);
	info.set_resolution(1);
	info.set_min_interval(1);
	info.set_fifo_count(0);
	info.set_max_batch_count(0);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(false);

	return true;
}

void rotation_vector_sensor::synthesize(const sensor_event_t& event)
{
	sensor_event_t *rotation_vector_event;
	float R[9];
	float I[9];
	float quat[4];
	int error;

	if (event.event_type != GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME &&
		event.event_type != ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)
		return;

	if (event.event_type == GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_mag[0] = event.data->values[0];
		m_mag[1] = event.data->values[1];
		m_mag[2] = event.data->values[2];
		m_accuracy = event.data->accuracy;

		m_state |= STATE_MAGNETIC;
	}

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_acc[0] = event.data->values[0];
		m_acc[1] = event.data->values[1];
		m_acc[2] = event.data->values[2];

		m_state |= STATE_ACCEL;
	}

	if (m_state != (STATE_ACCEL | STATE_MAGNETIC))
		return;

	m_state = 0;

	unsigned long long timestamp = event.data->timestamp;

	error = calculate_rotation_matrix(m_acc, m_mag, R, I);
	ret_if(error);

	error = matrix_to_quat(R, quat);
	ret_if(error);

	rotation_vector_event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
	if (!rotation_vector_event) {
		_E("Failed to allocate memory");
		return;
	}
	rotation_vector_event->data = (sensor_data_t *)malloc(sizeof(sensor_data_t));
	if (!rotation_vector_event->data) {
		_E("Failed to allocate memory");
		free(rotation_vector_event);
		return;
	}

	rotation_vector_event->sensor_id = get_id();
	rotation_vector_event->event_type = CONVERT_TYPE_EVENT(ROTATION_VECTOR_SENSOR);
	rotation_vector_event->data_length = sizeof(sensor_data_t);
	rotation_vector_event->data->accuracy = m_accuracy;
	rotation_vector_event->data->timestamp = timestamp;
	rotation_vector_event->data->value_count = 4;
	rotation_vector_event->data->values[0] = quat[0];
	rotation_vector_event->data->values[1] = quat[1];
	rotation_vector_event->data->values[2] = quat[2];
	rotation_vector_event->data->values[3] = quat[3];
	push(rotation_vector_event);

	m_time = timestamp;
	m_x = quat[0];
	m_y = quat[1];
	m_z = quat[2];
	m_w = quat[3];
	m_accuracy = event.data->accuracy;

	_D("[rotation_vector] : [%10f] [%10f] [%10f] [%10f]", m_x, m_y, m_z, m_w);
}

int rotation_vector_sensor::get_data(sensor_data_t **data, int *length)
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

bool rotation_vector_sensor::set_interval(unsigned long interval)
{
	m_interval = interval;
	return true;
}

bool rotation_vector_sensor::set_batch_latency(unsigned long latency)
{
	return false;
}

bool rotation_vector_sensor::on_start(void)
{
	if (m_accel_sensor)
		m_accel_sensor->start();

	if (m_mag_sensor)
		m_mag_sensor->start();

	m_time = 0;
	return activate();
}

bool rotation_vector_sensor::on_stop(void)
{
	if (m_accel_sensor)
		m_accel_sensor->stop();

	if (m_mag_sensor)
		m_mag_sensor->stop();

	m_time = 0;
	m_state = 0;

	return deactivate();
}

bool rotation_vector_sensor::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->add_interval(client_id, interval, true);

	if (m_mag_sensor)
		m_mag_sensor->add_interval(client_id, interval, true);

	return sensor_base::add_interval(client_id, interval, is_processor);
}

bool rotation_vector_sensor::delete_interval(int client_id, bool is_processor)
{
	if (m_accel_sensor)
		m_accel_sensor->delete_interval(client_id, true);

	if (m_mag_sensor)
		m_mag_sensor->delete_interval(client_id, true);

	return sensor_base::delete_interval(client_id, is_processor);
}
