/*
 * accel_sensor_hal
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

#ifndef _ACCEL_SENSOR_HAL_H_
#define _ACCEL_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>

using std::string;

class accel_sensor_hal : public sensor_hal
{
	enum accel_cmd_cal_t {
		CAL_CHECK = 0,
		CAL_SET,
		CAL_MKDIR,
	};

public:
	accel_sensor_hal();
	virtual ~accel_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);
	bool check_hw_node(void);
	long set_command(const unsigned int cmd, long value);

private:
	int m_x;
	int m_y;
	int m_z;
	int m_sensor_type;
	int m_node_handle;
	unsigned long m_polling_interval;
	unsigned long long m_fired_time;
	bool m_sensorhub_supported;

	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	int m_resolution;
	float m_raw_data_unit;

	string m_resource;
	string m_enable_resource;
	string m_polling_resource;

	cmutex m_value_mutex;

	bool enable_resource(string &resource_node, bool enable);
	bool update_value(bool wait);
	bool calibration(int cmd);
	bool is_sensorhub_supported(void);

};
#endif /*_ACCEL_SENSOR_HAL_CLASS_H_*/
