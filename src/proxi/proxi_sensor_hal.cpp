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
#include <proxi_sensor_hal.h>
#include <iio_common.h>

using std::ifstream;
using config::CConfig;

#define INITIAL_VALUE -1
#define INITIAL_TIME 0

#define SENSOR_TYPE_PROXI		"PROXI"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE				"value"

#define INPUT_NAME	"proximity_sensor"

#define NO_FLAG			0
#define ENABLE_VAL		true
#define DISABLE_VAL		false

proxi_sensor_hal::proxi_sensor_hal()
: m_state(PROXIMITY_STATE_FAR)
, m_node_handle(INITIAL_VALUE)
, m_fired_time(INITIAL_TIME)
, m_sensorhub_supported(false)
{
	int fd, ret;

	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_PROXI, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_PROXI, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

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

	INFO("proxi_sensor_hal is created!");
}

proxi_sensor_hal::~proxi_sensor_hal()
{
	close(m_event_fd);
	INFO("proxi_sensor_hal is destroyed!");
}

string proxi_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t proxi_sensor_hal::get_type(void)
{
	return PROXIMITY_SENSOR;
}

bool proxi_sensor_hal::enable_resource(bool enable)
{
	update_sysfs_num(m_enable_resource.c_str(), enable);
	return true;
}

bool proxi_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(ENABLE_VAL);

	m_fired_time = 0;
	INFO("Proximity sensor real starting");
	return true;
}

bool proxi_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(DISABLE_VAL);

	INFO("Proximity sensor real stopping");
	return true;
}

bool proxi_sensor_hal::update_value(bool wait)
{
	iio_event_t proxi_event;
	fd_set readfds, exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_SET(m_event_fd, &readfds);
	FD_SET(m_event_fd, &exceptfds);

	int ret;
	ret = select(m_event_fd + 1, &readfds, NULL, &exceptfds, NULL);

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
		INFO("proximity event detection!");
		int len = read(m_event_fd, &proxi_event, sizeof(proxi_event));

		if (len == -1)
		{
			DBG("Error in read(m_event_fd):%s.", strerror(errno));
			return false;
		}

		ull_bytes_t ev_data;
		ev_data.num = proxi_event.event_id;
		if (ev_data.bytes[CH_TYPE] == PROXIMITY_TYPE)
		{
			AUTOLOCK(m_value_mutex);
			int temp;
			temp = GET_DIR_VAL(ev_data.bytes[DIRECTION]);
			if (temp == PROXIMITY_NODE_STATE_FAR)
			{
				INFO("PROXIMITY_STATE_FAR state occurred");
				m_state = PROXIMITY_STATE_FAR;
			}
			else if (temp == PROXIMITY_NODE_STATE_NEAR)
			{
				INFO("PROXIMITY_STATE_NEAR state occurred");
				m_state = PROXIMITY_STATE_NEAR;
			}
			else
			{
				ERR("PROXIMITY_STATE Unknown: %d", proxi_event.event_id);
				return false;
			}
		}
		m_fired_time = proxi_event.timestamp;
	}
	else
	{
		ERR("No proximity event data available to read");
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
	data.values[0] = (float)(m_state);
	return 0;
}

bool proxi_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_STATE_ON_OFF;
	properties.sensor_min_range = 0;
	properties.sensor_max_range = 1;
	snprintf(properties.sensor_name,   sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = 1;
	return true;
}

bool proxi_sensor_hal::is_sensorhub_supported(void)
{
	return false;
}

bool proxi_sensor_hal::check_hw_node(void)
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

			if (strncmp(dir_entry->d_name, IIO_DEV_BASE_NAME, IIO_DEV_STR_LEN) == 0)
			{
				if (CConfig::get_instance().is_supported(SENSOR_TYPE_PROXI, hw_name) == true)
				{
					m_name = m_model_id = hw_name;
					m_proxi_dir = string(dir_entry->d_name);
					m_enable_resource = string(IIO_DIR) + m_proxi_dir + string(EVENT_DIR) + string(EVENT_EN_NODE);
					m_event_resource = string(DEV_DIR) + m_proxi_dir;

					INFO("m_enable_resource = %s", m_enable_resource.c_str());
					INFO("m_model_id = %s", m_model_id.c_str());
					INFO("m_proxi_dir = %s", m_proxi_dir.c_str());
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
	proxi_sensor_hal *inst;

	try
	{
		inst = new proxi_sensor_hal();
	}
	catch (int err)
	{
		ERR("Failed to create proxi_sensor_hal class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (proxi_sensor_hal *)inst;
}
