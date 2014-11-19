/*
 * geo_sensor_hal
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
#include <geo_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <iio_common.h>

using std::ifstream;
using config::csensor_config;

#define SENSOR_TYPE_MAGNETIC	"MAGNETIC"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_MIN_RANGE		"MIN_RANGE"
#define ELEMENT_MAX_RANGE		"MAX_RANGE"
#define ATTR_VALUE				"value"

#define INITIAL_TIME			-1
#define GAUSS_TO_UTESLA(val)	((val) * 100.0f)

geo_sensor_hal::geo_sensor_hal()
: m_x(0)
, m_y(0)
, m_z(0)
, m_hdst(0)
, m_node_handle(-1)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(INITIAL_TIME)
{
	const string sensorhub_interval_node_name = "mag_poll_delay";
	csensor_config &config = csensor_config::get_instance();

	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_MAGNETIC, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;

	}

	query.input_method = input_method;
	query.sensorhub_controlled = m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_MAGNETIC;
	query.input_event_key = "geomagnetic_sensor";
	query.iio_enable_node_name = "geomagnetic_enable";
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_path_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	show_node_path_info(info);

	m_data_node = info.data_node_path;
	m_enable_node = info.enable_node_path;
	m_interval_node = info.interval_node_path;

	if (input_method == IIO_METHOD) {
		m_geo_dir = info.base_dir;
		m_x_node = m_geo_dir + string(X_RAW_VAL_NODE);
		m_y_node = m_geo_dir + string(Y_RAW_VAL_NODE);
		m_z_node = m_geo_dir + string(Z_RAW_VAL_NODE);
		m_x_scale_node = m_geo_dir + string(X_SCALE_NODE);
		m_y_scale_node = m_geo_dir + string(Y_SCALE_NODE);
		m_z_scale_node = m_geo_dir + string(Z_SCALE_NODE);
		INFO("Raw data node X: %s", m_x_node.c_str());
		INFO("Raw data node Y: %s", m_y_node.c_str());
		INFO("Raw data node Z: %s", m_z_node.c_str());
		INFO("scale node X: %s", m_x_scale_node.c_str());
		INFO("scale node Y: %s", m_y_scale_node.c_str());
		INFO("scale node Z: %s", m_z_scale_node.c_str());
	}

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_MAGNETIC, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	init_resources();

	INFO("m_chip_name = %s\n",m_chip_name.c_str());
	INFO("m_raw_data_unit = %f\n", m_raw_data_unit);
	INFO("geo_sensor_hal is created!\n");

}

geo_sensor_hal::~geo_sensor_hal()
{
	if (m_node_handle > 0)
		close(m_node_handle);
	m_node_handle = -1;

	INFO("geo_sensor is destroyed!\n");
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
	m_fired_time = INITIAL_TIME;
	INFO("Geo sensor real starting");
	return true;
}

bool geo_sensor_hal::disable(void)
{
	INFO("Geo sensor real stopping");
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
	INFO("x = %d, y = %d, z = %d, time = %lluus", raw_values[0], raw_values[1], raw_values[2], m_fired_time);
	INFO("x = %f, y = %f, z = %f, time = %lluus", m_x, m_y, m_z, m_fired_time);

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
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time;
	data.value_count = 3;
	data.values[0] = (float)m_x;
	data.values[1] = (float)m_y;
	data.values[2] = (float)m_z;
	return 0;
}

bool geo_sensor_hal::get_properties(sensor_properties_t &properties)
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

bool geo_sensor_hal::init_resources(void)
{
	ifstream temp_handle;

	if (!read_node_value<double>(m_x_scale_node, m_x_scale)) {
		ERR("Failed to read x scale node");
		return false;
	}
	if (!read_node_value<double>(m_y_scale_node, m_y_scale)) {
		ERR("Failed to read y scale node");
		return false;
	}
	if (!read_node_value<double>(m_z_scale_node, m_z_scale)) {
		ERR("Failed to read y scale node");
		return false;
	}
	INFO("Scale Values: %f, %f, %f", m_x_scale, m_y_scale, m_z_scale);
	return true;
}

extern "C" void *create(void)
{
	geo_sensor_hal *inst;

	try {
		inst = new geo_sensor_hal();
	} catch (int err) {
		ERR("geo_sensor_hal class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (geo_sensor_hal*)inst;
}
