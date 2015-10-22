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

#include <bio_led_red_sensor.h>
#include <sensor_plugin_loader.h>
#include <algorithm>

using std::bind1st;
using std::mem_fun;
using std::string;
using std::vector;

#define SENSOR_NAME "BIO_LED_RED_SENSOR"

bio_led_red_sensor::bio_led_red_sensor()
: m_sensor_hal(NULL)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(BIO_LED_RED_RAW_DATA_EVENT);

	physical_sensor::set_poller(bio_led_red_sensor::working, this);
}

bio_led_red_sensor::~bio_led_red_sensor()
{
	INFO("bio_led_red_sensor is destroyed!");
}

bool bio_led_red_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(SENSOR_HAL_TYPE_BIO_LED_RED);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	INFO("%s is created!", sensor_base::get_name());

	return true;
}

void bio_led_red_sensor::get_types(vector<sensor_type_t> &types)
{
	types.push_back(BIO_LED_RED_SENSOR);
}

bool bio_led_red_sensor::working(void *inst)
{
	bio_led_red_sensor *sensor = (bio_led_red_sensor*)inst;
	return sensor->process_event();
}

bool bio_led_red_sensor::process_event(void)
{
	sensor_event_t event;

	if (!m_sensor_hal->is_data_ready(true))
		return true;

	m_sensor_hal->get_sensor_data(event.data);

	AUTOLOCK(m_client_info_mutex);

	if (get_client_cnt(BIO_LED_RED_RAW_DATA_EVENT)) {
		event.sensor_id = get_id();
		event.event_type = BIO_LED_RED_RAW_DATA_EVENT;
		raw_to_base(event.data);
		push(event);
	}

	return true;
}

bool bio_led_red_sensor::on_start(void)
{
	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail\n");
		return false;
	}

	return start_poll();
}

bool bio_led_red_sensor::on_stop(void)
{
	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail\n");
		return false;
	}

	return stop_poll();
}

bool bio_led_red_sensor::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	return m_sensor_hal->get_properties(properties);
}

int bio_led_red_sensor::get_sensor_data(unsigned int type, sensor_data_t &data)
{
	int ret;

	ret = m_sensor_hal->get_sensor_data(data);

	if (ret < 0)
		return -1;

	return -1;
}

bool bio_led_red_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_hal->set_interval(interval);
}

void bio_led_red_sensor::raw_to_base(sensor_data_t &data)
{

}

extern "C" sensor_module* create(void)
{
	bio_led_red_sensor *sensor;

	try {
		sensor = new(std::nothrow) bio_led_red_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
