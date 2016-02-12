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

cmutex physical_sensor::m_mutex;

physical_sensor::physical_sensor()
: m_sensor_device(NULL)
{
}

physical_sensor::~physical_sensor()
{
}

void physical_sensor::set_sensor_handle(sensor_handle_t handle)
{
	m_handle.id = handle.id;
	m_handle.name = handle.name;
	m_handle.type = handle.type;
	m_handle.event_type = handle.event_type;
	m_handle.model_name = handle.model_name;
	m_handle.vendor = handle.vendor;
	m_handle.min_range = handle.min_range;
	m_handle.max_range = handle.max_range;
	m_handle.resolution = handle.resolution;
	m_handle.min_interval = handle.min_interval;
	m_handle.max_batch_count = handle.max_batch_count;
	m_handle.wakeup_supported = handle.wakeup_supported;
}

void physical_sensor::set_sensor_device(sensor_device *device)
{
	m_sensor_device = device;
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
	if (!m_handle.name)
		return UNKNOWN_NAME;

	return m_handle.name;
}

uint32_t physical_sensor::get_hal_id(void)
{
	return m_handle.id;
}

int physical_sensor::get_poll_fd()
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return -1;

	return m_sensor_device->get_poll_fd();
}

bool physical_sensor::read_fd(std::vector<uint32_t> &ids)
{
	AUTOLOCK(m_mutex);
	int size;
	uint32_t *_ids;

	if (!m_sensor_device)
		return false;

	size = m_sensor_device->read_fd(&_ids);

	for (int i = 0; i < size; ++i)
		ids.push_back(_ids[i]);

	return true;
}

int physical_sensor::get_data(sensor_data_t **data, int *length)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return -1;

	int remains = 0;
	remains = m_sensor_device->get_data(m_handle.id, data, length);

	if (*length < 0) {
		ERR("Failed to get sensor event");
		return -1;
	}

	return remains;
}

bool physical_sensor::flush(void)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->flush(m_handle.id);
}

bool physical_sensor::set_interval(unsigned long interval)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	INFO("Polling interval is set to %dms", interval);

	return m_sensor_device->set_interval(m_handle.id, interval);
}

bool physical_sensor::set_batch_latency(unsigned long latency)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	INFO("Polling interval is set to %dms", latency);

	return m_sensor_device->set_batch_latency(m_handle.id, latency);
}

int physical_sensor::set_attribute(int32_t attribute, int32_t value)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->set_attribute(m_handle.id, attribute, value);
}

int physical_sensor::set_attribute(char *attribute, char *value, int value_len)
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->set_attribute_str(m_handle.id, attribute, value, value_len);
}

bool physical_sensor::set_wakeup(int wakeup)
{
	return false;
}

bool physical_sensor::on_start()
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->enable(m_handle.id);
}

bool physical_sensor::on_stop()
{
	AUTOLOCK(m_mutex);

	if (!m_sensor_device)
		return false;

	return m_sensor_device->disable(m_handle.id);
}

bool physical_sensor::get_sensor_info(sensor_info &info)
{
	info.set_type(get_type());
	info.set_id(get_id());
	info.set_privilege(SENSOR_PRIVILEGE_PUBLIC); // FIXME
	info.set_name(m_handle.model_name);
	info.set_vendor(m_handle.vendor);
	info.set_min_range(m_handle.min_range);
	info.set_max_range(m_handle.max_range);
	info.set_resolution(m_handle.resolution);
	info.set_min_interval(m_handle.min_interval);
	info.set_fifo_count(0); // FIXME
	info.set_max_batch_count(m_handle.max_batch_count);
	info.set_supported_event(get_event_type());
	info.set_wakeup_supported(m_handle.wakeup_supported);

	return true;
}

