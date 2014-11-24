/*
 * light_sensor_hal
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
#include <dirent.h>
#include <linux/input.h>
#include <csensor_config.h>
#include <light_sensor_hal.h>
#include <sys/ioctl.h>
#include <iio_common.h>

using std::ifstream;

#define SENSOR_TYPE_LIGHT		"LIGHT"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"
#define ATTR_VALUE				"value"
#define INITIAL_TIME -1
#define BIAS	1
#define INVALID_VALUE	-1
#define INITIAL_VALUE	-1

light_sensor_hal::light_sensor_hal()
: m_polling_interval(POLL_1HZ_MS)
, m_adc(INVALID_VALUE)
, m_fired_time(INITIAL_TIME)
, m_node_handle(-1)
{
	const string sensorhub_interval_node_name = "light_poll_delay";
	csensor_config &config = csensor_config::get_instance();

	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_LIGHT, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;

	}

	query.input_method = IIO_METHOD;
	query.sensorhub_controlled = m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_LIGHT;
	query.input_event_key = "light_sensor";
	query.iio_enable_node_name = "light_enable";
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
		m_light_dir=info.base_dir;
		m_light_node = m_light_dir + string(ILL_CLEAR_NODE);

		INFO("m_light_node = %s", m_light_node.c_str());
		INFO("m_light_dir = %s", m_light_dir.c_str());
	}

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_LIGHT, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s\n",m_chip_name.c_str());

	if ((m_node_handle = open(m_light_node.c_str(),O_RDWR)) < 0) {
		ERR("Failed to open handle(%d)", m_node_handle);
		throw ENXIO;
	}

	INFO("light_sensor_hal is created!\n");

}

light_sensor_hal::~light_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

	INFO("light_sensor_hal is destroyed!\n");
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
	m_fired_time = INITIAL_TIME;
	INFO("Light sensor real starting");
	return true;
}

bool light_sensor_hal::disable(void)
{
	INFO("Light sensor real stopping");
	return true;
}

bool light_sensor_hal::set_interval(unsigned long val)
{
	return true;
}


bool light_sensor_hal::update_value(bool wait)
{

	unsigned short int adc = INITIAL_VALUE;
	if (!read_node_value<unsigned short int>(m_light_node, adc))
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
	ret = update_value(wait);
	return ret;
}

int light_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time ;
	data.value_count = 1;
	data.values[0] = (float) m_adc;

	return 0;
}


bool light_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = 0;
	properties.max_range = 65536;
	properties.min_interval = 1;
	properties.resolution = 1;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	return true;
}

extern "C" void *create(void)
{
	light_sensor_hal *inst;

	try {
		inst = new light_sensor_hal();
	} catch (int err) {
		ERR("light_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (light_sensor_hal*)inst;
}
