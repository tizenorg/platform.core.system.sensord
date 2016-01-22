/*
 * libsensord-share
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

#include <physical_sensor.h>
#include <sensor_event_queue.h>

#define UNKNOWN_NAME "UNKNOWN_SENSOR"

physical_sensor::physical_sensor()
: m_sensor_device(NULL)
{

}

physical_sensor::~physical_sensor()
{

}

sensor_type_t physical_sensor::get_type(void)
{
	return static_cast<sensor_type_t>(m_handle.type);
}

unsigned int physical_sensor::get_event_type(void)
{
	return m_handle.event_type;
}

const char* physical_sensor::get_name(void)
{
	if (m_handle.name.empty())
		return UNKNOWN_NAME;

	return m_handle.name.c_str();
}

void physical_sensor::set_sensor_handle(sensor_handle_t handle)
{
	m_handle.id = handle.id;
	m_handle.name = handle.name;
	m_handle.type = handle.type;
	m_handle.event_type = handle.event_type;
	m_handle.properties = handle.properties;
}

void physical_sensor::set_sensor_device(sensor_device *device)
{
	m_sensor_device = device;
}

int physical_sensor::get_poll_fd()
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return -1;

	return m_sensor_device->get_poll_fd();
}

bool physical_sensor::on_start()
{
	AUTOLOCK(m_mutex);

	return m_sensor_device->enable(m_handle.id);
}

bool physical_sensor::on_stop()
{
	AUTOLOCK(m_mutex);

	return m_sensor_device->disable(m_handle.id);
}

long physical_sensor::set_command(unsigned int cmd, long value)
{
	AUTOLOCK(m_mutex);

	return m_sensor_device->set_command(m_handle.id, std::to_string(cmd), std::to_string(value));
}

bool physical_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_device->set_interval(m_handle.id, interval);
}

bool physical_sensor::set_batch(unsigned long latency)
{
	AUTOLOCK(m_mutex);

	INFO("Polling interval is set to %dms", latency);

	return m_sensor_device->set_batch_latency(m_handle.id, latency);
}

bool physical_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool physical_sensor::is_data_ready(void)
{
	AUTOLOCK(m_mutex);

	return m_sensor_device->is_data_ready();
}

int physical_sensor::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device->get_sensor_data(m_handle.id, data)) {
		ERR("Failed to get sensor data");
		return -1;
	}

	return 0;
}

int physical_sensor::get_sensor_event(sensor_event_t **event)
{
	AUTOLOCK(m_mutex);

	int event_length = -1;
	event_length = m_sensor_device->get_sensor_event(m_handle.id, event);

	if (event_length < 0) {
		ERR("Failed to get sensor event");
		return -1;
	}

	return event_length;
}

bool physical_sensor::get_properties(sensor_properties_s &properties)
{
	return m_sensor_device->get_properties(m_handle.id, properties);
}

