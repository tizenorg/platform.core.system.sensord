/*
 * pressure_sensor_hal
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

#ifndef _PRESSURE_SENSOR_HAL_H_
#define _PRESSURE_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>
#include <fstream>

#define IIO_DIR			"/sys/bus/iio/devices/"
#define NAME_NODE		"/name"
#define EVENT_DIR		"/events"
#define EVENT_EN_NODE	"/in_pressure_mag_either_en"
#define DEV_DIR			"/dev/"
#define PRESSURE_SCALE	"/in_pressure_scale"
#define PRESSURE_RAW	"/in_pressure_raw"
#define TEMP_OFFSET		"/in_temp_offset"
#define TEMP_SCALE		"/in_temp_scale"
#define TEMP_RAW		"/in_temp_raw"

#define IIO_DEV_BASE_NAME	"iio:device"
#define IIO_DEV_STR_LEN		10

using std::string;
using std::ifstream;

class pressure_sensor_hal : public sensor_hal
{
public:
	pressure_sensor_hal();
	virtual ~pressure_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);

	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);

private:
	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	int m_pressure_scale;
	float m_pressure;

	int m_temp_scale;
	float m_temp_offset;
	float m_temperature;

	float m_min_range;
	float m_max_range;
	float m_raw_data_unit;

	unsigned long m_polling_interval;

	unsigned long long m_fired_time;
	int m_event_fd;

	string m_pressure_dir;
	string m_pressure_node;
	string m_temp_node;
	string m_event_resource;
	string m_enable_resource;

	cmutex m_value_mutex;

	bool m_sensorhub_supported;

	bool check_hw_node(void);
	bool update_value(bool wait);
	bool enable_resource(bool enable);
	bool is_sensorhub_supported(void);

	template <typename value_t>
	bool read_node_value(string node_path, value_t &value)
	{
		ifstream handle;
		handle.open(node_path.c_str());
		if (!handle)
		{
			ERR("Failed to open handle(%s)", node_path.c_str());
			return false;
		}
		handle >> value;
		handle.close();

		return true;
	}
};
#endif /*_PRESSURE_SENSOR_HAL_CLASS_H_*/
