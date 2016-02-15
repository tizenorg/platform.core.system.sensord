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

#include <command_common.h>
#include <client_info_manager.h>
#include <sensor_logs.h>
#include <csocket.h>

using std::pair;
using std::string;

client_info_manager::client_info_manager()
{
}
client_info_manager::~client_info_manager()
{
	m_clients.clear();
}

client_info_manager& client_info_manager::get_instance()
{
	static client_info_manager inst;
	return inst;
}

bool client_info_manager::get_registered_events(int client_id, sensor_id_t sensor_id, event_type_vector &event_vec)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.get_registered_events(sensor_id, event_vec))
		return false;

	return true;
}


bool client_info_manager::register_event(int client_id, sensor_id_t sensor_id, unsigned int event_type)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.register_event(sensor_id, event_type))
		return false;

	return true;
}

bool client_info_manager::unregister_event(int client_id, sensor_id_t sensor_id, unsigned int event_type)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.unregister_event(sensor_id, event_type))
		return false;

	return true;
}

bool client_info_manager::set_batch(int client_id, sensor_id_t sensor_id, unsigned int interval, unsigned latency)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	return it_record->second.set_batch(sensor_id, interval, latency);
}

bool client_info_manager::get_batch(int client_id, sensor_id_t sensor_id, unsigned int &interval, unsigned int &latency)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	return it_record->second.get_batch(sensor_id, interval, latency);
}

bool client_info_manager::set_option(int client_id, sensor_id_t sensor_id, int option)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.set_option(sensor_id, option))
		return false;

	return true;
}

bool client_info_manager::set_wakeup(int client_id, sensor_id_t sensor_id, int wakeup)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.set_wakeup(sensor_id, wakeup))
		return false;

	return true;
}

bool client_info_manager::set_start(int client_id, sensor_id_t sensor_id, bool start)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.set_start(sensor_id, start))
		return false;

	return true;

}

bool client_info_manager::is_started(int client_id, sensor_id_t sensor_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	return it_record->second.is_started(sensor_id);
}

int client_info_manager::create_client_record(void)
{
	AUTOLOCK(m_mutex);

	int client_id = 0;

	client_sensor_record client_record;

	while (m_clients.count(client_id) > 0)
		client_id++;

	if (client_id == MAX_HANDLE) {
		_E("Sensor records of clients are full");
		return MAX_HANDLE_REACHED;
	}

	client_record.set_client_id(client_id);

	m_clients.insert(pair<int,client_sensor_record> (client_id, client_record));

	return client_id;
}


bool client_info_manager::remove_client_record(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	m_clients.erase(it_record);

	_I("Client record for client[%d] is removed from client info manager", client_id);

	return true;
}


bool client_info_manager::has_client_record(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	return (it_record != m_clients.end());
}


void client_info_manager::set_client_info(int client_id, pid_t pid, const string &name)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return;
	}

	it_record->second.set_client_info(pid, name);

	return;
}

const char* client_info_manager::get_client_info(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return NULL;
	}

	return it_record->second.get_client_info();
}

bool client_info_manager::set_permission(int client_id, int permission)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	it_record->second.set_permission(permission);
	return true;
}

bool client_info_manager::get_permission(int client_id, int &permission)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	permission = it_record->second.get_permission();
	return true;
}

bool client_info_manager::create_sensor_record(int client_id, sensor_id_t sensor_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client record[%d] is not registered", client_id);
		return false;
	}

	it_record->second.add_sensor_usage(sensor_id);

	return true;
}

bool client_info_manager::remove_sensor_record(int client_id, sensor_id_t sensor_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.remove_sensor_usage(sensor_id))
		return false;

	if(!it_record->second.has_sensor_usage())
		remove_client_record(client_id);

	return true;
}


bool client_info_manager::has_sensor_record(int client_id, sensor_id_t sensor_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.has_sensor_usage(sensor_id))
		return false;

	return true;
}

bool client_info_manager::has_sensor_record(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	if(!it_record->second.has_sensor_usage())
		return false;

	return true;
}

bool client_info_manager::get_listener_ids(sensor_id_t sensor_id, unsigned int event_type, client_id_vec &id_vec)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.begin();

	while (it_record != m_clients.end()) {
		if(it_record->second.is_listening_event(sensor_id, event_type))
			id_vec.push_back(it_record->first);

		++it_record;
	}

	return true;
}

bool client_info_manager::get_event_socket(int client_id, csocket &socket)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	it_record->second.get_event_socket(socket);

	return true;
}

bool client_info_manager::set_event_socket(int client_id, const csocket &socket)
{

	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	it_record->second.set_event_socket(socket);

	return true;
}
