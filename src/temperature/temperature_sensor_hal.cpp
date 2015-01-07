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
#include <fstream>
#include <temperature_sensor_hal.h>
#include <sys/ioctl.h>
#include <iio_common.h>

using std::ifstream;

#define SENSOR_TYPE_TEMPERATURE		"TEMPERATURE"
#define ELEMENT_NAME				"NAME"
#define ELEMENT_VENDOR				"VENDOR"
#define ELEMENT_RAW_DATA_UNIT		"RAW_DATA_UNIT"

#define TEMP_INPUT_NAME					"temperature_sensor"
#define TEMP_IIO_ENABLE_NODE_NAME		"temp_enable"
#define TEMP_SENSORHUB_POLL_NODE_NAME 	"temp_poll_delay"
#define INITIAL_TIME -1

temperature_sensor_hal::temperature_sensor_hal()
: m_temperature(0)
, m_node_handle(-1)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(INITIAL_TIME)
{
	const string sensorhub_interval_node_name = TEMP_SENSORHUB_POLL_NODE_NAME;
	string file_name;
	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_TEMPERATURE, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;
	}

	query.input_method = input_method;
	query.sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_TEMPERATURE;
	query.input_event_key = TEMP_INPUT_NAME;
	query.iio_enable_node_name = TEMP_IIO_ENABLE_NODE_NAME;
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_path_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	show_node_path_info(info);

	m_data_node = info.data_node_path;
	m_enable_node = info.enable_node_path;
	m_interval_node = info.interval_node_path;

	if(input_method == IIO_METHOD) {
		m_temperature_dir=info.base_dir;
		m_temp_node = m_temperature_dir + string(TEMP_RAW);
		INFO("m_temperature_dir = %s", m_temperature_dir.c_str());
		INFO("m_temp_node = %s", m_temp_node.c_str());
	}
	csensor_config &config = csensor_config::get_instance();

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_TEMPERATURE, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);

	INFO("m_data_node = %s\n",m_data_node.c_str());

	if ((m_node_handle = open(m_temp_node.c_str(),O_RDWR)) < 0) {
		ERR("Failed to open handle(%d)", m_node_handle);
		throw ENXIO;
	}

	INFO("m_data_node = %s\n",m_data_node.c_str());
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);

	file_name = m_temperature_dir + string(TEMP_SCALE);
	if (!read_node_value<int>(file_name, m_temp_scale))
		throw ENXIO;

	file_name = m_temperature_dir + string(TEMP_OFFSET);
	if (!read_node_value<float>(file_name, m_temp_offset))
		throw ENXIO;

	INFO("m_temp_offset %f",m_temp_offset);
	INFO("m_temp_scale %d",m_temp_scale);
	INFO("m_vendor = %s", m_vendor.c_str());
	INFO("m_chip_name = %s", m_chip_name.c_str());
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);
	INFO("temperature_sensor_hal is created!\n");
}

temperature_sensor_hal::~temperature_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

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

bool temperature_sensor_hal::enable(void)
{
	m_fired_time = INITIAL_TIME;
	INFO("Temperature sensor real starting");
	return true;
}

bool temperature_sensor_hal::disable(void)
{
	INFO("Temperature sensor real stopping");
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
	INFO("m_temperature %f",m_temperature);
	INFO("m_temp_offset %f",m_temp_offset);
	INFO("raw_temp_count %d",raw_temp_count);
	INFO("m_temp_scale %d",m_temp_scale);
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
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time ;
	data.value_count = 1;
	data.values[0] = (float) m_temperature;

	return 0;
}


bool temperature_sensor_hal::get_properties(sensor_properties_s &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = -45;
	properties.max_range = 130;
	properties.min_interval = 1;
	properties.resolution = 1;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;

	return true;
}

extern "C" void *create(void)
{
	temperature_sensor_hal *inst;

	try {
		inst = new temperature_sensor_hal();
	} catch (int err) {
		ERR("temperature_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (temperature_sensor_hal*)inst;
}
