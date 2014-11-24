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
#include <csensor_config.h>
#include <proxi_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <iio_common.h>

using std::ifstream;

#define NO_FLAG			0
#define PROXIMITY_TYPE	8

#define EVENT_DIR		"events/"
#define EVENT_EN_NODE	"in_proximity_thresh_either_en"

#define SENSOR_TYPE_PROXI		"PROXI"
#define ELEMENT_NAME 			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE 				"value"

#define PROXI_CODE	0x0019

proxi_sensor_hal::proxi_sensor_hal()
: m_state(PROXIMITY_STATE_FAR)
, m_fired_time(0)
, m_node_handle(-1)
{
	const string sensorhub_interval_node_name = "prox_poll_delay";
	csensor_config &config = csensor_config::get_instance();

	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_PROXI, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;

	}

	query.input_method = input_method;
	query.sensorhub_controlled = m_sensorhub_controlled = false;
	query.sensor_type = SENSOR_TYPE_PROXI;
	query.input_event_key = "proximity_sensor";
	query.iio_enable_node_name = "proximity_enable";
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_path_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	m_data_node = info.data_node_path;
	m_enable_node = info.base_dir + string(EVENT_DIR) + string(EVENT_EN_NODE);

	INFO("data node: %s",m_data_node.c_str());
	INFO("enable node: %s",m_enable_node.c_str());

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

	int fd, ret;
	fd = open(m_data_node.c_str(), NO_FLAG);
	if (fd == -1) {
		ERR("Could not open event resource");
		throw ENXIO;
	}

	ret = ioctl(fd, IOCTL_IIO_EVENT_FD, &m_node_handle);

	close(fd);

	if ((ret == -1) || (m_node_handle == -1)) {
		ERR("Failed to retrieve event fd");
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

bool proxi_sensor_hal::enable_resource(bool enable)
{
	update_sysfs_num(m_enable_node.c_str(), enable);
	return true;
}

bool proxi_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(true);

	m_fired_time = 0;
	INFO("Proxi sensor real starting");
	return true;
}

bool proxi_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(true);

	INFO("Proxi sensor real stopping");
	return true;
}

bool proxi_sensor_hal::update_value(bool wait)
{
	iio_event_t proxi_event;
	fd_set readfds, exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);
	FD_SET(m_node_handle, &readfds);
	FD_SET(m_node_handle, &exceptfds);

	int ret;
	ret = select(m_node_handle + 1, &readfds, NULL, &exceptfds, NULL);

	if (ret == -1) {
		ERR("select error:%s m_node_handle:%d", strerror(errno), m_node_handle);
		return false;
	}
	else if (!ret) {
		DBG("select timeout");
		return false;
	}

	if (FD_ISSET(m_node_handle, &exceptfds)) {
		ERR("select exception occurred!");
		return false;
	}

	if (FD_ISSET(m_node_handle, &readfds)) {
		INFO("proximity event detection!");
		int len = read(m_node_handle, &proxi_event, sizeof(proxi_event));

		if (len == -1) {
			DBG("Error in read(m_node_handle):%s.", strerror(errno));
			return false;
		}

		ull_bytes_t ev_data;
		ev_data.num = proxi_event.event_id;
		if (ev_data.bytes[CH_TYPE] == PROXIMITY_TYPE) {
			AUTOLOCK(m_value_mutex);
			int temp;
			temp = GET_DIR_VAL(ev_data.bytes[DIRECTION]);
			if (temp == PROXIMITY_NODE_STATE_FAR) {
				INFO("PROXIMITY_STATE_FAR state occurred");
				m_state = PROXIMITY_STATE_FAR;
			}
			else if (temp == PROXIMITY_NODE_STATE_NEAR) {
				INFO("PROXIMITY_STATE_NEAR state occurred");
				m_state = PROXIMITY_STATE_NEAR;
			}
			else {
				ERR("PROXIMITY_STATE Unknown: %d", proxi_event.event_id);
				return false;
			}
		}
		m_fired_time = proxi_event.timestamp;
	}
	else {
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
	data.accuracy = SENSOR_ACCURACY_UNDEFINED;
	data.timestamp = m_fired_time;
	data.value_count = 1;
	data.values[0] = m_state;

	return 0;
}

bool proxi_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = 0;
	properties.max_range = 1;
	properties.min_interval = 1;
	properties.resolution = 1;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	return true;
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
