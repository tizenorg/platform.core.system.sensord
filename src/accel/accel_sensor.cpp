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

#include <common.h>
#include <sf_common.h>
#include <accel_sensor.h>
#include <sensor_plugin_loader.h>
#include <algorithm>

using std::bind1st;
using std::mem_fun;

#define GRAVITY 9.80665
#define G_TO_MG 1000

#define RAW_DATA_TO_G_UNIT(X) (((float)(X))/((float)G_TO_MG))
#define RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(X) (GRAVITY * (RAW_DATA_TO_G_UNIT(X)))

#define SENSOR_NAME "ACCELEROMETER_SENSOR"

#define ROTATION_CHECK_INTERVAL	200
#define ROTATION_RULE_CNT 4
#define TILT_MIN 30

#define ROTATION_0 0
#define ROTATION_90 90
#define ROTATION_180 180
#define ROTATION_270 270
#define ROTATION_360 360

#define DEGREE_90 90
#define DEGREE_180 180
#define DEGREE_360 360

struct rotation_rule {
	int tilt;
	int angle;
};

struct rotation_rule rot_rule[ROTATION_RULE_CNT] = {
	{40, 80},
	{50, 70},
	{60, 65},
	{90, 60},
};

accel_sensor::accel_sensor()
: m_sensor_hal(NULL)
, m_interval(POLL_1HZ_MS)
{
	m_name = string(SENSOR_NAME);

	vector<unsigned int> supported_events = {
		ACCELEROMETER_EVENT_ROTATION_CHECK,
		ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME,
		ACCELEROMETER_EVENT_ORIENTATION_DATA_REPORT_ON_TIME,
	};

	for_each(supported_events.begin(), supported_events.end(),
			bind1st(mem_fun(&sensor_base::register_supported_event), this));

	physical_sensor::set_poller(accel_sensor::working, this);
}

accel_sensor::~accel_sensor()
{
	INFO("accel_sensor is destroyed!");
}

bool accel_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(ACCELEROMETER_SENSOR);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	sensor_properties_t properties;

	if (m_sensor_hal->get_properties(properties) == false) {
		ERR("sensor->get_properties() is failed!");
		return false;
	}

	m_raw_data_unit = properties.sensor_resolution / GRAVITY * G_TO_MG;

	INFO("m_raw_data_unit accel : [%f]", m_raw_data_unit);
	INFO("%s is created!", sensor_base::get_name());
	return true;
}

sensor_type_t accel_sensor::get_type(void)
{
	return ACCELEROMETER_SENSOR;
}

bool accel_sensor::working(void *inst)
{
	accel_sensor *sensor = (accel_sensor *)inst;
	return sensor->process_event();
}

bool accel_sensor::process_event(void)
{
	sensor_data_t raw_data;

	if (!m_sensor_hal->is_data_ready(true))
		return true;

	m_sensor_hal->get_sensor_data(raw_data);

	AUTOLOCK(m_mutex);
	AUTOLOCK(m_client_info_mutex);

	if (get_client_cnt(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)) {
		sensor_event_t base_event;

		copy_sensor_data(&raw_data, &(base_event.data));

		base_event.event_type = ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME;
		raw_to_base(base_event.data);
		push(base_event);
         
	}

	if (get_client_cnt(ACCELEROMETER_EVENT_ROTATION_CHECK)) {
		if (is_rotation_time()) {
			sensor_data_t base_data;
			float x, y, z;
			int rotation;

			copy_sensor_data(&raw_data, &base_data);
			raw_to_base(base_data);

			x = base_data.values[0];
			y = base_data.values[1];
			z = base_data.values[2];

			rotation = get_rotation_event(x, y, z);

			if (rotation != -1) {
				AUTOLOCK(m_value_mutex);

				sensor_event_t rotation_event;
				rotation_event.event_type = ACCELEROMETER_EVENT_ROTATION_CHECK;
				rotation_event.data.timestamp = raw_data.timestamp;
				rotation_event.data.values_num = 1;
				rotation_event.data.values[0] = rotation;
				push(rotation_event);
                                 

				INFO("Rotation event occurred, rotation value = %d", rotation);
                               
			}
		}
	}

	if (get_client_cnt(ACCELEROMETER_EVENT_ORIENTATION_DATA_REPORT_ON_TIME)) {
		sensor_event_t orientation_event;

		copy_sensor_data(&raw_data, &(orientation_event.data));

		orientation_event.event_type = ACCELEROMETER_EVENT_ORIENTATION_DATA_REPORT_ON_TIME;
		raw_to_orientation(raw_data, orientation_event.data);

		push(orientation_event);
	}

	return true;
}

bool accel_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail");
		return false;
	}

	reset_rotation();

	return start_poll();
}

bool accel_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail");
		return false;
	}

	return stop_poll();
}

bool accel_sensor::add_client(unsigned int event_type)
{
	AUTOLOCK(m_mutex);

	if (!sensor_base::add_client(event_type))
		return false;

	switch (event_type) {
	case ACCELEROMETER_EVENT_ROTATION_CHECK:
		if (get_client_cnt(ACCELEROMETER_EVENT_ROTATION_CHECK) == 1)
			reset_rotation();
		break;
	default:
		break;
	}

	return true;
}

long accel_sensor::set_command(const unsigned int cmd, long value)
{
	if (m_sensor_hal->set_command(cmd, value) < 0) {
		ERR("m_sensor_hal set_cmd fail");
		return -1;
	}

	return 0;
}

bool accel_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	return m_sensor_hal->get_properties(properties);
}

int accel_sensor::get_sensor_data(const unsigned int type, sensor_data_t &data)
{
	if (type == ACCELEROMETER_ROTATION_DATA_SET) {
		AUTOLOCK(m_value_mutex);

		data.data_accuracy = SENSOR_ACCURACY_NORMAL;
		data.data_unit_idx = SENSOR_UNDEFINED_UNIT;
		data.values_num = 1;
		data.values[0] = m_rotation;
		data.timestamp = m_rotation_time;
		return 0;
	}

	if (m_sensor_hal->get_sensor_data(data) < 0) {
		ERR("Failed to get sensor data");
		return -1;
	}

	if (type == ACCELEROMETER_BASE_DATA_SET)
		raw_to_base(data);
	else if (type == ACCELEROMETER_ORIENTATION_DATA_SET) {
		sensor_data_t raw;

		copy_sensor_data(&data, &raw);
		raw_to_orientation(raw, data);
	} else {
		ERR("Does not support type: 0x%x", type);
		return -1;
	}

	return 0;
}

int accel_sensor::get_rotation_event(float x, float y, float z)
{
	int cur_rotation = ROTATION_UNKNOWN;

	double atan_value;
	int acc_theta, acc_pitch;
	double realg;
	bool is_stable = false;
	bool rotation_on = false;
	int tilt, angle;
	int i;

	atan_value = atan2(x, y);
	acc_theta = (int)(atan_value * (RADIAN_VALUE) + DEGREE_360) % DEGREE_360;
	realg = (double)sqrt((x * x) + (y * y) + (z * z));
	acc_pitch = ROTATION_90 - abs((int) (asin(z / realg) * RADIAN_VALUE));

	for (i = 0; i < ROTATION_RULE_CNT; ++i) {
		tilt = rot_rule[i].tilt;

		if ((acc_pitch >= TILT_MIN) && (acc_pitch <= tilt)) {
			if ((m_rotation == ROTATION_EVENT_0) || (m_rotation == ROTATION_EVENT_180))
				angle = rot_rule[i].angle;
			else
				angle = ROTATION_90 - rot_rule[i].angle;

			if ((acc_theta >= ROTATION_360 - angle && acc_theta <= ROTATION_360 - 1) ||
					(acc_theta >= ROTATION_0 && acc_theta <= ROTATION_0 + angle)) {
				cur_rotation = ROTATION_EVENT_0;
			} else if (acc_theta >= ROTATION_0 + angle && acc_theta <= ROTATION_180 - angle) {
				cur_rotation = ROTATION_EVENT_90;
			} else if (acc_theta >= ROTATION_180 - angle && acc_theta <= ROTATION_180 + angle) {
				cur_rotation = ROTATION_EVENT_180;
			} else if (acc_theta >= ROTATION_180 + angle && acc_theta <= ROTATION_360 - angle) {
				cur_rotation = ROTATION_EVENT_270;
			}
			break;
		}
	}

	m_windowing[m_curr_window_count++] = cur_rotation;

	if (m_curr_window_count == MAX_WINDOW_NUM)
		m_curr_window_count = 0;

	for (i = 0; i < MAX_WINDOW_NUM ; i++) {
		if (m_windowing[i] == cur_rotation)
			is_stable = true;
		else {
			is_stable = false;
			break;
		}
	}

	rotation_on = (m_rotation != cur_rotation);

	if (rotation_on && is_stable) {
		m_rotation = cur_rotation;
		m_rotation_time = get_timestamp();
		return m_rotation;
	}

	return -1;
}

void accel_sensor::reset_rotation(void)
{
	int i;

	for (i = 0 ; i < MAX_WINDOW_NUM ; i++)
		m_windowing[i] = 0;

	m_curr_window_count = 0;
	m_rotation = ROTATION_UNKNOWN;
	m_rotation_time = 0;
	m_rotation_check_remained_time = ROTATION_CHECK_INTERVAL;
}

bool accel_sensor::is_rotation_time(void)
{
	AUTOLOCK(m_mutex);
	m_rotation_check_remained_time -= m_interval;

	if (m_rotation_check_remained_time <= 0) {
		m_rotation_check_remained_time = ROTATION_CHECK_INTERVAL;
		return true;
	}

	return false;
}

bool accel_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	m_interval = interval;
	INFO("Polling interval is set to %dms", interval);
	return m_sensor_hal->set_interval(interval);
}

void accel_sensor::raw_to_base(sensor_data_t &data)
{
	data.data_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
	data.values_num = 3;
	data.values[0] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[0] * m_raw_data_unit);
	data.values[1] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[1] * m_raw_data_unit);
	data.values[2] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[2] * m_raw_data_unit);
}

void accel_sensor::raw_to_orientation(sensor_data_t &raw, sensor_data_t &orientation)
{
	orientation.timestamp = raw.time_stamp;
	orientation.data_accuracy = raw.data_accuracy;
	orientation.data_unit_idx = SENSOR_UNIT_DEGREE;
	orientation.values_num = 3;
	orientation.values[0] = fmodf((atan2(raw.values[0], raw.values[1]) * RADIAN_VALUE + DEGREE_360), DEGREE_360);
	orientation.values[1] = fmodf((atan2(raw.values[1], raw.values[2]) * RADIAN_VALUE), DEGREE_180);
	orientation.values[2] = fmodf((atan2(raw.values[0], raw.values[2]) * RADIAN_VALUE), DEGREE_180);

	if (orientation.values[2] > DEGREE_90)
		orientation.values[2] = DEGREE_180 - orientation.values[2];
	else if (orientation.values[2] < -DEGREE_90)
		orientation.values[2] = -DEGREE_180 - orientation.values[2];
}

extern "C" void *create(void)
{
	accel_sensor *inst;

	try {
		inst = new accel_sensor();
	} catch (int err) {
		ERR("Failed to create accel_sensor class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (accel_sensor *)inst;
}
