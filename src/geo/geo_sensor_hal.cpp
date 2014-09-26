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
#include <iio_common.h>

using std::ifstream;
using std::string;
using config::CConfig;

#define SENSOR_TYPE_MAGNETIC	"MAGNETIC"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE				"value"

#define SENSOR_MIN_RANGE		-1200
#define SENSOR_MAX_RANGE		1200
#define INITIAL_TIME			0
#define INITIAL_VALUE			-1
#define GAUSS_TO_UTESLA(val)	((val) * 100)

geo_sensor_hal::geo_sensor_hal()
: m_x(INITIAL_VALUE)
, m_y(INITIAL_VALUE)
, m_z(INITIAL_VALUE)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(INITIAL_TIME)
, m_sensorhub_supported(false)
{
	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	if (!init_resources())
		throw ENXIO;

	INFO("geo_sensor_hal is created!");
}

geo_sensor_hal::~geo_sensor_hal()
{
	INFO("geo_sensor_hal is destroyed!");
}

bool geo_sensor_hal::init_resources(void)
{
	ifstream temp_handle;

	if (!read_node_value<double>(m_x_scale_node, m_x_scale))
		return false;

	if (!read_node_value<double>(m_y_scale_node, m_y_scale))
		return false;

	if (!read_node_value<double>(m_z_scale_node, m_z_scale))
		return false;

	INFO("Scale Values: %f, %f, %f", m_x_scale, m_y_scale, m_z_scale);
	return true;
}

string geo_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t geo_sensor_hal::get_type(void)
{
	return GEOMAGNETIC_SENSOR;
}

bool geo_sensor_hal::enable(void)
{
	INFO("Resource already enabled. Enable not supported.");
	return true;
}

bool geo_sensor_hal::disable(void)
{
	INFO("Disable not supported.");
	return true;
}

bool geo_sensor_hal::set_interval(unsigned long val)
{
	INFO("Polling interval cannot be changed.");
	return true;
}

bool geo_sensor_hal::update_value(void)
{
	int raw_values[3] = {0,};
	ifstream temp_handle;

	if (!read_node_value<int>(m_x_node, raw_values[0]))
		return false;

	if (!read_node_value<int>(m_y_node, raw_values[1]))
		return false;

	if (!read_node_value<int>(m_z_node, raw_values[2]))
		return false;

	m_x = GAUSS_TO_UTESLA(raw_values[0] * m_x_scale);
	m_y = GAUSS_TO_UTESLA(raw_values[1] * m_y_scale);
	m_z = GAUSS_TO_UTESLA(raw_values[2] * m_z_scale);

	m_fired_time = INITIAL_TIME;
	INFO("x = %d, y = %d, z = %d, time = %lluus", raw_values[0], raw_values[0], raw_values[0], m_fired_time);

	return true;
}

bool geo_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value();
	return ret;
}

int geo_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	data.data_unit_idx = SENSOR_UNIT_MICRO_TESLA;
	data.timestamp = m_fired_time;
	data.values_num = 3;
	data.values[0] = (float)m_x;
	data.values[1] = (float)m_y;
	data.values[2] = (float)m_z;
	return 0;
}

bool geo_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_MICRO_TESLA;
	properties.sensor_min_range = SENSOR_MIN_RANGE;
	properties.sensor_max_range = SENSOR_MAX_RANGE;
	snprintf(properties.sensor_name, sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = 1;
	return true;
}

bool geo_sensor_hal::is_sensorhub_supported(void)
{
	return false;
}

bool geo_sensor_hal::check_hw_node(void)
{
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
		ERR("Could not open IIO directory\n");
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

			if (CConfig::get_instance().is_supported(SENSOR_TYPE_MAGNETIC, hw_name) == true)
			{
				m_name = m_model_id = hw_name;
				INFO("m_model_id = %s", m_model_id.c_str());

				string temp = string(IIO_DIR) + string(dir_entry->d_name);

				m_x_node = temp + string(X_RAW_VAL_NODE);
				m_y_node = temp + string(Y_RAW_VAL_NODE);
				m_z_node = temp + string(Z_RAW_VAL_NODE);
				m_x_scale_node = temp + string(X_SCALE_NODE);
				m_y_scale_node = temp + string(Y_SCALE_NODE);
				m_z_scale_node = temp + string(Z_SCALE_NODE);

				find_node = true;
				break;
			}
		}
	}

	closedir(main_dir);
	return find_node;
}

extern "C" void *create(void)
{
	geo_sensor_hal *inst;

	try
	{
		inst = new geo_sensor_hal();
	}
	catch (int err)
	{
		ERR("Failed to create geo_sensor_hal class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (geo_sensor_hal *)inst;
}
