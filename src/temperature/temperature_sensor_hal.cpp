/*
 * temperature_sensor_hal
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
#include <csensor_config.h>
#include <temperature_sensor_hal.h>
#include <fstream>
#include <iio_common.h>

using std::ifstream;
using config::csensor_config;

#define SENSOR_TYPE_TEMPERATURE		"TEMPERATURE"
#define ELEMENT_NAME				"NAME"
#define ELEMENT_VENDOR				"VENDOR"
#define ELEMENT_RAW_DATA_UNIT		"RAW_DATA_UNIT"

#define ENABLE_VAL			true
#define DISABLE_VAL			false
#define NO_FLAG				0
#define TIMEOUT				1
#define INITIAL_TIME		0
#define NO_OF_DATA_VAL		1

temperature_sensor_hal::temperature_sensor_hal()
: m_temperature(0)
, m_sensorhub_supported(false)
{
	int fd, ret;
	string file_name;

	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	csensor_config &config = csensor_config::get_instance();

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit))
	{
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);

	file_name = string(IIO_DIR) + m_temperature_dir + string(TEMP_SCALE);
	if (!read_node_value<int>(file_name, m_temp_scale))
		throw ENXIO;

	file_name = string(IIO_DIR) + m_temperature_dir + string(TEMP_OFFSET);
	if (!read_node_value<float>(file_name, m_temp_offset))
		throw ENXIO;

	INFO("Temperature scale:%d", m_temp_scale);
	INFO("Temperature offset:%f", m_temp_offset);

	INFO("temperature_sensor_hal is created!\n");
}

temperature_sensor_hal::~temperature_sensor_hal()
{
	INFO("temperature_sensor_hal is destroyed!\n");
}

string temperature_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t temperature_sensor_hal::get_type(void)
{
	return TEMPERATURE_SENSOR;
}

bool temperature_sensor_hal::enable_resource(bool enable)
{
	INFO("Enable not supported");
	return true;
}

bool temperature_sensor_hal::enable(void)
{
	enable_resource(ENABLE_VAL);
	return true;
}

bool temperature_sensor_hal::disable(void)
{
	enable_resource(DISABLE_VAL);
	return true;
}

bool temperature_sensor_hal::set_interval(unsigned long val)
{
	return true;
}

bool temperature_sensor_hal::update_value(bool wait)
{
	int raw_temp_count;

	if (!read_node_value<int>(m_temp_node, raw_temp_count))
		return false;
	m_temperature = m_temp_offset + ((float)raw_temp_count)/((float)m_temp_scale);
	return true;
}

bool temperature_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int temperature_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);
	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_CELSIUS;
	data.timestamp = INITIAL_TIME;
	data.values_num = NO_OF_DATA_VAL;
	data.values[0] = m_temperature;
	return 0;
}

bool temperature_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_CELSIUS;
	snprintf(properties.sensor_name, sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = m_raw_data_unit;
	return true;
}

bool temperature_sensor_hal::is_sensorhub_supported(void)
{
	return false;
}

bool temperature_sensor_hal::check_hw_node(void)
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
				if (CConfig::get_instance().is_supported(SENSOR_TYPE_TEMPERATURE, hw_name) == true)
				{
					m_model_id = hw_name;
					m_temperature_dir = string(dir_entry->d_name);
					m_temp_node = string(IIO_DIR) + m_temperature_dir + string(TEMP_RAW);

					INFO("m_model_id = %s", m_model_id.c_str());
					INFO("m_temperature_dir = %s", m_temperature_dir.c_str());

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
	temperature_sensor_hal *inst;

	try
	{
		inst = new temperature_sensor_hal();
	}
	catch (int err)
	{
		ERR("temperature_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (temperature_sensor_hal*)inst;
}
