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

#include <sensor_logs.h>
#include <sf_common.h>

#include <geo_sensor.h>
#include <sensor_plugin_loader.h>

using std::string;
using std::vector;

#define SENSOR_NAME "GEOMAGNETIC_SENSOR"

geo_sensor::geo_sensor()
: m_sensor_hal(NULL)
, m_resolution(0.0f)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(GEOMAGNETIC_RAW_DATA_EVENT);
	register_supported_event(GEOMAGNETIC_UNPROCESSED_DATA_EVENT);

	physical_sensor::set_poller(geo_sensor::working, this);
}

geo_sensor::~geo_sensor()
{
	INFO("geo_sensor is destroyed!\n");
}

bool geo_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_GEOMAGNETIC);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	sensor_properties_s properties;

	if (m_sensor_hal->get_properties(properties) == false) {
		ERR("sensor->get_properties() is failed!\n");
		return false;
	}

	m_resolution = properties.resolution;

	INFO("%s is created!\n", sensor_base::get_name());

	return true;
}

void geo_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(GEOMAGNETIC_SENSOR);
}

bool geo_sensor::working(void *inst)
{
	geo_sensor *sensor = (geo_sensor*)inst;
	return sensor->process_event();
}

bool geo_sensor::process_event(void)
{
	sensor_event_t event;

	if (!m_sensor_hal->is_data_ready())
		return true;

	m_sensor_hal->get_sensor_data(event.data);

	AUTOLOCK(m_client_info_mutex);
	AUTOLOCK(m_mutex);

	if (get_client_cnt(GEOMAGNETIC_UNPROCESSED_DATA_EVENT)) {
		event.sensor_id = get_id();
		event.event_type = GEOMAGNETIC_UNPROCESSED_DATA_EVENT;
		push(event);
	}

	if (get_client_cnt(GEOMAGNETIC_RAW_DATA_EVENT)) {
		event.sensor_id = get_id();
		event.event_type = GEOMAGNETIC_RAW_DATA_EVENT;
		raw_to_base(event.data);
		push(event);
	}

	return true;
}

bool geo_sensor::on_start(void)
{
	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail\n");
		return false;
	}

	return start_poll();
}

bool geo_sensor::on_stop(void)
{
	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail\n");
		return false;
	}

	return stop_poll();
}

bool geo_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	return m_sensor_hal->get_properties(properties);
}

int geo_sensor::get_sensor_data(unsigned int type, sensor_data_t &data)
{
	int state;

	if (type != GEOMAGNETIC_RAW_DATA_EVENT)
		return -1;

	state = m_sensor_hal->get_sensor_data(data);
	raw_to_base(data);

	if (state < 0) {
		ERR("m_sensor_hal get struct_data fail\n");
		return -1;
	}

	return 0;
}

bool geo_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_hal->set_interval(interval);
}

void geo_sensor::raw_to_base(sensor_data_t &data)
{
	data.value_count = 3;
	data.values[0] = data.values[0] * m_resolution;
	data.values[1] = data.values[1] * m_resolution;
	data.values[2] = data.values[2] * m_resolution;
}
