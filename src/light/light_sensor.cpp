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

#include <common.h>
#include <sf_common.h>

#include <light_sensor.h>
#include <sensor_plugin_loader.h>
#include <algorithm>

using std::bind1st;
using std::mem_fun;

#define SENSOR_NAME "LIGHT_SENSOR"


const int light_sensor::m_light_level[] = {0, 1, 165, 288, 497, 869, 1532, 2692, 4692, 8280, 21428, 65535, 137852};

light_sensor::light_sensor()
: m_sensor_hal(NULL)
, m_level(-1)
{
	m_name = string(SENSOR_NAME);

	vector<unsigned int> supported_events = {
		LIGHT_EVENT_CHANGE_LEVEL,
		LIGHT_EVENT_LEVEL_DATA_REPORT_ON_TIME,
		LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME,
	};

	for_each(supported_events.begin(), supported_events.end(),
		bind1st(mem_fun(&sensor_base::register_supported_event), this));

	physical_sensor::set_poller(light_sensor::working, this);
}

light_sensor::~light_sensor()
{
	INFO("light_sensor is destroyed!");
}

bool light_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(LIGHT_SENSOR);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	INFO("%s is created!", sensor_base::get_name());

	return true;
}

sensor_type_t light_sensor::get_type(void)
{
	return LIGHT_SENSOR;
}

bool light_sensor::working(void *inst)
{
	light_sensor *sensor = (light_sensor*)inst;
	return sensor->process_event();
}

bool light_sensor::process_event(void)
{
	sensor_event_t event;
	int level;

	if (!m_sensor_hal->is_data_ready(true))
		return true;

	m_sensor_hal->get_sensor_data(event.data);

	level = (int) adc_to_light_level((int)event.data.values[0]);

	AUTOLOCK(m_client_info_mutex);

	if (get_client_cnt(LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME)) {
		event.event_type = LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME;
		push(event);
	}

	if (get_client_cnt(LIGHT_EVENT_LEVEL_DATA_REPORT_ON_TIME)) {
		event.event_type = LIGHT_EVENT_LEVEL_DATA_REPORT_ON_TIME;
		raw_to_level(event.data);
		push(event);
	}


	if (m_level != level) {
		m_level = level;

		if (get_client_cnt(LIGHT_EVENT_CHANGE_LEVEL)) {
			event.event_type = LIGHT_EVENT_CHANGE_LEVEL;
			raw_to_level(event.data);
			push(event);
		}
	}

	return true;
}



int light_sensor::adc_to_light_level(int adc)
{

	int level_cnt = sizeof(m_light_level)/sizeof(m_light_level[0]) - 1;

	for (int i = 0; i < level_cnt; ++i) {
		if (adc >= m_light_level[i] && adc < m_light_level[i + 1]) {
			return i;
		}
	}

	return -1;
}

bool light_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail\n");
		return false;
	}

	return start_poll();
}

bool light_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail\n");
		return false;
	}

	return stop_poll();
}

bool light_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	m_sensor_hal->get_properties(properties);

	if (type == LIGHT_LUX_DATA_SET)
		return 0;

	if (type == LIGHT_BASE_DATA_SET) {
		properties.sensor_unit_idx = SENSOR_UNIT_LEVEL;
		properties.sensor_min_range = 0;
		properties.sensor_max_range = sizeof(m_light_level)/sizeof(m_light_level[0]) - 1;
		properties.sensor_resolution = 1;
		return 0;
	}

	return -1;
}

int light_sensor::get_sensor_data(const unsigned int type, sensor_data_t &data)
{
	int ret;

	ret = m_sensor_hal->get_sensor_data(data);

	if (ret < 0)
		return -1;

	if (type == LIGHT_LUX_DATA_SET)
		return 0;

	if (type == LIGHT_BASE_DATA_SET) {
		raw_to_level(data);
		return 0;
	}

	return -1;
}

bool light_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_hal->set_interval(interval);
}

void light_sensor::raw_to_level(sensor_data_t &data)
{
	data.data_unit_idx = SENSOR_UNIT_LEVEL;
	data.values[0] = (int) adc_to_light_level((int)data.values[0]);
	data.values_num = 1;
}

extern "C" void *create(void)
{
	light_sensor *inst;

	try {
		inst = new light_sensor();
	} catch (int err) {
		ERR("light_sensor class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (light_sensor*)inst;;
}
