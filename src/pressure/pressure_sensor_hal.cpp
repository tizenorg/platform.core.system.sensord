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
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <linux/input.h>
#include <cconfig.h>

#include <pressure_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cconfig.h>
#include <iio_common.h>

using std::ifstream;
using config::CConfig;

#define SENSOR_TYPE_PRESSURE	"PRESSURE"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_MIN_RANGE		"MIN_RANGE"
#define ELEMENT_MAX_RANGE		"MAX_RANGE"

#define ENABLE_VAL			true
#define DISABLE_VAL			false
#define SEA_LEVEL_PRESSURE	101325.0
#define NO_FLAG				0
#define TIMEOUT				1

pressure_sensor_hal::pressure_sensor_hal()
: m_pressure(0)
, m_temperature(0)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(0)
, m_sensorhub_supported(false)
{
	int fd, ret;
	string file_name;

	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	double min_range;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_MIN_RANGE, min_range))
	{
		ERR("[MIN_RANGE] is empty\n");
		throw ENXIO;
	}

	m_min_range = (float)min_range;
	INFO("m_min_range = %f\n",m_min_range);

	double max_range;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_MAX_RANGE, max_range))
	{
		ERR("[MAX_RANGE] is empty\n");
		throw ENXIO;
	}

	m_max_range = (float)max_range;
	INFO("m_max_range = %f\n",m_max_range);

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit))
	{
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);

	file_name = string(IIO_DIR) + m_pressure_dir + string(TEMP_SCALE);
	if (!read_node_value<int>(file_name, m_temp_scale))
		throw ENXIO;

	file_name = string(IIO_DIR) + m_pressure_dir + string(TEMP_OFFSET);
	if (!read_node_value<float>(file_name, m_temp_offset))
		throw ENXIO;

	file_name = string(IIO_DIR) + m_pressure_dir + string(PRESSURE_SCALE);
	if (!read_node_value<int>(file_name, m_pressure_scale))
		throw ENXIO;

	INFO("Temperature scale:%d", m_temp_scale);
	INFO("Temperature offset:%f", m_temp_offset);
	INFO("Pressure scale:%d", m_pressure_scale);

	fd = open(m_event_resource.c_str(), NO_FLAG);
	if (fd == -1)
	{
		ERR("Could not open event resource");
		throw ENXIO;
	}

	ret = ioctl(fd, IOCTL_IIO_EVENT_FD, &m_event_fd);

	close(fd);

	if ((ret == -1) || (m_event_fd == -1))
	{
		ERR("Failed to retrieve event fd");
		throw ENXIO;
	}

	INFO("pressure_sensor_hal is created!\n");
}

pressure_sensor_hal::~pressure_sensor_hal()
{
	close(m_event_fd);
	INFO("pressure_sensor_hal is destroyed!\n");
}

string pressure_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t pressure_sensor_hal::get_type(void)
{
	return PRESSURE_SENSOR;
}

bool pressure_sensor_hal::enable_resource(bool enable)
{
	update_sysfs_num(m_enable_resource.c_str(), enable);
	return true;
}

bool pressure_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(ENABLE_VAL);

	m_fired_time = 0;
	INFO("Pressure sensor real starting");
	return true;
}

bool pressure_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(DISABLE_VAL);

	INFO("Pressure sensor real stopping");
	return true;
}

bool pressure_sensor_hal::set_interval(unsigned long val)
{
	return true;
}

bool pressure_sensor_hal::update_value(bool wait)
{
	iio_event_t pressure_event;
	fd_set readfds, exceptfds;
	struct timeval tv;
	int raw_pressure_count;
	int raw_temp_count;
	int ret;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_SET(m_event_fd, &readfds);
	FD_SET(m_event_fd, &exceptfds);

	if (wait)
	{
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
	}
	else
	{
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	}

	ret = select(m_event_fd + 1, &readfds, NULL, &exceptfds, &tv);

	if (ret == -1)
	{
		ERR("select error:%s m_event_fd:d", strerror(errno), m_event_fd);
		return false;
	}
	else if (!ret)
	{
		DBG("select timeout");
		return false;
	}

	if (FD_ISSET(m_event_fd, &exceptfds))
	{
		ERR("select exception occurred!");
		return false;
	}

	if (FD_ISSET(m_event_fd, &readfds))
	{
		INFO("pressure event detection!");
		int len = read(m_event_fd, &pressure_event, sizeof(pressure_event));

		if (len == -1)
		{
			DBG("Error in read(m_event_fd):%s.", strerror(errno));
			return false;
		}
		m_fired_time = pressure_event.timestamp;
		if (!read_node_value<int>(m_pressure_node, raw_pressure_count))
			return false;
		if (!read_node_value<int>(m_temp_node, raw_temp_count))
			return false;
		m_pressure = ((float)raw_pressure_count)/((float)m_pressure_scale);
		m_temperature = m_temp_offset + ((float)raw_temp_count)/((float)m_temp_scale);
	}
	else
	{
		ERR("No pressure event data available to read");
		return false;
	}
	return true;
}

bool pressure_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int pressure_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);
	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_HECTOPASCAL;
	data.timestamp = m_fired_time ;
	data.values_num = 3;
	data.values[0] = m_pressure;
	data.values[1] = SEA_LEVEL_PRESSURE;
	data.values[2] = m_temperature;

	return 0;
}

bool pressure_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_HECTOPASCAL;
	properties.sensor_min_range = m_min_range;
	properties.sensor_max_range = m_max_range;
	snprintf(properties.sensor_name, sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = m_raw_data_unit;
	return true;
}

bool pressure_sensor_hal::is_sensorhub_supported(void)
{
	return false;
}

bool pressure_sensor_hal::check_hw_node(void)
{
	string name_node;
	string hw_name;
	string file_name;

	DIR *main_dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find_node = false;

	INFO("======================start check_hw_node=============================\n");

	m_sensorhub_supported = is_sensorhub_supported();

	main_dir = opendir(IIO_DIR);

	if (!main_dir)
	{
		ERR("Could not open IIO directory\n");
		return false;
	}

	while (!find_node)
	{
		dir_entry = readdir(main_dir);
		if(dir_entry == NULL)
			break;

		if ((strncasecmp(dir_entry->d_name ,".",1 ) != 0) && (strncasecmp(dir_entry->d_name ,"..",2 ) != 0) && (dir_entry->d_ino != 0))
		{
			file_name = string(IIO_DIR) + string(dir_entry->d_name) + string(NAME_NODE);

			ifstream infile(file_name.c_str());

			if (!infile)
				continue;

			infile >> hw_name;

			if (strncmp(dir_entry->d_name, IIO_DEV_BASE_NAME, IIO_DEV_STR_LEN) == 0)
			{
				if (CConfig::get_instance().is_supported(SENSOR_TYPE_PRESSURE, hw_name) == true)
				{
					m_name = m_model_id = hw_name;
					m_pressure_dir = string(dir_entry->d_name);
					m_enable_resource = string(IIO_DIR) + m_pressure_dir + string(EVENT_DIR) + string(EVENT_EN_NODE);
					m_event_resource = string(DEV_DIR) + m_pressure_dir;
					m_pressure_node = string(IIO_DIR) + m_pressure_dir + string(PRESSURE_RAW);
					m_temp_node = string(IIO_DIR) + m_pressure_dir + string(TEMP_RAW);

					INFO("m_enable_resource = %s", m_enable_resource.c_str());
					INFO("m_model_id = %s", m_model_id.c_str());
					INFO("m_pressure_dir = %s", m_pressure_dir.c_str());
					INFO("m_event_resource = %s", m_event_resource.c_str());

					find_node = true;
					break;
				}
			}
		}
	}

	closedir(main_dir);
	return find_node;
}

extern "C" void *create(void)
{
	pressure_sensor_hal *inst;

	try
	{
		inst = new pressure_sensor_hal();
	}
	catch (int err)
	{
		ERR("pressure_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (pressure_sensor_hal*)inst;
}
