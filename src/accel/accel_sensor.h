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

#include <sensor_hal.h>
#include <string>

class cmutex;

class accel_sensor : public sensor_hal
{
public:
	accel_sensor();
	virtual ~accel_sensor();

	virtual bool initialize(void);
	virtual bool enable(void);
	virtual bool disable(void);
	virtual bool set_handle(int handle);
	virtual bool get_fd(int &fd);
	virtual bool get_info(sensor_info_t &info);
	virtual bool get_sensor_data(sensor_data_t &data);
	virtual bool set_command(unsigned int cmd, long val);
	virtual bool batch(int flags,
			unsigned long long interval_ms,
			unsigned long long max_report_latency_ns);
	virtual bool flush(void);
private:
	cmutex m_mutex;
	cmutex m_value_mutex;
	float m_x;
	float m_y;
	float m_z;
	unsigned long long m_fired_time;
	unsigned long long m_polling_interval;

	int m_resolution;
	float m_raw_data_unit;

	int m_method;
	int m_handle;
	int m_node_handle;

	std::string m_name;
	std::string m_model_id;
	std::string m_vendor;
	std::string m_chip_name;

	std::string m_data_node;
	std::string m_enable_node;
	std::string m_interval_node;
};

#endif
