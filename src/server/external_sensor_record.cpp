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
#include <external_sensor_record.h>

using std::string;

external_sensor_record::external_sensor_record()
: m_client_id(0)
, m_pid(-1)
{
}

external_sensor_record::~external_sensor_record()
{
	close_command_socket();
}

bool external_sensor_record::add_usage(sensor_id_t sensor)
{
	if (!m_usages.insert(sensor).second) {
		_E("Sensor[%#x] is already registered", sensor);
		return false;
	}

	return true;
}

bool external_sensor_record::remove_usage(sensor_id_t sensor)
{
	if (!m_usages.erase(sensor)) {
		_E("Sensor[%#x] is not found", sensor);
		return false;
	}

	return true;
}

bool external_sensor_record::has_usage(void)
{
	return !m_usages.empty();
}

bool external_sensor_record::has_usage(sensor_id_t sensor)
{
	auto it_usage = m_usages.find(sensor);

	return (it_usage != m_usages.end());
}

void external_sensor_record::set_client_id(int client_id)
{
	m_client_id = client_id;
}

void external_sensor_record::set_client_info(pid_t pid, const string &name)
{
	char client_info[NAME_MAX + 32];
	m_pid = pid;

	snprintf(client_info, sizeof(client_info), "%s[pid=%d, id=%d]", name.c_str(), m_pid, m_client_id);
	m_client_info.assign(client_info);
}

const char* external_sensor_record::get_client_info(void)
{
	return m_client_info.c_str();
}

void external_sensor_record::set_command_socket(const csocket &socket)
{
	m_command_socket = socket;
}

void external_sensor_record::get_command_socket(csocket &socket)
{
	socket = m_command_socket;
}

bool external_sensor_record::close_command_socket(void)
{
	return m_command_socket.close();
}

