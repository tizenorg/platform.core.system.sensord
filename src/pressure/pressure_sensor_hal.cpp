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
#include <csensor_config.h>
#include <sys/ioctl.h>
#include <pressure_sensor_hal.h>
#include <fstream>
#include <string>
#include <iio_common.h>

using std::ifstream;
using std::string;

#define SENSOR_TYPE_PRESSURE	"PRESSURE"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"
#define ELEMENT_MIN_RANGE		"MIN_RANGE"
#define ELEMENT_MAX_RANGE		"MAX_RANGE"
#define ELEMENT_TEMPERATURE_RESOLUTION	"TEMPERATURE_RESOLUTION"
#define ELEMENT_TEMPERATURE_OFFSET		"TEMPERATURE_OFFSET"
#define ATTR_VALUE				"value"

#define SEA_LEVEL_PRESSURE 101325.0

#define EVENT_EN_NODE	"events/in_pressure_mag_either_en"
#define PRESSURE_SCALE	"/in_pressure_scale"
#define PRESSURE_RAW	"/in_pressure_raw"
#define TEMP_OFFSET		"/in_temp_offset"
#define TEMP_SCALE		"/in_temp_scale"
#define TEMP_RAW		"/in_temp_raw"
#define NO_FLAG			0
#define TIMEOUT			1

pressure_sensor_hal::pressure_sensor_hal()
: m_pressure(0)
, m_temperature(0)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(0)
, m_node_handle(-1)
{
	const string sensorhub_interval_node_name = "pressure_poll_delay";
	csensor_config &config = csensor_config::get_instance();

	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_PRESSURE, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;

	}

	query.input_method = input_method;
	query.sensorhub_controlled = m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_PRESSURE;
	query.input_event_key = "pressure_sensor";
	query.iio_enable_node_name = EVENT_EN_NODE;
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_path_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}
	m_data_node = info.data_node_path;
	m_pressure_dir = info.base_dir;
	m_enable_node = info.enable_node_path;
	m_pressure_node = m_pressure_dir + string(PRESSURE_RAW);
	m_temp_node = m_pressure_dir + string(TEMP_RAW);

	INFO("m_data_node:%s",m_data_node.c_str());
	INFO("m_pressure_dir:%s",m_pressure_dir.c_str());
	INFO("m_enable_node:%s",m_enable_node.c_str());

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	double min_range;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_MIN_RANGE, min_range)) {
		ERR("[MIN_RANGE] is empty\n");
		throw ENXIO;
	}

	m_min_range = (float)min_range;
	INFO("m_min_range = %f\n",m_min_range);

	double max_range;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_MAX_RANGE, max_range)) {
		ERR("[MAX_RANGE] is empty\n");
		throw ENXIO;
	}

	m_max_range = (float)max_range;
	INFO("m_max_range = %f\n",m_max_range);

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_PRESSURE, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);

	string file_name;

	file_name = m_pressure_dir + string(TEMP_SCALE);
	if (!read_node_value<int>(file_name, m_temp_scale))
		throw ENXIO;

	file_name = m_pressure_dir + string(TEMP_OFFSET);
	if (!read_node_value<float>(file_name, m_temp_offset))
		throw ENXIO;

	file_name = m_pressure_dir + string(PRESSURE_SCALE);
	if (!read_node_value<int>(file_name, m_pressure_scale))
		throw ENXIO;

	INFO("Temperature scale:%d", m_temp_scale);
	INFO("Temperature offset:%f", m_temp_offset);
	INFO("Pressure scale:%d", m_pressure_scale);

	int fd, ret;
	fd = open(m_data_node.c_str(), NO_FLAG);
	if (fd == -1) {
		ERR("Could not open event resource");
		throw ENXIO;
	}

	ret = ioctl(fd, IOCTL_IIO_EVENT_FD, &m_node_handle);

	close(fd);

	if ((ret == -1) || (m_node_handle == -1)) {
		ERR("Failed to retrieve node handle from event node: %s", m_data_node.c_str());
		throw ENXIO;
	}

	INFO("pressure_sensor_hal is created!\n");
}

pressure_sensor_hal::~pressure_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

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

bool pressure_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);
	update_sysfs_num(m_enable_node.c_str(), true);
	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Pressure sensor real starting");
	return true;
}

bool pressure_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	update_sysfs_num(m_enable_node.c_str(), false);

	INFO("Pressure sensor real stopping");
	return true;
}

bool pressure_sensor_hal::set_interval(unsigned long val)
{
	INFO("set_interval not supported");
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
	FD_SET(m_node_handle, &readfds);
	FD_SET(m_node_handle, &exceptfds);

	if (wait) {
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;
	}
	else {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	}

	ret = select(m_node_handle + 1, &readfds, NULL, &exceptfds, &tv);

	if (ret == -1) {
		ERR("select error:%s m_node_handle:d", strerror(errno), m_node_handle);
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
		INFO("pressure event detection!");
		int len = read(m_node_handle, &pressure_event, sizeof(pressure_event));

		if (len == -1) {
			ERR("Error in read(m_event_fd):%s.", strerror(errno));
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
	else {
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
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time ;
	data.value_count = 3;
	data.values[0] = m_pressure;
	data.values[1] = SEA_LEVEL_PRESSURE;
	data.values[2] = m_temperature;

	return 0;
}


bool pressure_sensor_hal::get_properties(sensor_properties_s &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = m_min_range;
	properties.max_range = m_max_range;
	properties.min_interval = 1;
	properties.resolution = m_raw_data_unit;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	return true;
}

extern "C" void *create(void)
{
	pressure_sensor_hal *inst;

	try {
		inst = new pressure_sensor_hal();
	} catch (int err) {
		ERR("pressure_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (pressure_sensor_hal*)inst;
}
