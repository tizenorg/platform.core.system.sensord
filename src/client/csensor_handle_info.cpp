/*
 * libsensord
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

#include <client_common.h>
#include <csensor_handle_info.h>
#include <limits>

using std::pair;

unsigned long long csensor_handle_info::m_event_id = 0;

csensor_handle_info::csensor_handle_info()
: m_handle(0)
, m_sensor_id(UNKNOWN_SENSOR)
, m_sensor_state(SENSOR_STATE_UNKNOWN)
, m_sensor_option(SENSOR_OPTION_DEFAULT)
, m_sensor_wakeup(SENSOR_WAKEUP_OFF)
, m_bad_accuracy(false)
, m_accuracy(-1)
, m_accuracy_cb(NULL)
, m_accuracy_user_data(NULL)
{

}

csensor_handle_info::~csensor_handle_info()
{
	clear_all_events();
}

creg_event_info* csensor_handle_info::get_reg_event_info(unsigned int event_type)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		DBG("Event %s[0x%x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return NULL;
	}

	return &(it_event->second);
}

void csensor_handle_info::get_reg_event_types(event_type_vector &event_types)
{
	auto it_event = m_reg_event_infos.begin();

	while (it_event != m_reg_event_infos.end()) {
		event_types.push_back(it_event->first);
		++it_event;
	}
}

bool csensor_handle_info::add_reg_event_info(unsigned int event_type, unsigned int interval, unsigned int latency, int cb_type, void *cb, void *user_data)
{
	creg_event_info event_info;

	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event != m_reg_event_infos.end()) {
		ERR("Event %s[0x%x] is already registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	event_info.m_id = renew_event_id();
	event_info.m_handle = m_handle;
	event_info.type = event_type;
	event_info.m_interval = interval;
	event_info.m_latency = latency;
	event_info.m_cb_type = cb_type;
	event_info.m_cb = cb;
	event_info.m_user_data = user_data;

	m_reg_event_infos.insert(pair<unsigned int,creg_event_info> (event_type, event_info));

	return true;
}

bool csensor_handle_info::delete_reg_event_info(unsigned int event_type)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		ERR("Event %s[0x%x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	m_reg_event_infos.erase(it_event);

	return true;
}

void csensor_handle_info::clear_all_events(void)
{
	m_reg_event_infos.clear();
}


unsigned long long csensor_handle_info::renew_event_id(void)
{
	return m_event_id++;
}

bool csensor_handle_info::change_reg_event_batch(unsigned int event_type, unsigned int interval, unsigned int latency)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		ERR("Event %s[0x%x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	it_event->second.m_id = renew_event_id();
	it_event->second.m_interval = interval;
	it_event->second.m_latency = latency;

	return true;
}

bool csensor_handle_info::change_reg_event_maincontext(unsigned int event_type, GMainContext *maincontext)
{
	auto it_event = m_reg_event_infos.find(event_type);

	if (it_event == m_reg_event_infos.end()) {
		ERR("Event %s[0x%x] is not registered for client %s", get_event_name(event_type), event_type, get_client_name());
		return false;
	}

	it_event->second.m_maincontext = maincontext;

	return true;
}

void csensor_handle_info::get_batch(unsigned int &interval, unsigned int &latency)
{
	if (m_reg_event_infos.empty()) {
		DBG("No events are registered for client %s", get_client_name());
		interval = POLL_MAX_HZ_MS;
		latency = 0;
		return;
	}

	unsigned int min_interval = POLL_MAX_HZ_MS;
	unsigned int min_latency = std::numeric_limits<unsigned int>::max();

	unsigned int _interval;
	unsigned int _latency;

	auto it_event = m_reg_event_infos.begin();

	while (it_event != m_reg_event_infos.end()) {
		_interval = it_event->second.m_interval;
		_latency = it_event->second.m_latency;

		min_interval = (_interval < min_interval) ? _interval : min_interval;
		min_latency = (_latency < min_latency) ? _latency : min_latency;
		++it_event;
	}

	interval = min_interval;
	latency = min_latency;
}

unsigned int csensor_handle_info::get_reg_event_count(void)
{
	return m_reg_event_infos.size();
}

