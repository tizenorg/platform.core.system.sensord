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

#include <geo_sensor.h>
#include <sensor_plugin_loader.h>

#define SENSOR_NAME "GEOMAGNETIC_SENSOR"

geo_sensor::geo_sensor()
: m_sensor_hal(NULL)
{
	m_name = string(SENSOR_NAME);

	register_supported_event(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);
	register_supported_event(GEOMAGNETIC_EVENT_CALIBRATION_NEEDED);

	physical_sensor::set_poller(geo_sensor::working, this);
}

geo_sensor::~geo_sensor()
{
	INFO("geo_sensor is destroyed!\n");
}

bool geo_sensor::init()
{
	m_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(GEOMAGNETIC_SENSOR);

	if (!m_sensor_hal) {
		ERR("cannot load sensor_hal[%s]", sensor_base::get_name());
		return false;
	}

	sensor_properties_t properties;

	if (m_sensor_hal->get_properties(properties) == false) {
		ERR("sensor->get_properties() is failed!\n");
		return false;
	}

	INFO("%s is created!\n", sensor_base::get_name());

	return true;
}

sensor_type_t geo_sensor::get_type(void)
{
	return GEOMAGNETIC_SENSOR;
}

bool geo_sensor::working(void *inst)
{
	geo_sensor *sensor = (geo_sensor*)inst;
	return sensor->process_event();;
}

bool geo_sensor::process_event(void)
{
	sensor_event_t event;

	if (!m_sensor_hal->is_data_ready(true))
		return true;

	m_sensor_hal->get_sensor_data(event.data);

	AUTOLOCK(m_client_info_mutex);
	AUTOLOCK(m_mutex);

	if (get_client_cnt(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME)) {
		event.event_type = GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME;

		push(event);
	}

	return true;
}

bool geo_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->enable()) {
		ERR("m_sensor_hal start fail\n");
		return false;
	}

	return start_poll();
}

bool geo_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_hal->disable()) {
		ERR("m_sensor_hal stop fail\n");
		return false;
	}

	return stop_poll();
}

long geo_sensor::set_command(const unsigned int cmd, long value)
{
	if (m_sensor_hal->set_command(cmd, value) < 0) {
		ERR("m_sensor_hal set_cmd fail\n");
		return -1;
	}

	return 0;
}

bool geo_sensor::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	return m_sensor_hal->get_properties(properties);
}

int geo_sensor::get_sensor_data(const unsigned int type, sensor_data_t &data)
{
	int state;

	if (type != GEOMAGNETIC_BASE_DATA_SET)
		return -1;

	state = m_sensor_hal->get_sensor_data(data);

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

extern "C" void *create(void)
{
	geo_sensor *inst;

	try {
		inst = new geo_sensor();
	} catch (int err) {
		ERR("geo_sensor class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (geo_sensor*)inst;;
}
