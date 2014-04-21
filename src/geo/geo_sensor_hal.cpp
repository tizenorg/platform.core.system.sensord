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
#include <geo_sensor_hal.h>

using std::ifstream;
using config::CConfig;

#define NODE_NAME "name"
#define NODE_INPUT "input"
#define NODE_ENABLE "enable"
#define NODE_POLL_DELAY "poll_delay"
#define NODE_MAG_POLL_DELAY "mag_poll_delay"
#define SENSOR_NODE "/sys/class/sensors/"
#define SENSORHUB_NODE "/sys/class/sensors/ssp_sensor/"
#define INPUT_DEVICE_NODE "/sys/class/input/"
#define DEV_INPUT_NODE "/dev/input/event/"

#define LBS_TO_UTESLA 0.06f

#define SENSOR_TYPE_MAGNETIC	"MAGNETIC"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE				"value"

#define INPUT_NAME	"geomagnetic_sensor"

geo_sensor_hal::geo_sensor_hal()
: m_x(INITIAL_VALUE)
, m_y(INITIAL_VALUE)
, m_z(INITIAL_VALUE)
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

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_NAME, m_chip_name)) {
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

	INFO("geo_sensor_hal is created!");
}

geo_sensor_hal::~geo_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = INITIAL_VALUE;

	INFO("geo_sensor_hal is destroyed!");
}

string geo_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t geo_sensor_hal::get_type(void)
{
	return GEOMAGNETIC_SENSOR;
}

bool geo_sensor_hal::enable_resource(string &resource_node, bool enable)
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
			status = prev_status | (1 << SENSORHUB_GEOMAGNETIC_ENABLE_BIT);
		else
			status = 1;
	} else {
		if (m_sensorhub_supported)
			status = prev_status ^ (1 << SENSORHUB_GEOMAGNETIC_ENABLE_BIT);
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

bool geo_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, true);
	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Geo sensor real starting");
	return true;
}

bool geo_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, false);
	INFO("Geo sensor real stopping");
	return true;
}

bool geo_sensor_hal::set_interval(unsigned long val)
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

bool geo_sensor_hal::update_value(bool wait)
{
	const int TIMEOUT = 1;
	int geo_raw[4] = {0,};
	bool x, y, z, hdst;
	int read_input_cnt = 0;
	const int INPUT_MAX_BEFORE_SYN = 10;
	unsigned long long fired_time = 0;
	bool syn = false;
	x = y = z = hdst = false;

	struct timeval tv;
	fd_set readfds, exceptfds;

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
		struct input_event geo_input;
		DBG("geo event detection!");

		while ((syn == false) && (read_input_cnt < INPUT_MAX_BEFORE_SYN)) {
			int len = read(m_node_handle, &geo_input, sizeof(geo_input));

			if (len != sizeof(geo_input)) {
				ERR("geo_file read fail, read_len = %d", len);
				return false;
			}

			++read_input_cnt;

			if (geo_input.type == EV_REL) {
				switch (geo_input.code) {
				case REL_RX:
					geo_raw[0] = (int)geo_input.value;
					x = true;
					break;
				case REL_RY:
					geo_raw[1] = (int)geo_input.value;
					y = true;
					break;
				case REL_RZ:
					geo_raw[2] = (int)geo_input.value;
					z = true;
					break;
				case REL_HWHEEL:
					geo_raw[3] = (int)geo_input.value;
					hdst = true;
					break;
				default:
					ERR("geo_input event[type = %d, code = %d] is unknown.", geo_input.type, geo_input.code);
					return false;
					break;
				}
			} else if (geo_input.type == EV_SYN) {
				syn = true;
				fired_time = get_timestamp(&geo_input.time);
			} else {
				ERR("geo_input event[type = %d, code = %d] is unknown.", geo_input.type, geo_input.code);
				return false;
			}
		}
	} else {
		ERR("select nothing to read!!!");
		return false;
	}

	if (syn == false) {
		ERR("EV_SYN didn't come until %d inputs had come", read_input_cnt);
		return false;
	}

	AUTOLOCK(m_value_mutex);

	if (x)
		m_x = geo_raw[0];

	if (y)
		m_y = geo_raw[1];

	if (z)
		m_z = geo_raw[2];

	if (hdst)
		m_hdst = geo_raw[3] - 1; /* accuracy bias: -1 */

	m_fired_time = fired_time;

	DBG("m_x = %d, m_y = %d, m_z = %d, m_hdst = %d, time = %lluus", m_x, m_y, m_z, m_hdst, m_fired_time);
	return true;
}

bool geo_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int geo_sensor_hal::get_sensor_data(sensor_data_t &data)
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

	data.data_accuracy = (m_hdst == 1) ? 0 : m_hdst; /* hdst 0 and 1 are needed to calibrate */
	data.data_unit_idx = SENSOR_UNIT_MICRO_TESLA;
	data.timestamp = m_fired_time;
	data.values_num = 3;
	data.values[0] = (float)m_x * LBS_TO_UTESLA;
	data.values[1] = (float)m_y * LBS_TO_UTESLA;
	data.values[2] = (float)m_z * LBS_TO_UTESLA;

	return 0;
}

bool geo_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_MICRO_TESLA;
	properties.sensor_min_range = -1200;
	properties.sensor_max_range = 1200;
	snprintf(properties.sensor_name, sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = 1;
	return true;
}

bool geo_sensor_hal::is_sensorhub_supported(void)
{
	FILE *fp = NULL;
	string mag_polling_resource = string(SENSORHUB_NODE) + string(MAG_POLL_DELAY);
	fp = fopen(mag_polling_resource.c_str(), "r");

	if (!fp) {
		ERR("Fail to open a resource file");
		return false;
	}

	INFO("Supported by Sensor-Hub");
	fclose(fp);
	return true;
}

bool geo_sensor_hal::check_hw_node(void)
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

			if (CConfig::get_instance().is_supported(SENSOR_TYPE_MAGNETIC, hw_name) == true) {
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
					DBG("Find H/W  for mag_sensor");

					find_node = true;
					string dir_name;
					dir_name = string(dir_entry->d_name);
					unsigned found = dir_name.find_first_not_of(NODE_INPUT);
					m_resource = string(DEV_INPUT_NODE) + dir_name.substr(found);

					if (m_sensorhub_supported) {
						m_enable_resource = string(SENSORHUB_NODE) + string(NODE_ENABLE);
						m_polling_resource = string(SENSORHUB_NODE) + string(NODE_MAG_POLL_DELAY);
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
	geo_sensor_hal *inst;

	try {
		inst = new geo_sensor_hal();
	} catch (int err) {
		ERR("Failed to create geo_sensor_hal class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (geo_sensor_hal *)inst;
}
