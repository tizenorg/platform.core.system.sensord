/*
 * libsensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <csensor_event_listener.h>
#include <client_common.h>
#include <sf_common.h>
#include <sensor_info_manager.h>

#include <thread>
#include <chrono>

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

using std::thread;
using std::pair;

csensor_client_info::csensor_client_info()
: m_client_id(CLIENT_ID_INVALID)
{
}

csensor_client_info::~csensor_client_info()
{
}


csensor_client_info& csensor_client_info::get_instance(void)
{
	static csensor_client_info inst;
	return inst;
}


int csensor_client_info::create_handle(sensor_id_t sensor)
{
	csensor_handle_info handle_info;
	int handle = 0;

	AUTOLOCK(m_handle_info_lock);

	while (m_sensor_handle_infos.count(handle) > 0)
		handle++;

	if (handle == MAX_HANDLE) {
		ERR("Handles of client %s are full", get_client_name());
		return MAX_HANDLE_REACHED;
	}

	handle_info.m_sensor_id = sensor;
	handle_info.m_sensor_state = SENSOR_STATE_STOPPED;
	handle_info.m_sensor_option = SENSOR_OPTION_DEFAULT;
	handle_info.m_handle = handle;
	handle_info.m_accuracy = -1;
	handle_info.m_accuracy_cb = NULL;
	handle_info.m_accuracy_user_data = NULL;

	m_sensor_handle_infos.insert(pair<int,csensor_handle_info> (handle, handle_info));

	return handle;
}

bool csensor_client_info::delete_handle(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	m_sensor_handle_infos.erase(it_handle);
	return true;
}


bool csensor_client_info::is_active()
{
	AUTOLOCK(m_handle_info_lock);

	return !m_sensor_handle_infos.empty();
}

bool csensor_client_info::add_register_event_info(int handle, unsigned int event_type,
		unsigned int interval, unsigned int latency, int cb_type, void *cb, void* user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.add_reg_event_info(event_type, interval, latency, cb_type, cb, user_data))
		return false;

	return true;
}

bool csensor_client_info::delete_register_event_info(int handle, unsigned int event_type)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.delete_reg_event_info(event_type))
		return false;

	return true;
}

bool csensor_client_info::add_accuracy_cb_info(int handle, sensor_accuracy_changed_cb_t cb, void* user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = -1;
	it_handle->second.m_accuracy_cb = cb;
	it_handle->second.m_accuracy_user_data = user_data;

	return true;
}

bool csensor_client_info::delete_accuracy_cb_info(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = -1;
	it_handle->second.m_accuracy_cb = NULL;
	it_handle->second.m_accuracy_user_data = NULL;

	return true;
}

bool csensor_client_info::set_sensor_params_info(int handle, int sensor_state, int sensor_option)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_state = sensor_state;
	it_handle->second.m_sensor_option = sensor_option;

	return true;
}

bool csensor_client_info::get_sensor_params_info(int handle, int &sensor_state, int &sensor_option)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor_state = it_handle->second.m_sensor_state;
	sensor_option = it_handle->second.m_sensor_option;

	return true;
}

bool csensor_client_info::set_sensor_state_info(int handle, int sensor_state)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_state = sensor_state;

	return true;
}

bool csensor_client_info::set_sensor_option_info(int handle, int sensor_option)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_option = sensor_option;

	return true;
}

bool csensor_client_info::set_event_batch_info(int handle, unsigned int event_type, unsigned int interval, unsigned int latency)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	if (!it_handle->second.change_reg_event_batch(event_type, interval, latency))
		return false;

	return true;
}

bool csensor_client_info::set_accuracy_info(int handle, int accuracy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_accuracy = accuracy;

	return true;
}

bool csensor_client_info::set_bad_accuracy_info(int handle, int bad_accuracy)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_bad_accuracy = bad_accuracy;

	return true;
}

bool csensor_client_info::get_event_info(int handle, unsigned int event_type, unsigned int &interval, unsigned int &latency, int &cb_type, void* &cb, void* &user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	const creg_event_info *event_info;

	event_info = it_handle->second.get_reg_event_info(event_type);

	if (!event_info)
		return NULL;


	interval = event_info->m_interval;
	cb_type = event_info->m_cb_type;
	cb = event_info->m_cb;
	user_data = event_info->m_user_data;
	latency = event_info->m_latency;

	return true;
}


void csensor_client_info::get_listening_sensors(sensor_id_vector &sensors)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		sensors.push_back(it_handle->second.m_sensor_id);
		++it_handle;
	}

	sort(sensors.begin(), sensors.end());
	unique(sensors.begin(),sensors.end());
}


bool csensor_client_info::add_command_channel(sensor_id_t sensor, command_channel *cmd_channel)
{
	auto it_channel = m_command_channels.find(sensor);

	if (it_channel != m_command_channels.end()) {
		ERR("%s alreay has command_channel for %s", get_client_name(), get_sensor_name(sensor));
		return false;
	}

	m_command_channels.insert(pair<sensor_id_t, command_channel *> (sensor, cmd_channel));

	return true;

}
bool csensor_client_info::get_command_channel(sensor_id_t sensor, command_channel **cmd_channel)
{
	auto it_channel = m_command_channels.find(sensor);

	if (it_channel == m_command_channels.end()) {
		ERR("%s doesn't have command_channel for %s", get_client_name(), get_sensor_name(sensor));
		return false;
	}

	*cmd_channel = it_channel->second;

	return true;
}


bool csensor_client_info::close_command_channel(void)
{
	auto it_channel = m_command_channels.begin();

	if (it_channel != m_command_channels.end()) {
		delete it_channel->second;
		++it_channel;
	}

	m_command_channels.clear();

	return true;
}

bool csensor_client_info::close_command_channel(sensor_id_t sensor_id)
{
	auto it_channel = m_command_channels.find(sensor_id);

	if (it_channel == m_command_channels.end()) {
		ERR("%s doesn't have command_channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	delete it_channel->second;

	m_command_channels.erase(it_channel);

	return true;
}


bool csensor_client_info::has_client_id(void)
{
	return (m_client_id != CLIENT_ID_INVALID);
}

int csensor_client_info::get_client_id(void)
{
	return m_client_id;
}

void csensor_client_info::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool csensor_client_info::get_active_batch(sensor_id_t sensor, unsigned int &interval, unsigned int &latency)
{
	unsigned int min_interval = POLL_MAX_HZ_MS;
	unsigned int min_latency = std::numeric_limits<unsigned int>::max();

	bool active_sensor_found = false;
	unsigned int _interval;
	unsigned int _latency;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
			(it_handle->second.m_sensor_state == SENSOR_STATE_STARTED)) {
				active_sensor_found = true;
				it_handle->second.get_batch(_interval, _latency);
				min_interval = (_interval < min_interval) ? _interval : min_interval;
				min_latency = (_latency < min_latency) ? _latency : min_latency;
		}

		++it_handle;
	}

	if (!active_sensor_found) {
		DBG("Active sensor[0x%x] is not found for client %s", sensor, get_client_name());
		return false;
	}

	interval = min_interval;
	latency = min_latency;

	return true;
}

unsigned int csensor_client_info::get_active_option(sensor_id_t sensor)
{
	int active_option = SENSOR_OPTION_DEFAULT;
	bool active_sensor_found = false;
	int option;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
			(it_handle->second.m_sensor_state == SENSOR_STATE_STARTED)) {
				active_sensor_found = true;
				option = it_handle->second.m_sensor_option;
				active_option = (option > active_option) ? option : active_option;
		}

		++it_handle;
	}

	if (!active_sensor_found)
		DBG("Active sensor[0x%x] is not found for client %s", sensor, get_client_name());

	return active_option;
}

bool csensor_client_info::get_sensor_id(int handle, sensor_id_t &sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor = it_handle->second.m_sensor_id;

	return true;
}

bool csensor_client_info::get_sensor_state(int handle, int &sensor_state)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor_state = it_handle->second.m_sensor_state;

	return true;
}

bool csensor_client_info::get_sensor_wakeup(int handle, int &sensor_wakeup)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor_wakeup = it_handle->second.m_sensor_wakeup;

	return true;
}

bool csensor_client_info::set_sensor_wakeup(int handle, int sensor_wakeup)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		ERR("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor_wakeup = sensor_wakeup;

	return true;
}

void csensor_client_info::get_active_event_types(sensor_id_t sensor, event_type_vector &active_event_types)
{
	event_type_vector event_types;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
			(it_handle->second.m_sensor_state == SENSOR_STATE_STARTED))
				it_handle->second.get_reg_event_types(event_types);

		++it_handle;
	}

	if (event_types.empty())
		return;

	sort(event_types.begin(), event_types.end());

	unique_copy(event_types.begin(), event_types.end(), back_inserter(active_event_types));

}


void csensor_client_info::get_all_handles(handle_vector &handles)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		handles.push_back(it_handle->first);
		++it_handle;
	}
}

void csensor_client_info::get_sensor_handle_info(sensor_id_t sensor, sensor_handle_info_map &handles_info) {

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if (it_handle->second.m_sensor_id == sensor) {
			handles_info.insert(pair<int,csensor_handle_info> (it_handle->first, it_handle->second));
		}

		++it_handle;
	}
}

void csensor_client_info::get_all_handle_info(sensor_handle_info_map &handles_info) {

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		handles_info.insert(pair<int,csensor_handle_info> (it_handle->first, it_handle->second));
		++it_handle;
	}
}

bool csensor_client_info::is_sensor_registered(sensor_id_t sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if (it_handle->second.m_sensor_id == sensor)
			return true;

		++it_handle;
	}

	return false;
}


bool csensor_client_info::is_sensor_active(sensor_id_t sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		if ((it_handle->second.m_sensor_id == sensor) &&
			(it_handle->second.m_sensor_state == SENSOR_STATE_STARTED))
			return true;

		++it_handle;
	}

	return false;
}

bool csensor_client_info::is_event_active(int handle, unsigned int event_type, unsigned long long event_id)
{
	creg_event_info *event_info;

	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end())
		return false;

	event_info = it_handle->second.get_reg_event_info(event_type);
	if (!event_info)
		return false;

	if (event_info->m_id != event_id)
		return false;

	return true;
}

void csensor_client_info::clear(void)
{
	close_command_channel();
	m_sensor_handle_infos.clear();
	m_command_channels.clear();
	set_client_id(CLIENT_ID_INVALID);
}
