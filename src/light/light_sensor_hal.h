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

#ifndef _LIGHT_SENSOR_HAL_H_
#define _LIGHT_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>
#include <fstream>

#define IIO_DIR				"/sys/bus/iio/devices/"
#define NAME_NODE			"/name"
#define ILL_CLEAR_NODE		"/in_illuminance_clear_raw"

#define IIO_DEV_BASE_NAME	"iio:device"
#define IIO_DEV_STR_LEN		10

using std::string;
using std::ifstream;

class light_sensor_hal : public sensor_hal
{
public:
	light_sensor_hal();
	virtual ~light_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);
	bool check_hw_node(void);

private:
	int m_adc;
	bool m_sensorhub_supported;

	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	cmutex m_value_mutex;
	string m_clear_raw_node;

	bool update_value(void);
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
#endif /*_LIGHT_SENSOR_HAL_H_*/
