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
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <linux/input.h>
#include <cconfig.h>

#include <accel_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cconfig.h>

using std::ifstream;
using config::CConfig;

#define CALIBRATION_NODE	"/sys/class/sensors/accelerometer_sensor/calibration"
#define CALIBRATION_FILE	"/csa/sensor/accel_cal_data"
#define CALIBRATION_DIR		"/csa/sensor"

#define GRAVITY 9.80665
#define G_TO_MG 1000
#define RAW_DATA_TO_G_UNIT(X) (((float)(X))/((float)G_TO_MG))
#define RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(X) (GRAVITY * (RAW_DATA_TO_G_UNIT(X)))

#define MIN_RANGE(RES) (-((2 << (RES))/2))
#define MAX_RANGE(RES) (((2 << (RES))/2)-1)

#define SENSOR_TYPE_ACCEL		"ACCEL"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"

#define ATTR_VALUE				"value"

#define INPUT_NAME	"accelerometer_sensor"

accel_sensor_hal::accel_sensor_hal()
: m_x(-1)
, m_y(-1)
, m_z(-1)
, m_sensor_type(ACCELEROMETER_SENSOR)
, m_node_handle(-1)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(0)
, m_sensorhub_supported(false)
{
	if (!check_hw_node()) {
		ERR("check_hw_node() fail\n");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s\n",m_chip_name.c_str());

	long resolution;

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_RESOLUTION, resolution)) {
		ERR("[RESOLUTION] is empty\n");
		throw ENXIO;
	}

	m_resolution = (int)resolution;

	INFO("m_resolution = %d\n",m_resolution);

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);

	if ((m_node_handle = open(m_resource.c_str(), O_RDWR)) < 0) {
		ERR("accel handle open fail for accel processor, error:%s\n", strerror(errno));
		throw ENXIO;
	}

	int clockId = CLOCK_MONOTONIC;
	if (ioctl(m_node_handle, EVIOCSCLOCKID, &clockId) != 0) {
		ERR("Fail to set monotonic timestamp for %s", m_resource.c_str());
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);
	INFO("accel_sensor is created!\n");
}

accel_sensor_hal::~accel_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

	INFO("accel_sensor is destroyed!\n");
}

string accel_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t accel_sensor_hal::get_type(void)
{
	return ACCELEROMETER_SENSOR;
}

long accel_sensor_hal::set_command(const unsigned int cmd, long value)
{
	FILE *fp;

	AUTOLOCK(m_mutex);

	switch (cmd) {
		case ACCELEROMETER_PROPERTY_SET_CALIBRATION :
			if (calibration(CAL_SET)) {
				INFO("acc_sensor_calibration OK\n");
				return 0;
			}

			ERR("acc_sensor_calibration FAIL\n");
			return -1;
		case ACCELEROMETER_PROPERTY_CHECK_CALIBRATION_STATUS :
			if (calibration(CAL_CHECK)) {
				INFO("acc_sensor_calibration check OK\n");
				return 0;
			}

			ERR("acc_sensor_calibration check FAIL\n");
			return -1;
		default:
			ERR("Invalid property_cmd\n");
			break;
	}
	return -1;

}

bool accel_sensor_hal::enable_resource(string &resource_node, bool enable)
{
	int prev_status, status;

	FILE *fp = NULL;

	fp = fopen(resource_node.c_str(), "r");

	if (!fp) {
		ERR("Fail to open a resource file: %s\n", resource_node.c_str());
		return false;
	}

	if (fscanf(fp, "%d", &prev_status) < 0) {
		ERR("Failed to get data from %s\n", resource_node.c_str());
		fclose(fp);
		return false;
	}

	fclose(fp);

	if (enable) {
		if (m_sensorhub_supported) {
			status = prev_status | (1 << SENSORHUB_ACCELEROMETER_ENABLE_BIT);
		} else {
			status = 1;
		}
	} else {
		if (m_sensorhub_supported) {
			status = prev_status ^ (1 << SENSORHUB_ACCELEROMETER_ENABLE_BIT);
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

bool accel_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, true);
	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Accel sensor real starting");
	return true;
}

bool accel_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(m_enable_resource, false);

	INFO("Accel sensor real stopping");
	return true;
}

bool accel_sensor_hal::set_interval(unsigned long val)
{
	unsigned long long polling_interval_ns;

	AUTOLOCK(m_mutex);

	polling_interval_ns = ((unsigned long long)(val) * 1000llu * 1000llu);

	FILE *fp = NULL;

	fp = fopen(m_polling_resource.c_str(), "w");
	if (!fp) {
		ERR("Failed to open a resource file: %s\n", m_polling_resource.c_str());
		return false;
	}

	if (fprintf(fp, "%llu", polling_interval_ns) < 0) {
		ERR("Failed to set data %llu\n", polling_interval_ns);
		fclose(fp);
		return false;
	}

	if (fp)
		fclose(fp);

	INFO("Interval is changed from %dms to %dms]", m_polling_interval, val);
	m_polling_interval = val;
	return true;

}


bool accel_sensor_hal::update_value(bool wait)
{
	const int TIMEOUT = 1;
	int accel_raw[3] = {0,};
	bool x,y,z;
	int read_input_cnt = 0;
	const int INPUT_MAX_BEFORE_SYN = 10;
	unsigned long long fired_time = 0;
	bool syn = false;

	x = y = z = false;

	struct timeval tv;
	fd_set readfds,exceptfds;

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
	ret = select(m_node_handle+1, &readfds, NULL, &exceptfds, &tv);

	if (ret == -1) {
		ERR("select error:%s m_node_handle:d\n", strerror(errno), m_node_handle);
		return false;
	} else if (!ret) {
		DBG("select timeout: %d seconds elapsed.\n", tv.tv_sec);
		return false;
	}

	if (FD_ISSET(m_node_handle, &exceptfds)) {
			ERR("select exception occurred!\n");
			return false;
	}

	if (FD_ISSET(m_node_handle, &readfds)) {

		struct input_event accel_input;
		DBG("accel event detection!");

		while ((syn == false) && (read_input_cnt < INPUT_MAX_BEFORE_SYN)) {
			int len = read(m_node_handle, &accel_input, sizeof(accel_input));
			if (len != sizeof(accel_input)) {
				ERR("accel_file read fail, read_len = %d\n",len);
				return false;
			}

			++read_input_cnt;

			if (accel_input.type == EV_REL) {
				switch (accel_input.code) {
					case REL_X:
						accel_raw[0] = (int)accel_input.value;
						x = true;
						break;
					case REL_Y:
						accel_raw[1] = (int)accel_input.value;
						y = true;
						break;
					case REL_Z:
						accel_raw[2] = (int)accel_input.value;
						z = true;
						break;
					default:
						ERR("accel_input event[type = %d, code = %d] is unknown.", accel_input.type, accel_input.code);
						return false;
						break;
				}
			} else if (accel_input.type == EV_SYN) {
				syn = true;
				fired_time = sensor_hal::get_timestamp(&accel_input.time);
			} else {
				ERR("accel_input event[type = %d, code = %d] is unknown.", accel_input.type, accel_input.code);
				return false;
			}
		}
	} else {
		ERR("select nothing to read!!!\n");
		return false;
	}

	if (syn == false) {
		ERR("EV_SYN didn't come until %d inputs had come", read_input_cnt);
		return false;
	}

	AUTOLOCK(m_value_mutex);

	if (x)
		m_x =  accel_raw[0];
	if (y)
		m_y =  accel_raw[1];
	if (z)
		m_z =  accel_raw[2];

	m_fired_time = fired_time;

	DBG("m_x = %d, m_y = %d, m_z = %d, time = %lluus", m_x, m_y, m_z, m_fired_time);

	return true;
}

bool accel_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int accel_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	const int chance = 3;
	int retry = 0;

	while ((m_fired_time == 0) && (retry++ < chance)) {
		INFO("Try usleep for getting a valid BASE DATA value");
		usleep(m_polling_interval * 1000llu);
	}
	if (m_fired_time == 0) {
		ERR("get_struct_value failed");
		return -1;
	}

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_VENDOR_UNIT;
	data.timestamp = m_fired_time ;
	data.values_num = 3;
	data.values[0] = m_x;
	data.values[1] = m_y;
	data.values[2] = m_z;

	return 0;
}


bool accel_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_METRE_PER_SECOND_SQUARED;
	properties.sensor_min_range = MIN_RANGE(m_resolution)* RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(m_raw_data_unit);
	properties.sensor_max_range = MAX_RANGE(m_resolution)* RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(m_raw_data_unit);
	snprintf(properties.sensor_name,   sizeof(properties.sensor_name),"%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor),"%s", m_vendor.c_str());
	properties.sensor_resolution = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(m_raw_data_unit);
	return true;
}

bool accel_sensor_hal::calibration(int cmd)
{
	if (cmd == CAL_CHECK) {

		struct calibration_data {
			short x;
			short y;
			short z;
		};

		struct calibration_data cal_data;

		if (access(CALIBRATION_FILE, F_OK) == 0) {

			FILE *fp = NULL;
			fp = fopen(CALIBRATION_FILE, "r");

			if (!fp) {
				ERR("cannot open calibration file");
				return false;
			}

			size_t read_cnt;

			read_cnt = fread(&cal_data, sizeof(cal_data), 1, fp);

			if (read_cnt != 1) {
				ERR("cal_data read fail, read_cnt = %d\n",read_cnt);
				fclose(fp);
				return false;
			}

			fclose(fp);

			INFO("x = [%d] y = [%d] z = [%d]",cal_data.x, cal_data.y, cal_data.z);

			if (cal_data.x == 0 && cal_data.y == 0 && cal_data.z == 0) {
				DBG("cal_data values is zero");
				return false;
			} else
				return true;
		} else {
			INFO("cannot access calibration file");
			return false;
		}
	} else if (cmd == CAL_SET) {
		if (mkdir(CALIBRATION_DIR,0755) != 0)
			INFO("mkdir fail");

		FILE *fp;
		fp = fopen(CALIBRATION_NODE, "w");

		if (!fp) {
			ERR("Failed to open a calibration file\n");
			return false;
		}

		if (fprintf(fp, "%d", cmd) < 0) {
			ERR("Failed to set calibration\n");
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	} else if (cmd == CAL_MKDIR) {
		if (mkdir(CALIBRATION_DIR,0755) != 0) {
			ERR("mkdir fail");
			return false;
		}

		return true;
	}

	ERR("Non supported calibration cmd = %d\n" , cmd);
	return false;
}

bool accel_sensor_hal::is_sensorhub_supported(void)
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

bool accel_sensor_hal::check_hw_node(void)
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

			if (!infile) {
				continue;
			}

			infile >> hw_name;

			if (CConfig::get_instance().is_supported(SENSOR_TYPE_ACCEL,hw_name) == true) {
				m_name = m_model_id = hw_name;
				INFO("m_model_id = %s\n",m_model_id.c_str());
				find_node = true;
				calibration(CAL_MKDIR);
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
					DBG("Find H/W  for accel_sensor\n");

					find_node = true;

					string dir_name;
					dir_name = string(dir_entry->d_name);
					unsigned found = dir_name.find_first_not_of("input");

					m_resource = string("/dev/input/event") + dir_name.substr(found);
					if (m_sensorhub_supported) {
						m_enable_resource = string("/sys/class/sensors/ssp_sensor/enable");
						m_polling_resource = string("/sys/class/sensors/ssp_sensor/accel_poll_delay");
					} else {
						m_enable_resource = string("/sys/class/input/") + string(dir_entry->d_name) + string("/enable");
						m_polling_resource = string("/sys/class/input/") + string(dir_entry->d_name) + string("/poll_delay");
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
		INFO("m_polling_resource = %s\n", m_polling_resource.c_str());
	}

	return find_node;

}

extern "C" void *create(void)
{
	accel_sensor_hal *inst;

	try {
		inst = new accel_sensor_hal();
	} catch (int err) {
		ERR("accel_sensor class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (accel_sensor_hal*)inst;
}
