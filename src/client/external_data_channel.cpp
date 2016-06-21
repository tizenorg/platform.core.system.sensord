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
#include <sensor_types.h>
#include <external_data_channel.h>

using std::string;

external_data_channel::external_data_channel()
: m_client_id(CLIENT_ID_INVALID)
, m_sensor_id(UNKNOWN_SENSOR)
{
}

external_data_channel::~external_data_channel()
{
	m_socket.close();
}

bool external_data_channel::command_handler(cpacket *packet, void **return_payload)
{
	packet_header header;
	char *buffer = NULL;

	if (!m_socket.is_valid()) {
		_E("Socket(%d) is not valid for client %s", m_socket.get_socket_fd(), get_client_name());
		return false;
	}

	if (!packet->size()) {
		_E("Packet is not valid for client %s", get_client_name());
		return false;
	}

	if (m_socket.send(packet->packet(), packet->size()) <= 0) {
		m_socket.close();
		_E("Failed to send command in client %s", get_client_name());
		return false;
	}

	if (m_socket.recv(&header, sizeof(header)) <= 0) {
		m_socket.close();
		_E("Failed to receive header for command packet in client %s", get_client_name());
		return false;
	}

	buffer = new(std::nothrow) char[header.size];
	retvm_if(!buffer, false, "Failed to allocate memory");

	if (m_socket.recv(buffer, header.size) <= 0) {
		m_socket.close();
		_E("Failed to receive command packet in client %s", get_client_name());
		delete[] buffer;
		return false;
	}

	*return_payload = buffer;

	return true;
}

bool external_data_channel::create_channel(void)
{
	const int client_type = CLIENT_TYPE_EXTERNAL_SOURCE;

	if (!m_socket.create(SOCK_STREAM)) {
		_E("Failed to create external data channel for client %s", get_client_name());
		return false;
	}

	if (!m_socket.connect(COMMAND_CHANNEL_PATH)) {
		_E("Failed to connect external data channel for client %s, command socket fd[%d]", get_client_name(), m_socket.get_socket_fd());
		return false;
	}

	m_socket.set_connection_mode();

	if (m_socket.send(&client_type, sizeof(client_type)) <= 0) {
		_E("Failed to send client type in client %s, command socket fd[%d]", get_client_name(), m_socket.get_socket_fd());
		return false;
	}

	return true;
}

void external_data_channel::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool external_data_channel::cmd_get_id(int &client_id)
{
	cpacket *packet;
	cmd_ext_get_id_t *cmd_ext_get_id;
	cmd_ext_get_id_done_t *cmd_ext_get_id_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_ext_get_id_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_EXT_GET_ID);

	cmd_ext_get_id = (cmd_ext_get_id_t *)packet->data();

	get_proc_name(getpid(), cmd_ext_get_id->name);

	_I("%s send cmd_get_id()", get_client_name());

	if (!command_handler(packet, (void **)&cmd_ext_get_id_done)) {
		_E("Client %s failed to send/receive command", get_client_name());
		delete packet;
		return false;
	}

	if (cmd_ext_get_id_done->client_id < 0) {
		_E("Client %s failed to get client_id[%d] from server",
			get_client_name(), cmd_ext_get_id_done->client_id);
		delete[] (char *)cmd_ext_get_id_done;
		delete packet;
		return false;
	}

	client_id = cmd_ext_get_id_done->client_id;

	delete[] (char *)cmd_ext_get_id_done;
	delete packet;

	return true;
}

bool external_data_channel::cmd_connect(const string &key, sensor_id_t &sensor_id)
{
	cpacket *packet;
	cmd_ext_connect_t *cmd_ext_connect;
	cmd_ext_connect_done_t *cmd_ext_connect_done;

	int key_size = key.size();

	if ((key_size == 0) || (key_size >= NAME_MAX)) {
		_I("Key(%s) is not valid", key.c_str());
		return false;
	}

	packet = new(std::nothrow) cpacket(sizeof(cmd_ext_connect_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_EXT_CONNECT);

	cmd_ext_connect = (cmd_ext_connect_t *)packet->data();
	cmd_ext_connect->client_id = m_client_id;
	strncpy(cmd_ext_connect->key, key.c_str(), NAME_MAX-1);

	_I("%s send cmd_get_connect(key = %s, client_id = %d)", get_client_name(), key.c_str(), m_client_id);

	if (!command_handler(packet, (void **)&cmd_ext_connect_done)) {
		_E("Client %s failed to send/receive command", get_client_name());
		delete packet;
		return false;
	}

	if (cmd_ext_connect_done->sensor_id == UNKNOWN_SENSOR) {
		_E("Client %s failed to connect to external sensor", get_client_name());
		delete[] (char *)cmd_ext_connect_done;
		delete packet;
		return false;
	}

	m_sensor_id = sensor_id = cmd_ext_connect_done->sensor_id;

	delete[] (char *)cmd_ext_connect_done;
	delete packet;

	return true;
}

bool external_data_channel::cmd_disconnect(void)
{
	cpacket *packet;
	cmd_ext_done_t *cmd_ext_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_ext_disconnect_t));
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_EXT_DISCONNECT);

	_I("%s send cmd_disconnect(client_id=%d)", get_client_name(), m_client_id);

	if (!command_handler(packet, (void **)&cmd_ext_done)) {
		_E("Client %s failed to send/receive command  with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_ext_done->value < 0) {
		_E("Client %s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_ext_done->value, m_client_id);

		delete[] (char *)cmd_ext_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_ext_done;
	delete packet;

	m_socket.close();
	m_client_id = CLIENT_ID_INVALID;
	return true;
}

bool external_data_channel::cmd_post(unsigned long long timestamp, const float *data, int data_cnt)
{
	cpacket *packet;
	cmd_ext_post_t *cmd_ext_post;
	cmd_done_t *cmd_done;

	packet = new(std::nothrow) cpacket(sizeof(cmd_ext_post_t) + sizeof(float) * data_cnt);
	retvm_if(!packet, false, "Failed to allocate memory");

	packet->set_cmd(CMD_EXT_POST);

	cmd_ext_post = (cmd_ext_post_t*)packet->data();
	cmd_ext_post->timestamp = timestamp;
	cmd_ext_post->data_cnt = data_cnt;
	memcpy(cmd_ext_post->data, data, sizeof(float) * data_cnt);

	_I("%s send cmd_post(client_id=%d, data = %#x, data_cnt = %d)",
		get_client_name(), m_client_id, data, data_cnt);

	if (!command_handler(packet, (void **)&cmd_done)) {
		_E("%s failed to send/receive command with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		_E("%s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_done->value, m_client_id);

		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	return true;
}
