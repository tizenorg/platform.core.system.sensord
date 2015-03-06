/*
 * libsensord-share
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

#ifndef _SENSOR_HAL_SHARED_H_
#define _SENSOR_HAL_SHARED_H_

typedef struct {
	int method;
	string data_node_path;
	string enable_node_path;
	string interval_node_path;
	string buffer_enable_node_path;
	string buffer_length_node_path;
	string trigger_node_path;
} node_info;

typedef struct {
	bool sensorhub_controlled;
	string sensor_type;
	string key;
	string iio_enable_node_name;
	string sensorhub_interval_node_name;
} node_info_query;

enum input_method {
	IIO_METHOD = 0,
	INPUT_EVENT_METHOD = 1,
};

typedef struct {
	int method;
	std::string dir_path;
	std::string prefix;
} input_method_info;

class sensor_hal_shared {
public:
	sensor_hal_shared();
	~sensor_hal_shared();

	bool set_enable_node(const string &node_path, bool sensorhub_controlled, bool enable, int enable_bit = 0);
	bool find_model_id(const string &sensor_type, string &model_id);
	bool is_sensorhub_controlled(const string &key);
	bool get_node_info(const node_info_query &query, node_info &info);
	void show_node_info(node_info &info);
	bool set_node_value(const string &node_path, int value);
	bool set_node_value(const string &node_path, unsigned long long value);
	bool get_node_value(const string &node_path, int &value);
	bool get_event_num(const string &node_path, string &event_num);
	bool get_input_method(const string &key, int &method, string &device_num);

	bool get_iio_node_info(const string& enable_node_name, const string& device_num, node_info &info);
	bool get_sensorhub_iio_node_info(const string &interval_node_name, const string& device_num, node_info &info);
	bool get_input_event_node_info(const string& device_num, node_info &info);
	bool get_sensorhub_input_event_node_info(const string &interval_node_name, const string& device_num, node_info &info);

	unsigned long long get_timestamp(void);
	unsigned long long get_timestamp(timeval *t);
}
