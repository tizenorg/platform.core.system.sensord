/*
 * proxi_sensor_hal
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
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <linux/input.h>
#include <cconfig.h>

#include <proxi_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cconfig.h>

using std::ifstream;
using config::CConfig;


#define SENSOR_TYPE_PROXI		"PROXI"
#define ELEMENT_NAME 			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE 				"value"

#define INPUT_NAME	"proximity_sensor"
#define PROXI_CODE	0x0019

proxi_sensor_hal::proxi_sensor_hal()
: m_state(PROXIMITY_STATE_FAR)
, m_fired_time(0)
, m_node_handle(-1)
, m_sensorhub_supported(false)
{
	if (!check_hw_node()) {
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_PROXI, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_PROXI, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s\n",m_chip_name.c_str());

	if ((m_node_handle = open(m_resource.c_str(), O_RDWR)) < 0) {
		ERR("Proxi handle(%d) open fail", m_node_handle);
		throw ENXIO;
	}

	int clockId = CLOCK_MONOTONIC;
	if (ioctl(m_node_handle, EVIOCSCLOCKID, &clockId) != 0) {
		ERR("Fail to set monotonic timestamp for %s", m_resource.c_str());
		throw ENXIO;
	}

	INFO("Proxi_sensor_hal is created!\n");

}

proxi_sensor_hal::~proxi_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

	INFO("Proxi_sensor_hal is destroyed!\n");
}

string proxi_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t proxi_sensor_hal::get_type(void)
{
	return PROXIMITY_SENSOR;
}

bool proxi_sensor_hal::enable_resource(string &resource_node, bool enable)
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
		if (m_sensorhub_supported) {
			status = prev_status | (1 << SENSORHUB_PROXIMITY_ENABLE_BIT);
		} else {
			status = 1;
		}
	} else {
		if (m_sensorhub_supported) {
			status = prev_status ^ (1 << SENSORHUB_PROXIMITY_ENABLE_BIT);
		} else {
			status = 0;
		}
	}

	fp = fopen(resource_node.c_str(), "w");
	if (!fp) {
		ERR("Failed to open a resource file: %s\n", resource_node.c_str());
		return false;
	}

	if (fprintf(fp, "%d", status) < 0) {
		ERR("Failed to enable a resource file: %s\n", resource_node.c_str());
		fclose(fp);
		return false;
	}

	if (fp)
		fclose(fp);

	return true;

}

bool proxi_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, true);

	m_fired_time = 0;
	INFO("Proxi sensor real starting");
	return true;
}

bool proxi_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, false);

	INFO("Proxi sensor real stopping");
	return true;
}

bool proxi_sensor_hal::update_value(bool wait)
{
	struct input_event proxi_event;

	fd_set readfds,exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);

	FD_SET(m_node_handle, &readfds);
	FD_SET(m_node_handle, &exceptfds);

	int ret;
	ret = select(m_node_handle+1, &readfds, NULL, &exceptfds, NULL);

	if (ret == -1) {
		ERR("select error:%s m_node_handle:d\n", strerror(errno), m_node_handle);
		return false;
	} else if (!ret) {
		DBG("select timeout");
		return false;
	}

	if (FD_ISSET(m_node_handle, &exceptfds)) {
		ERR("select exception occurred!\n");
		return false;
	}

	if (FD_ISSET(m_node_handle, &readfds)) {
		INFO("proxi event detection!");

		int len = read(m_node_handle, &proxi_event, sizeof(proxi_event));

		if (len == -1) {
			DBG("read(m_node_handle) is error:%s.\n", strerror(errno));
			return false;
		}

		DBG("read event,  len : %d , type : %x , code : %x , value : %x", len, proxi_event.type, proxi_event.code, proxi_event.value);
		if ((proxi_event.type == EV_ABS) && (proxi_event.code == PROXI_CODE)) {
			AUTOLOCK(m_value_mutex);
			if (proxi_event.value == PROXIMITY_NODE_STATE_FAR) {
				INFO("PROXIMITY_STATE_FAR state occured\n");
				m_state = PROXIMITY_STATE_FAR;
			} else if (proxi_event.value == PROXIMITY_NODE_STATE_NEAR) {
				INFO("PROXIMITY_STATE_NEAR state occured\n");
				m_state = PROXIMITY_STATE_NEAR;
			} else {
				ERR("PROXIMITY_STATE Unknown: %d\n",proxi_event.value);
				return false;
			}
			m_fired_time = sensor_hal::get_timestamp(&proxi_event.time);
		} else {
			return false;
		}
	} else {
		ERR("select nothing to read!!!\n");
		return false;
	}

	return true;
}

bool proxi_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int proxi_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);

	data.data_accuracy = SENSOR_ACCURACY_UNDEFINED;
	data.data_unit_idx = SENSOR_UNIT_STATE_ON_OFF;
	data.timestamp = m_fired_time;
	data.values_num = 1;
	data.values[0] = m_state;

	return 0;
}

bool proxi_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_STATE_ON_OFF;
	properties.sensor_min_range = 0;
	properties.sensor_max_range = 1;
	snprintf(properties.sensor_name,   sizeof(properties.sensor_name),"%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor),"%s", m_vendor.c_str());
	properties.sensor_resolution = 1;
	return true;
}

bool proxi_sensor_hal::is_sensorhub_supported(void)
{
	DIR *main_dir = NULL;

	main_dir = opendir("/sys/class/sensors/ssp_sensor");

	if (!main_dir) {
		INFO("Sensor Hub is not supported\n");
		return false;
	}

	INFO("It supports sensor hub");

	closedir(main_dir);

	return true;
}

bool proxi_sensor_hal::check_hw_node(void)
{

	string name_node;
	string hw_name;

	DIR *main_dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find_node = false;

	INFO("======================start check_hw_node=============================\n");

	m_sensorhub_supported = is_sensorhub_supported();

	main_dir = opendir("/sys/class/sensors/");
	if (!main_dir) {
		ERR("Directory open failed to collect data\n");
		return false;
	}

	while ((!find_node) && (dir_entry = readdir(main_dir))) {
		if ((strncasecmp(dir_entry->d_name ,".",1 ) != 0) && (strncasecmp(dir_entry->d_name ,"..",2 ) != 0) && (dir_entry->d_ino != 0)) {

			name_node = string("/sys/class/sensors/") + string(dir_entry->d_name) + string("/name");

			ifstream infile(name_node.c_str());

			if (!infile)
				continue;

			infile >> hw_name;

			if (CConfig::get_instance().is_supported(SENSOR_TYPE_PROXI, hw_name) == true) {
				m_name = m_model_id = hw_name;
				INFO("m_model_id = %s\n",m_model_id.c_str());
				find_node = true;
				break;
			}
		}
	}

	closedir(main_dir);

	if(find_node)
	{
		main_dir = opendir("/sys/class/input/");
		if (!main_dir) {
			ERR("Directory open failed to collect data\n");
			return false;
		}

		find_node = false;

		while ((!find_node) && (dir_entry = readdir(main_dir))) {
			if (strncasecmp(dir_entry->d_name ,"input", 5) == 0) {
				name_node = string("/sys/class/input/") + string(dir_entry->d_name) + string("/name");
				ifstream infile(name_node.c_str());

				if (!infile)
					continue;

				infile >> hw_name;

				if (hw_name == string(INPUT_NAME)) {
					INFO("name_node = %s\n",name_node.c_str());
					DBG("Find H/W  for proxi_sensor\n");

					find_node = true;

					string dir_name;
					dir_name = string(dir_entry->d_name);
					unsigned found = dir_name.find_first_not_of("input");

					m_resource = string("/dev/input/event") + dir_name.substr(found);
					if (m_sensorhub_supported) {
						m_enable_resource = string("/sys/class/sensors/ssp_sensor/enable");
					} else {
						m_enable_resource = string("/sys/class/input/") + string(dir_entry->d_name) + string("/enable");
					}

					break;
				}
			}
		}
		closedir(main_dir);
	}

	if (find_node) {
		INFO("m_resource = %s\n", m_resource.c_str());
		INFO("m_enable_resource = %s\n", m_enable_resource.c_str());
	}

	return find_node;

}

extern "C" void *create(void)
{
	proxi_sensor_hal *inst;

	try {
		inst = new proxi_sensor_hal();
	} catch (int err) {
		ERR("proxi_sensor class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (proxi_sensor_hal*)inst;
}
