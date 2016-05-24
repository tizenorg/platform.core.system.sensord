/*
 * sensord
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

#include <command_common.h>
#include <external_client_manager.h>
#include <sensor_common.h>

using std::shared_ptr;
using std::make_shared;
using std::pair;
using std::string;

external_client_manager::external_client_manager()
{
}
external_client_manager::~external_client_manager()
{
}

external_client_manager& external_client_manager::get_instance(void)
{
	static external_client_manager instance;
	return instance;
}

int external_client_manager::create_client_record(void)
{
	AUTOLOCK(m_mutex);

	int client_id = 0;

	shared_ptr<external_sensor_record> client_record = make_shared<external_sensor_record>();

	while (m_clients.count(client_id) > 0)
		client_id++;

	if (client_id == MAX_HANDLE) {
		_E("Sensor records of clients are full");
		return MAX_HANDLE_REACHED;
	}

	client_record->set_client_id(client_id);

	m_clients.insert(pair<int, shared_ptr<external_sensor_record>>(client_id, client_record));

	return client_id;
}

bool external_client_manager::remove_client_record(int client_id)
{
	AUTOLOCK(m_mutex);

	if (!m_clients.erase(client_id)) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	_I("Client record for client[%d] is removed from external client manager", client_id);
	return true;
}

bool external_client_manager::has_client_record(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	return (it_record != m_clients.end());
}

void external_client_manager::set_client_info(int client_id, pid_t pid, const string &name)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return;
	}

	it_record->second->set_client_info(pid, name);

	return;
}

const char* external_client_manager::get_client_info(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return NULL;
	}

	return it_record->second->get_client_info();
}

bool external_client_manager::create_sensor_record(int client_id, sensor_id_t sensor)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client record[%d] is not registered", client_id);
		return false;
	}

	return it_record->second->add_usage(sensor);
}

bool external_client_manager::remove_sensor_record(int client_id, sensor_id_t sensor)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	if (!it_record->second->remove_usage(sensor))
		return false;

	if (!it_record->second->has_usage())
		remove_client_record(client_id);

	return true;
}

bool external_client_manager::has_sensor_record(int client_id, sensor_id_t sensor)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	return it_record->second->has_usage(sensor);
}

bool external_client_manager::has_sensor_record(int client_id)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_D("Client[%d] is not found", client_id);
		return false;
	}

	return it_record->second->has_usage();
}

bool external_client_manager::get_listener_socket(sensor_id_t sensor, csocket &sock)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.begin();

	while (it_record != m_clients.end()) {
		if (it_record->second->has_usage(sensor)) {
			it_record->second->get_command_socket(sock);
			return true;
		}

		++it_record;
	}

	return false;
}

bool external_client_manager::get_command_socket(int client_id, csocket &socket)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	it_record->second->get_command_socket(socket);

	return true;
}

bool external_client_manager::set_command_socket(int client_id, const csocket &socket)
{
	AUTOLOCK(m_mutex);

	auto it_record = m_clients.find(client_id);

	if (it_record == m_clients.end()) {
		_E("Client[%d] is not found", client_id);
		return false;
	}

	it_record->second->set_command_socket(socket);

	return true;
}
