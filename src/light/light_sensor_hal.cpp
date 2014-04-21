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

#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/input.h>
#include <cconfig.h>
#include <light_sensor_hal.h>

using std::ifstream;
using config::CConfig;

#define NODE_NAME "name"
#define NODE_INPUT "input"
#define NODE_ENABLE "enable"
#define NODE_POLL_DELAY "poll_delay"
#define NODE_LIGHT_POLL_DELAY "light_poll_delay"
#define SENSOR_NODE "/sys/class/sensors/"
#define SENSORHUB_NODE "/sys/class/sensors/ssp_sensor/"
#define INPUT_DEVICE_NODE "/sys/class/input/"
#define DEV_INPUT_NODE "/dev/input/event/"

#define BIAS	1
#define INITIAL_VALUE -1
#define INITIAL_TIME 0

#define SENSOR_TYPE_LIGHT		"LIGHT"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"
#define ATTR_VALUE				"value"

#define INPUT_NAME	"light_sensor"

light_sensor_hal::light_sensor_hal()
: m_adc(INITIAL_VALUE)
, m_node_handle(INITIAL_VALUE)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(INITIAL_TIME)
, m_sensorhub_supported(false)
{
	if (!check_hw_node()) {
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	if ((m_node_handle = open(m_resource.c_str(), O_RDWR)) < 0) {
		ERR("Failed to open handle(%d)", m_node_handle);
		throw ENXIO;
	}

	int clockId = CLOCK_MONOTONIC;

	if (ioctl(m_node_handle, EVIOCSCLOCKID, &clockId) != 0) {
		ERR("Fail to set monotonic timestamp for %s", m_resource.c_str());
		throw ENXIO;
	}

	INFO("light_sensor_hal is created!");
}

light_sensor_hal::~light_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = INITIAL_VALUE;

	INFO("light_sensor_hal is destroyed!");
}

string light_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t light_sensor_hal::get_type(void)
{
	return LIGHT_SENSOR;
}

bool light_sensor_hal::enable_resource(string &resource_node, bool enable)
{
	int prev_status, status;
	FILE *fp = NULL;
	fp = fopen(resource_node.c_str(), "r");

	if (!fp) {
		ERR("Fail to open a resource file: %s", resource_node.c_str());
		return false;
	}

	if (fscanf(fp, "%d", &prev_status) < 0) {
		ERR("Failed to get data from %s", resource_node.c_str());
		fclose(fp);
		return false;
	}

	fclose(fp);

	if (enable) {
		if (m_sensorhub_supported)
			status = prev_status | (1 << SENSORHUB_LIGHT_ENABLE_BIT);
		else
			status = 1;
	} else {
		if (m_sensorhub_supported)
			status = prev_status ^ (1 << SENSORHUB_LIGHT_ENABLE_BIT);
		else
			status = 0;
	}

	fp = fopen(resource_node.c_str(), "w");

	if (!fp) {
		ERR("Failed to open a resource file: %s", resource_node.c_str());
		return false;
	}

	if (fprintf(fp, "%d", status) < 0) {
		ERR("Failed to enable a resource file: %s", resource_node.c_str());
		fclose(fp);
		return false;
	}

	if (fp)
		fclose(fp);

	return true;
}

bool light_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, true);
	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Light sensor real starting");
	return true;
}

bool light_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, false);
	INFO("Light sensor real stopping");
	return true;
}

bool light_sensor_hal::set_interval(unsigned long val)
{
	unsigned long long polling_interval_ns;
	FILE *fp = NULL;

	AUTOLOCK(m_mutex);

	polling_interval_ns = ((unsigned long long)(val) * MS_TO_SEC * MS_TO_SEC);
	fp = fopen(m_polling_resource.c_str(), "w");

	if (!fp) {
		ERR("Failed to open a resource file: %s", m_polling_resource.c_str());
		return false;
	}

	if (fprintf(fp, "%llu", polling_interval_ns) < 0) {
		ERR("Failed to set data %llu", polling_interval_ns);
		fclose(fp);
		return false;
	}

	if (fp)
		fclose(fp);

	INFO("Interval is changed from %dms to %dms]", m_polling_interval, val);
	m_polling_interval = val;
	return true;
}

bool light_sensor_hal::update_value(bool wait)
{
	const int TIMEOUT = 1;
	struct timeval tv;
	fd_set readfds, exceptfds;
	int adc = INITIAL_VALUE;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_SET(m_node_handle, &readfds);
	FD_SET(m_node_handle, &exceptfds);

	if (wait) {
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
	} else {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	}

	int ret;
	ret = select(m_node_handle + 1, &readfds, NULL, &exceptfds, &tv);

	if (ret == -1) {
		ERR("select error:%s m_node_handle:%d", strerror(errno), m_node_handle);
		return false;
	} else if (!ret) {
		DBG("select timeout: %d seconds elapsed", tv.tv_sec);
		return false;
	}

	if (FD_ISSET(m_node_handle, &exceptfds)) {
		ERR("select exception occurred!");
		return false;
	}

	if (FD_ISSET(m_node_handle, &readfds)) {
		struct input_event light_event;
		DBG("light event detection!");

		int len;
		len = read(m_node_handle, &light_event, sizeof(light_event));

		if (len == -1) {
			DBG("read(m_node_handle) is error:%s.", strerror(errno));
			return false;
		}

		if (light_event.type == EV_ABS && light_event.code == ABS_MISC) {
			adc = light_event.value;
		} else if (light_event.type == EV_REL && light_event.code == REL_RX) {
			adc = light_event.value - BIAS;
		} else {
			DBG("light input event[type = %d, code = %d] is unknown.", light_event.type, light_event.code);
			return false;
		}

		DBG("read event, len : %d, type : %x, code : %x, value : %x",
			len, light_event.type, light_event.code, light_event.value);
		DBG("update_value, adc : %d", adc);

		AUTOLOCK(m_value_mutex);
		m_adc = adc;
		m_fired_time = get_timestamp(&light_event.time);
	} else {
		ERR("select nothing to read!!!");
		return false;
	}

	return true;
}

bool light_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int light_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	const int chance = 3;
	int retry = 0;

	while ((m_fired_time == 0) && (retry++ < chance)) {
		INFO("Try usleep for getting a valid BASE DATA value");
		usleep(m_polling_interval * MS_TO_SEC);
	}

	if (m_fired_time == 0) {
		ERR("get_sensor_data failed");
		return -1;
	}

	AUTOLOCK(m_value_mutex);
	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_LUX;
	data.timestamp = m_fired_time ;
	data.values_num = 1;
	data.values[0] = (float) m_adc;

	return 0;
}

bool light_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_LUX;
	properties.sensor_min_range = 0;
	properties.sensor_max_range = 65536;
	snprintf(properties.sensor_name, sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = 1.0f;
	return true;
}

bool light_sensor_hal::is_sensorhub_supported(void)
{
	DIR *main_dir = NULL;
	main_dir = opendir(SENSORHUB_NODE);

	if (!main_dir) {
		INFO("Sensor Hub is not supported");
		return false;
	}

	INFO("It supports sensor hub");
	closedir(main_dir);
	return true;
}

bool light_sensor_hal::check_hw_node(void)
{
	string name_node;
	string hw_name;
	DIR *main_dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find_node = false;

	INFO("======================start check_hw_node=============================");

	m_sensorhub_supported = is_sensorhub_supported();
	main_dir = opendir(SENSOR_NODE);

	if (!main_dir) {
		ERR("Directory open failed to collect data");
		return false;
	}

	while ((!find_node) && (dir_entry = readdir(main_dir))) {
		if ((strncasecmp(dir_entry->d_name , ".", 1 ) != 0) && (strncasecmp(dir_entry->d_name , "..", 2 ) != 0) && (dir_entry->d_ino != 0)) {
			name_node = string(SENSOR_NODE) + string(dir_entry->d_name) + string("/") + string(NODE_NAME);

			ifstream infile(name_node.c_str());

			if (!infile)
				continue;

			infile >> hw_name;

			if (CConfig::get_instance().is_supported(SENSOR_TYPE_LIGHT, hw_name) == true) {
				m_name = m_model_id = hw_name;
				INFO("m_model_id = %s", m_model_id.c_str());
				find_node = true;
				break;
			}
		}
	}

	closedir(main_dir);

	if (find_node) {
		main_dir = opendir(INPUT_DEVICE_NODE);

		if (!main_dir) {
			ERR("Directory open failed to collect data");
			return false;
		}

		find_node = false;

		while ((!find_node) && (dir_entry = readdir(main_dir))) {
			if (strncasecmp(dir_entry->d_name, NODE_INPUT, 5) == 0) {
				name_node = string(INPUT_DEVICE_NODE) + string(dir_entry->d_name) + string("/") + string(NODE_NAME);
				ifstream infile(name_node.c_str());

				if (!infile)
					continue;

				infile >> hw_name;

				if (hw_name == string(INPUT_NAME)) {
					INFO("name_node = %s", name_node.c_str());
					DBG("Find H/W  for light_sensor");

					find_node = true;
					string dir_name;
					dir_name = string(dir_entry->d_name);
					unsigned found = dir_name.find_first_not_of(NODE_INPUT);
					m_resource = string(DEV_INPUT_NODE) + dir_name.substr(found);

					if (m_sensorhub_supported) {
						m_enable_resource = string(SENSORHUB_NODE) + string(NODE_ENABLE);
						m_polling_resource = string(SENSORHUB_NODE) + string(NODE_LIGHT_POLL_DELAY);
					} else {
						m_enable_resource = string(INPUT_DEVICE_NODE) + string(dir_entry->d_name) + string("/") + string(NODE_ENABLE);
						m_polling_resource = string(INPUT_DEVICE_NODE) + string(dir_entry->d_name) + string("/") + string(NODE_POLL_DELAY);
					}

					break;
				}
			}
		}

		closedir(main_dir);
	}

	if (find_node) {
		INFO("m_resource = %s", m_resource.c_str());
		INFO("m_enable_resource = %s", m_enable_resource.c_str());
		INFO("m_polling_resource = %s", m_polling_resource.c_str());
	}

	return find_node;
}

extern "C" void *create(void)
{
	light_sensor_hal *inst;

	try {
		inst = new light_sensor_hal();
	} catch (int err) {
		ERR("Failed to create light_sensor_hal class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (light_sensor_hal *)inst;
}
