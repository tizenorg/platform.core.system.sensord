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

#ifndef _ACCEL_SENSOR_H_
#define _ACCEL_SENSOR_H_

#include <sensor_common.h>
#include <physical_sensor.h>
#include <sensor_hal.h>

class accel_sensor : public physical_sensor
{
public:
	accel_sensor();
	virtual ~accel_sensor();

	virtual bool init();
	virtual sensor_type_t get_type(void);

	static bool working(void *inst);

	virtual bool on_start(void);
	virtual bool on_stop(void);

	bool add_client(unsigned int event_type);

	virtual bool set_interval(unsigned long interval);
	virtual long set_command(const unsigned int cmd, long value);
	virtual bool get_properties(const unsigned int type, sensor_properties_t &properties);
	int get_sensor_data(const unsigned int type, sensor_data_t &data);
private:
	sensor_hal *m_sensor_hal;
	cmutex m_value_mutex;

	float m_raw_data_unit;

	int m_rotation;
	unsigned long long m_rotation_time;
	unsigned long m_interval;
	int m_curr_window_count;

	static const int RADIAN_VALUE = 57.29747;
	static const int MAX_WINDOW_NUM = 2;
	long m_windowing[MAX_WINDOW_NUM];

	int m_rotation_check_remained_time;

	int get_rotation_event(float x, float y, float z);
	void reset_rotation(void);
	bool is_rotation_time(void);

	void raw_to_base(sensor_data_t &data);
	void raw_to_orientation(sensor_data_t &raw, sensor_data_t &orientation);
	bool process_event(void);
};
#endif /*_ACCEL_SENSOR_H_*/
