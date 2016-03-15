/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <sensor_common.h>
#include <sensor_types.h>
#include <sensor_usage.h>
#include <sensor_log.h>

sensor_usage::sensor_usage()
: m_interval(POLL_MAX_HZ_MS)
, m_latency(0)
, m_option(SENSOR_OPTION_DEFAULT)
, m_start(false)
{

}
sensor_usage::~sensor_usage()
{
	m_reg_events.clear();
}

bool sensor_usage::register_event(unsigned int event_type)
{
	auto it_event = find(m_reg_events.begin(), m_reg_events.end(), event_type);

	if (it_event != m_reg_events.end()) {
		_E("Event[0x%x] is already registered", event_type);
		return false;
	}

	m_reg_events.push_back(event_type);
	return true;
}

bool sensor_usage::unregister_event(unsigned int event_type)
{
	auto it_event = find(m_reg_events.begin(), m_reg_events.end(), event_type);

	if (it_event == m_reg_events.end()) {
		_E("Event[0x%x] is not found",event_type);
		return false;
	}

	m_reg_events.erase(it_event);

	return true;
}

bool sensor_usage::is_event_registered(unsigned int event_type)
{
	auto it_event = find (m_reg_events.begin(), m_reg_events.end(), event_type);

	if (it_event == m_reg_events.end()){
		_D("Event[0x%x] is not registered",event_type);
		return false;
	}

	return true;
}
