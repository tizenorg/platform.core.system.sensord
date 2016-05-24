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
 */

#include <external_sensor_service.h>
#include <external_sensor.h>
#include <external_client_manager.h>
#include <command_queue.h>
#include <thread>

using std::thread;
using std::pair;
using std::string;
using std::shared_ptr;

external_sensor_service::external_sensor_service()
{
}

external_sensor_service::~external_sensor_service()
{
}

external_sensor_service& external_sensor_service::get_instance(void)
{
	static external_sensor_service instance;
	return instance;
}

bool external_sensor_service::run(void)
{
	thread dispatcher(&external_sensor_service::dispatch_command, this);
	dispatcher.detach();

	return true;
}

external_client_manager& external_sensor_service::get_client_manager(void)
{
	return external_client_manager::get_instance();
}

void external_sensor_service::accept_command_channel(csocket client_socket)
{
	thread th = thread([&, client_socket]() mutable {
		int client_id;
		channel_ready_t command_channel_ready;
		external_client_manager& client_manager = get_client_manager();

		client_socket.set_connection_mode();

		if (client_socket.recv(&client_id, sizeof(client_id)) <= 0) {
			_E("Failed to receive client id on socket fd[%d]", client_socket.get_socket_fd());
			return;
		}

		client_socket.set_transfer_mode();

		if (!client_manager.set_command_socket(client_id, client_socket)) {
			_E("Failed to store event socket[%d] for %s", client_socket.get_socket_fd(),
				client_manager.get_client_info(client_id));
			return;
		}

		command_channel_ready.magic = CHANNEL_MAGIC_NUM;
		command_channel_ready.client_id = client_id;

		_I("Command channel is accepted for %s on socket[%d]",
			client_manager.get_client_info(client_id), client_socket.get_socket_fd());

		if (client_socket.send(&command_channel_ready, sizeof(command_channel_ready)) <= 0) {
			_E("Failed to send command channel_ready packet to %s on socket fd[%d]",
				client_manager.get_client_info(client_id), client_socket.get_socket_fd());
			return;
		}
	});

	th.detach();
}

void external_sensor_service::dispatch_command(void)
{
	while (true) {
		shared_ptr<external_command_t> command = command_queue::get_instance().pop();
		csocket client_sock;
		sensor_id_t sensor_id = command->header.sensor_id;
		bool ret;

		ret = external_client_manager::get_instance().get_listener_socket(sensor_id, client_sock);

		if (!ret) {
			_E("Failed to get listener socket for sensor[%d]", sensor_id);
			continue;
		}

		if (client_sock.send(&(command->header), sizeof(command->header)) <= 0) {
			_E("Failed to send command header to the client of sensor[%d]", sensor_id);
			continue;
		}

		if (client_sock.send(command->command.data(), command->header.command_len) <= 0) {
			_E("Failed to send command header to the client of sensor[%d]", sensor_id);
			continue;
		}
	}
}

bool external_sensor_service::register_sensor(external_sensor *sensor)
{
	if (!m_external_sensors.insert(pair<string, external_sensor*>(sensor->get_key(), sensor)).second) {
		_E("Failed to register sensor, key: %s", sensor->get_key().c_str());
		return false;
	}

	return true;
}

bool external_sensor_service::unregister_sensor(external_sensor *sensor)
{
	if (!m_external_sensors.erase(sensor->get_key())) {
		_E("Failed to unregister sensor, key: %s", sensor->get_key().c_str());
		return false;
	}

	return true;
}

external_sensor* external_sensor_service::get_sensor(const string& key)
{
	auto it_sensor = m_external_sensors.find(key);

	if (it_sensor == m_external_sensors.end()) {
		_E("Sensor(key:%s) is not found", key.c_str());
		return NULL;
	}

	return it_sensor->second;
}
