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

#ifndef _GEO_SENSOR_HAL_H_
#define _GEO_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>
#include <fstream>

#define IIO_DIR			"/sys/bus/iio/devices/"
#define X_RAW_VAL_NODE	"/in_magn_x_raw"
#define Y_RAW_VAL_NODE	"/in_magn_y_raw"
#define Z_RAW_VAL_NODE	"/in_magn_z_raw"
#define X_SCALE_NODE	"/in_magn_x_scale"
#define Y_SCALE_NODE	"/in_magn_y_scale"
#define Z_SCALE_NODE	"/in_magn_z_scale"
#define NAME_NODE		"/name"

using std::string;
using std::ifstream;

class geo_sensor_hal : public sensor_hal
{
public:
	geo_sensor_hal();
	virtual ~geo_sensor_hal();
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
	double m_x;
	double m_y;
	double m_z;
	double m_x_scale;
	double m_y_scale;
	double m_z_scale;

	unsigned long m_polling_interval;
	unsigned long long m_fired_time;
	bool m_sensorhub_supported;

	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	string m_x_node;
	string m_y_node;
	string m_z_node;
	string m_x_scale_node;
	string m_y_scale_node;
	string m_z_scale_node;

	cmutex m_value_mutex;

	bool enable_resource(string &resource_node, bool enable);
	bool update_value(void);
	bool is_sensorhub_supported(void);
	bool init_resources(void);

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
#endif /*_GEO_SENSOR_HAL_H_*/
