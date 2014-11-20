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
#include <csensor_config.h>
#include <light_sensor_hal.h>
#include <iio_common.h>

using std::ifstream;
using config::csensor_config;

#define BIAS				1
#define INITIAL_VALUE		-1
#define INITIAL_TIME		0
#define NO_OF_DATA_VAL		1

#define SENSOR_TYPE_LIGHT		"LIGHT"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"
#define ATTR_VALUE				"value"

light_sensor_hal::light_sensor_hal()
: m_adc(INITIAL_VALUE)
, m_sensorhub_supported(false)
{
	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	csensor_config &config = csensor_config::get_instance();

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());
	INFO("light_sensor_hal is created!");
}

light_sensor_hal::~light_sensor_hal()
{
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

bool light_sensor_hal::enable(void)
{
	INFO("Resource already enabled. Enable not supported.");
	return true;
}

bool light_sensor_hal::disable(void)
{
	INFO("Disable not supported.");
	return true;
}

bool light_sensor_hal::set_interval(unsigned long val)
{
	INFO("Polling not supported. Polling interval cannot be changed.");
	return true;
}

bool light_sensor_hal::update_value(void)
{
	unsigned short int adc = INITIAL_VALUE;
	if (!read_node_value<unsigned short int>(m_clear_raw_node, adc))
	{
		INFO("Read Value Failed. clear val: %d", adc);
		return false;
	}
	INFO("Read Value success. Light Sensor clear val: %d", adc);
	AUTOLOCK(m_value_mutex);
	m_adc = (int)adc;

	return true;
}

bool light_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value();
	return ret;
}

int light_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);
	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_LUX;
	data.timestamp = INITIAL_TIME;
	data.values_num = NO_OF_DATA_VAL;
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
	return false;
}

bool light_sensor_hal::check_hw_node(void)
{
	string name_node;
	string hw_name;
	string file_name;
	DIR *main_dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find_node = false;

	INFO("======================start check_hw_node=============================");

	m_sensorhub_supported = is_sensorhub_supported();
	main_dir = opendir(IIO_DIR);

	if (!main_dir)
	{
		ERR("Directory open failed to collect data");
		return false;
	}

	while (!find_node)
	{
		dir_entry = readdir(main_dir);
		if(dir_entry == NULL)
			break;

		if ((strncasecmp(dir_entry->d_name , ".", 1 ) != 0) && (strncasecmp(dir_entry->d_name , "..", 2 ) != 0) && (dir_entry->d_ino != 0))
		{
			file_name = string(IIO_DIR) + string(dir_entry->d_name) + string(NAME_NODE);

			ifstream infile(file_name.c_str());

			if (!infile)
				continue;

			infile >> hw_name;

			if (strncmp(dir_entry->d_name, IIO_DEV_BASE_NAME, IIO_DEV_STR_LEN) == 0)
			{
				if (CConfig::get_instance().is_supported(SENSOR_TYPE_LIGHT, hw_name) == true)
				{
					m_name = m_model_id = hw_name;
					m_clear_raw_node = string(IIO_DIR) + string(dir_entry->d_name) + string(ILL_CLEAR_NODE);
					INFO("m_model_id = %s", m_model_id.c_str());
					INFO("m_clear_raw_node = %s", m_clear_raw_node.c_str());
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
