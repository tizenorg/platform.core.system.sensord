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

#include <sensor_hal.h>
#include <dirent.h>
#include <string.h>
#include <fstream>
#include <csensor_config.h>

using std::ifstream;
using std::fstream;

cmutex sensor_hal::m_shared_mutex;

sensor_hal::sensor_hal()
{
}

sensor_hal::~sensor_hal()
{
}

bool sensor_hal::init(void *data)
{
	return true;
}

bool sensor_hal::set_interval(unsigned long val)
{
	return true;
}

long sensor_hal::set_command(unsigned int cmd, long val)
{
	return -1;
}

int sensor_hal::send_sensorhub_data(const char* data, int data_len)
{
	return -1;
}

int sensor_hal::get_sensor_data(sensor_data_t &data)
{
	return -1;
}

int sensor_hal::get_sensor_data(sensorhub_data_t &data)
{
	return -1;
}

unsigned long long sensor_hal::get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

unsigned long long sensor_hal::get_timestamp(timeval *t)
{
	if (!t) {
		ERR("t is NULL");
		return 0;
	}

	return ((unsigned long long)(t->tv_sec)*1000000LL +t->tv_usec);
}

bool sensor_hal::is_sensorhub_controlled(const string &key)
{
	string key_node =  string("/sys/class/sensors/ssp_sensor/") + key;

	if (access(key_node.c_str(), F_OK) == 0)
		return true;

	return false;
}

bool sensor_hal::get_node_path_info(const node_path_info_query &query, node_path_info &info)
{
	bool ret = false;
	string model_id;

	if (query.input_method == IIO_METHOD) {

		find_model_id(IIO_METHOD, query.sensor_type, model_id);
		if (query.sensorhub_controlled)
			ret = get_sensorhub_iio_node_info(model_id, query.sensorhub_interval_node_name, info);
		else
			ret = get_iio_node_info(model_id, query.iio_enable_node_name, info);
	} else {
		if (query.sensorhub_controlled)
			ret = get_sensorhub_input_event_node_info(query.input_event_key, query.sensorhub_interval_node_name, info);
		else
			ret = get_input_event_node_info(query.input_event_key, info);
	}

	return ret;
}


void sensor_hal::show_node_path_info(node_path_info &info)
{
	if (info.data_node_path.size())
		INFO("Data node: %s", info.data_node_path.c_str());
	if (info.enable_node_path.size())
		INFO("Enable node: %s", info.enable_node_path.c_str());
	if (info.interval_node_path.size())
		INFO("Interval node: %s", info.interval_node_path.c_str());
	if (info.buffer_enable_node_path.size())
		INFO("Buffer enable node: %s", info.buffer_enable_node_path.c_str());
	if (info.buffer_length_node_path.size())
		INFO("Buffer length node: %s", info.buffer_length_node_path.c_str());
	if (info.trigger_node_path.size())
		INFO("Trigger node: %s", info.trigger_node_path.c_str());
}

bool sensor_hal::get_iio_node_info(const string &key, const string& enable_node_name, node_path_info &info)
{
	string device_num;

	if (!get_device_num(IIO_METHOD, key, device_num))
		return false;

	info.data_node_path = string("/dev/iio:device") + device_num;

	const string base_dir = string("/sys/bus/iio/devices/iio:device") + device_num + string("/");

	info.base_dir = base_dir;
	info.enable_node_path = base_dir + enable_node_name;
	info.interval_node_path = base_dir + string("sampling_frequency");
	info.buffer_enable_node_path = base_dir + string("buffer/enable");
	info.buffer_length_node_path = base_dir + string("buffer/length");
	info.trigger_node_path = base_dir + string("trigger/current_trigger");
	info.available_freq_node_path = base_dir + string("sampling_frequency_available");

	return true;
}

bool sensor_hal::get_sensorhub_iio_node_info(const string &key, const string &interval_node_name, node_path_info &info)
{
	const string base_dir = "/sys/class/sensors/ssp_sensor/";
	string device_num;

	if (!get_device_num(IIO_METHOD, key, device_num))
		return false;

	info.base_dir = base_dir;
	info.data_node_path = string("/dev/iio:device") + device_num;
	info.enable_node_path = base_dir + string("enable"); //this may need to be changed
	info.interval_node_path = base_dir + interval_node_name;
	return true;
}

bool sensor_hal::get_input_event_node_info(const string &key, node_path_info &info)
{
	string base_dir;
	string device_num;
	string event_num;

	if (!get_device_num(INPUT_EVENT_METHOD, key, device_num))
		return false;

	base_dir = string("/sys/class/input/input") + device_num + string("/");

	if (!get_event_num(base_dir, event_num))
		return false;

	info.data_node_path = string("/dev/input/event") + event_num;

	info.enable_node_path = base_dir + string("enable");
	info.interval_node_path = base_dir + string("poll_delay");
	return true;
}

bool sensor_hal::get_sensorhub_input_event_node_info(const string &key, const string &interval_node_name, node_path_info &info)
{
	const string base_dir = "/sys/class/sensors/ssp_sensor/";
	string device_num;
	string event_num;

	if (!get_device_num(INPUT_EVENT_METHOD, key, device_num))
		return false;

	string input_dir = string("/sys/class/input/input") + device_num + string("/");

	if (!get_event_num(input_dir, event_num))
		return false;

	info.data_node_path = string("/dev/input/event") + event_num;
	info.enable_node_path = base_dir + string("enable");
	info.interval_node_path = base_dir + interval_node_name;
	return true;
}

bool sensor_hal::set_node_value(const string &node_path, int value)
{
	fstream node(node_path, fstream::out);

	if (!node)
		return false;

	node << value;

	return true;
}

bool sensor_hal::set_node_value(const string &node_path, unsigned long long value)
{
	fstream node(node_path, fstream::out);

	if (!node)
		return false;

	node << value;

	return true;
}


bool sensor_hal::get_node_value(const string &node_path, int &value)
{
	fstream node(node_path, fstream::in);

	if (!node)
		return false;

	node >> value;

	return true;
}

bool sensor_hal::set_enable_node(const string &node_path, bool sensorhub_controlled, bool enable, int enable_bit)
{
	int prev_status, status;

	AUTOLOCK(m_shared_mutex);

	if (!get_node_value(node_path, prev_status)) {
		ERR("Failed to get node: %s", node_path.c_str());
		return false;
	}

	int _enable_bit = sensorhub_controlled ? enable_bit : 0;

	if (enable)
		status = prev_status | (1 << _enable_bit);
	else
		status = prev_status & (~(1 << _enable_bit));

	if (!set_node_value(node_path, status)) {
		ERR("Failed to set node: %s", node_path.c_str());
		return false;
	}

	return true;
}


bool sensor_hal::find_model_id(int method, const string &sensor_type, string &model_id)
{
	const string input_event_dir = "/sys/class/sensors/";
	const string iio_dir = "/sys/bus/iio/devices/";
	string dir_path;
	string name_node, name;
	string d_name;
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find = false;

	if (method == IIO_METHOD)
		dir_path = iio_dir;
	else
		dir_path = input_event_dir;

	dir = opendir(dir_path.c_str());
	if (!dir) {
		DBG("Failed to open dir: %s", dir_path.c_str());
		return false;
	}

	while (!find && (dir_entry = readdir(dir))) {
		d_name = string(dir_entry->d_name);

		if ((d_name != ".") && (d_name != "..") && (dir_entry->d_ino != 0)) {
			name_node = dir_path + d_name + string("/name");

			ifstream infile(name_node.c_str());

			if (!infile)
				continue;

			infile >> name;

			if (csensor_config::get_instance().is_supported(sensor_type, name)) {
				model_id = name;
				find = true;
				break;
			}
		}
	}

	closedir(dir);

	return find;
}

bool sensor_hal::verify_iio_trigger(const string &trigger_name)
{
	return true;
}

bool sensor_hal::get_model_properties(const string &sensor_type, string &model_id, int &input_method)
{
	if (find_model_id(INPUT_EVENT_METHOD, sensor_type, model_id)) {
		input_method = INPUT_EVENT_METHOD;
		return true;
	} else if (find_model_id(IIO_METHOD, sensor_type, model_id)) {
		input_method = IIO_METHOD;
		return true;
	}

	return false;
}

bool sensor_hal::get_event_num(const string &input_path, string &event_num)
{
	const string event_prefix = "event";
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	string node_name;
	bool find = false;

	dir = opendir(input_path.c_str());
	if (!dir) {
		ERR("Failed to open dir: %s", input_path.c_str());
		return false;
	}

	int prefix_size = event_prefix.size();

	while (!find && (dir_entry = readdir(dir))) {
		node_name = dir_entry->d_name;

		if (node_name.compare(0, prefix_size, event_prefix) == 0) {
			event_num = node_name.substr(prefix_size, node_name.size() - prefix_size);
			find = true;
			break;
		}
	}

	closedir(dir);

	return find;
}

bool sensor_hal::get_device_num(int method, const string &key, string &device_num)
{
	const string input_event_dir = "/sys/class/input/";
	const string iio_dir = "/sys/bus/iio/devices/";
	const string input_event_prefix = "input";
	const string iio_prefix = "iio:device";

	string dir_path;
	string prefix;
	size_t prefix_size;
	string name_node, name;
	string d_name;
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find = false;

	if (method == IIO_METHOD) {
		dir_path = iio_dir;
		prefix = iio_prefix;
	} else {
		dir_path = input_event_dir;
		prefix = input_event_prefix;
	}

	prefix_size = prefix.size();

	dir = opendir(dir_path.c_str());
	if (!dir) {
		ERR("Failed to open dir: %s", dir_path.c_str());
		return false;
	}

	while (!find && (dir_entry = readdir(dir))) {
		d_name = string(dir_entry->d_name);

		if (d_name.compare(0, prefix_size, prefix) == 0) {
			name_node = dir_path + d_name + string("/name");

			ifstream infile(name_node.c_str());
			if (!infile)
				continue;

			infile >> name;

			if (name == key) {
				device_num = d_name.substr(prefix_size, d_name.size() - prefix_size);
				find = true;
				break;
			}
		}
	}

	closedir(dir);

	return find;
}

bool sensor_hal::get_generic_channel_names(const string &scan_dir, const string &suffix, vector<string> &generic_channel_names)
{
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;
	string file_node;
	string d_name;
	unsigned int pos;

	dir = opendir(scan_dir.c_str());
	if (!dir) {
		DBG("Failed to open dir: %s", dir_path.c_str());
		return false;
	}

	generic_channel_names.clear();

	while (true) {
		dir_entry = readdir(dir);
		if (dir_entry == NULL)
			break;

		d_name = string(dir_entry->d_name);

		if ((d_name != ".") && (d_name != "..") && (dir_entry->d_ino != 0)) {
			pos = d_name.rfind(suffix.c_str());
			if (pos == string::npos)
				continue;
			generic_channel_names.push_back(d_name.substr(0 , pos));
		}
	}
	closedir(dir);
	if (generic_channel_names.size() > 0)
		return true;
	return false;
}
