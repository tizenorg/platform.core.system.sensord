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

#include <command_channel.h>
#include <client_common.h>
#include <sf_common.h>

command_channel::command_channel()
: m_client_id(CLIENT_ID_INVALID)
, m_sensor_type(UNKNOWN_SENSOR)
{
}

command_channel::~command_channel()
{
	if (m_command_socket.is_valid())
		m_command_socket.close();
}

bool command_channel::command_handler(cpacket *packet, void **return_payload)
{
	if (!m_command_socket.is_valid()) {
		ERR("Command socket(%d) is not valid for client %s", m_command_socket.get_socket_fd(), get_client_name());
		return false;
	}

	if (packet->size() < 0) {
		ERR("Packet is not valid for client %s", get_client_name());
		return false;
	}

	if (m_command_socket.send(packet->packet(), packet->size()) <= 0) {
		ERR("Failed to send command in client %s", get_client_name());
		return false;
	}

	packet_header header;

	if (m_command_socket.recv(&header, sizeof(header)) <= 0) {
		ERR("Failed to receive header for reply packet in client %s", get_client_name());
		return false;
	}

	char *buffer = new char[header.size];

	if (m_command_socket.recv(buffer, header.size) <= 0) {
		ERR("Failed to receive reply packet in client %s", get_client_name());
		delete[] buffer;
		return false;
	}

	*return_payload = buffer;
	return true;
}

bool command_channel::create_channel(void)
{
	if (!m_command_socket.create(SOCK_STREAM))
		return false;

	if (!m_command_socket.connect(COMMAND_CHANNEL_PATH)) {
		ERR("Failed to connect command channel for client %s, command socket fd[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	m_command_socket.set_connection_mode();
	return true;
}

void command_channel::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool command_channel::cmd_get_id(int &client_id)
{
	cpacket *packet;
	cmd_get_id_t *cmd_get_id;
	cmd_get_id_done_t *cmd_get_id_done;

	packet = new cpacket(sizeof(cmd_get_id_t));
	packet->set_cmd(CMD_GET_ID);
	packet->set_payload_size(sizeof(cmd_get_id_t));
	cmd_get_id = (cmd_get_id_t *)packet->data();
	cmd_get_id->pid = getpid();

	INFO("%s send cmd_get_id()", get_client_name());

	if (!command_handler(packet, (void **)&cmd_get_id_done)) {
		ERR("Client %s failed to send/receive command", get_client_name());
		delete packet;
		return false;
	}

	if (cmd_get_id_done->client_id < 0) {
		ERR("Client %s failed to get client_id[%d] from server",
			get_client_name(), cmd_get_id_done->client_id);
		delete[] (char *)cmd_get_id_done;
		delete packet;
		return false;
	}

	client_id = cmd_get_id_done->client_id;

	delete[] (char *)cmd_get_id_done;
	delete packet;
	return true;
}

bool command_channel::cmd_hello(sensor_type_t sensor)
{
	cpacket *packet;
	cmd_hello_t *cmd_hello;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_hello_t));
	packet->set_cmd(CMD_HELLO);
	packet->set_payload_size(sizeof(cmd_hello_t));
	cmd_hello = (cmd_hello_t *)packet->data();
	cmd_hello->client_id = m_client_id;
	cmd_hello->sensor = sensor;

	INFO("%s send cmd_hello(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(sensor));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s]",
			get_client_name(), get_sensor_name(sensor));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("client %s got error[%d] from server with sensor [%s]",
			get_client_name(), cmd_done->value, get_sensor_name(sensor));
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	m_sensor_type = sensor;
	return true;
}

bool command_channel::cmd_byebye(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_byebye_t));
	packet->set_cmd(CMD_BYEBYE);
	packet->set_payload_size(sizeof(cmd_byebye_t));

	INFO("%s send cmd_byebye(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;

	if (m_command_socket.is_valid())
		m_command_socket.close();

	m_client_id = CLIENT_ID_INVALID;
	m_sensor_type = UNKNOWN_SENSOR;
	return true;
}

bool command_channel::cmd_start(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_start_t));
	packet->set_cmd(CMD_START);
	packet->set_payload_size(sizeof(cmd_start_t));

	INFO("%s send cmd_start(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_stop(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_stop_t));
	packet->set_cmd(CMD_STOP);
	packet->set_payload_size(sizeof(cmd_stop_t));

	INFO("%s send cmd_stop(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_set_option(int option)
{
	cpacket *packet;
	cmd_set_option_t *cmd_set_option;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_set_option_t));
	packet->set_cmd(CMD_SET_OPTION);
	packet->set_payload_size(sizeof(cmd_set_option_t));
	cmd_set_option = (cmd_set_option_t *)packet->data();
	cmd_set_option->option = option;

	INFO("%s send cmd_set_option(client_id=%d, %s, option=%d)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type), option);

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d], option[%]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id, option);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d], option[%]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id, option);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_register_event(unsigned int event_type)
{
	cpacket *packet;
	cmd_reg_t *cmd_reg;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_reg_t));
	packet->set_cmd(CMD_REG);
	packet->set_payload_size(sizeof(cmd_reg_t));
	cmd_reg = (cmd_reg_t *)packet->data();
	cmd_reg->event_type = event_type;

	INFO("%s send cmd_register_event(client_id=%d, %s)",
		get_client_name(), m_client_id, get_event_name(event_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command with client_id [%d], event_type[%s]",
			get_client_name(), m_client_id, get_event_name(event_type));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server with client_id [%d], event_type[%s]",
			get_client_name(), cmd_done->value, m_client_id, get_event_name(event_type));
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_register_events(event_type_vector &event_vec)
{
	event_type_vector::iterator it_event;
	it_event = event_vec.begin();

	while (it_event != event_vec.end()) {
		if (!cmd_register_event(*it_event))
			return false;

		++it_event;
	}

	return true;
}

bool command_channel::cmd_unregister_event(unsigned int event_type)
{
	cpacket *packet;
	cmd_unreg_t *cmd_unreg;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_unreg_t));
	packet->set_cmd(CMD_UNREG);
	packet->set_payload_size(sizeof(cmd_unreg_t));
	cmd_unreg = (cmd_unreg_t *)packet->data();
	cmd_unreg->event_type = event_type;

	INFO("%s send cmd_unregister_event(client_id=%d, %s)",
		get_client_name(), m_client_id, get_event_name(event_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command with client_id [%d], event_type[%s]",
			get_client_name(), m_client_id, get_event_name(event_type));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server with client_id [%d], event_type[%s]",
			get_client_name(), cmd_done->value, m_client_id, get_event_name(event_type));
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_unregister_events(event_type_vector &event_vec)
{
	event_type_vector::iterator it_event;
	it_event = event_vec.begin();

	while (it_event != event_vec.end()) {
		if (!cmd_unregister_event(*it_event))
			return false;

		++it_event;
	}

	return true;
}

bool command_channel::cmd_check_event(unsigned int event_type)
{
	cpacket *packet;
	cmd_check_event_t *cmd_check_event;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_check_event_t));
	packet->set_cmd(CMD_CHECK_EVENT);
	packet->set_payload_size(sizeof(cmd_check_event_t));
	cmd_check_event = (cmd_check_event_t *)packet->data();
	cmd_check_event->event_type = event_type;

	INFO("%s send cmd_check_event(client_id=%d, %s)",
		get_client_name(), m_client_id, get_event_name(event_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command with client_id [%d], event_type[%s]",
			get_client_name(), m_client_id, get_event_name(event_type));
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_set_interval(unsigned int interval)
{
	cpacket *packet;
	cmd_set_interval_t *cmd_set_interval;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_set_interval_t));
	packet->set_cmd(CMD_SET_INTERVAL);
	packet->set_payload_size(sizeof(cmd_set_interval_t));
	cmd_set_interval = (cmd_set_interval_t *)packet->data();
	cmd_set_interval->interval = interval;

	INFO("%s send cmd_set_interval(client_id=%d, %s, interval=%d)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type), interval);

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("%s failed to send/receive command for sensor[%s] with client_id [%d], interval[%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id, interval);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("%s got error[%d] from server for sensor[%s] with client_id [%d], interval[%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id, interval);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_unset_interval(void)
{
	cpacket *packet;
	cmd_done_t *cmd_done;
	packet = new cpacket(sizeof(cmd_unset_interval_t));
	packet->set_cmd(CMD_UNSET_INTERVAL);
	packet->set_payload_size(sizeof(cmd_unset_interval_t));

	INFO("%s send cmd_unset_interval(client_id=%d, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type));

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), m_client_id);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

static bool is_sensor_property(unsigned int type)
{
	return ((type & 0xFFFF) == 1);
}

bool command_channel::cmd_get_properties(unsigned int type, void *properties)
{
	cpacket *packet;
	cmd_get_properties_t *cmd_get_properties;
	cmd_properties_done_t *cmd_properties_done;

	packet = new cpacket(sizeof(cmd_get_properties_t));
	packet->set_cmd(CMD_GET_PROPERTIES);
	packet->set_payload_size(sizeof(cmd_get_properties_t));
	cmd_get_properties = (cmd_get_properties_t *)packet->data();
	cmd_get_properties->type = type;

	INFO("%s send cmd_get_properties(client_id=%d, %s, %s)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type), get_data_name(type));

	if (!command_handler(packet, (void **)&cmd_properties_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d], data_id[%s]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id, get_data_name(type));
		delete packet;
		return false;
	}

	if (cmd_properties_done->state < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with client_id [%d], data_id[%s]",
			get_client_name(), cmd_properties_done->state, get_sensor_name(m_sensor_type), m_client_id, get_data_name(type));
		delete[] (char *)cmd_properties_done;
		delete packet;
		return false;
	}

	sensor_properties_t *ret_properties;
	ret_properties = &cmd_properties_done->properties;

	if (is_sensor_property(type)) {
		sensor_properties_t *sensor_properties;
		sensor_properties = (sensor_properties_t *)properties;
		sensor_properties->sensor_unit_idx = ret_properties->sensor_unit_idx;
		sensor_properties->sensor_min_range = ret_properties->sensor_min_range;
		sensor_properties->sensor_max_range = ret_properties->sensor_max_range;
		sensor_properties->sensor_resolution = ret_properties->sensor_resolution;
		strncpy(sensor_properties->sensor_name, ret_properties->sensor_name, strlen(ret_properties->sensor_name));
		strncpy(sensor_properties->sensor_vendor, ret_properties->sensor_vendor, strlen(ret_properties->sensor_vendor));
	} else {
		sensor_data_properties_t *data_properies;
		data_properies = (sensor_data_properties_t *)properties;
		data_properies->sensor_unit_idx = ret_properties->sensor_unit_idx ;
		data_properies->sensor_min_range = ret_properties->sensor_min_range;
		data_properies->sensor_max_range = ret_properties->sensor_max_range;
		data_properies->sensor_resolution = ret_properties->sensor_resolution;
	}

	delete[] (char *)cmd_properties_done;
	delete packet;
	return true;
}

bool command_channel::cmd_set_command(unsigned int cmd, long value)
{
	cpacket *packet;
	cmd_set_command_t *cmd_set_command;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_set_command_t));
	packet->set_cmd(CMD_SET_COMMAND);
	packet->set_payload_size(sizeof(cmd_set_command_t));
	cmd_set_command = (cmd_set_command_t *)packet->data();
	cmd_set_command->cmd = cmd;
	cmd_set_command->value = value;

	INFO("%s send cmd_set_command(client_id=%d, %s, 0x%x, %ld)",
		get_client_name(), m_client_id, get_sensor_name(m_sensor_type), cmd, value);

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("Client %s failed to send/receive command for sensor[%s] with client_id [%d], property[0x%x], value[%d]",
			get_client_name(), get_sensor_name(m_sensor_type), m_client_id, cmd, value);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("Client %s got error[%d] from server for sensor[%s] with property[0x%x], value[%d]",
			get_client_name(), cmd_done->value, get_sensor_name(m_sensor_type), cmd, value);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}

bool command_channel::cmd_get_data(unsigned int type, sensor_data_t *sensor_data)
{
	cpacket *packet;
	cmd_get_data_t *cmd_get_data;
	cmd_get_data_done_t *cmd_get_data_done;

	packet = new cpacket(sizeof(cmd_get_data_done_t));
	packet->set_cmd(CMD_GET_DATA);
	packet->set_payload_size(sizeof(cmd_get_data_t));
	cmd_get_data = (cmd_get_data_t *)packet->data();
	cmd_get_data->type = type;

	if (!command_handler(packet, (void **)&cmd_get_data_done)) {
		ERR("Client %s failed to send/receive command with client_id [%d], data_id[%s]",
			get_client_name(), m_client_id, get_data_name(type));
		delete packet;
		return false;
	}

	if (cmd_get_data_done->state < 0) {
		ERR("Client %s got error[%d] from server with client_id [%d], data_id[%s]",
			get_client_name(), cmd_get_data_done->state, m_client_id, get_data_name(type));
		sensor_data->data_accuracy = SENSOR_ACCURACY_UNDEFINED;
		sensor_data->data_unit_idx = SENSOR_UNDEFINED_UNIT;
		sensor_data->timestamp = 0;
		sensor_data->values_num = 0;

		delete[] (char *)cmd_get_data_done;
		delete packet;
		return false;
	}

	sensor_data_t *base_data;
	base_data = &cmd_get_data_done->base_data;
	sensor_data->timestamp = base_data->timestamp;
	sensor_data->data_accuracy = base_data->data_accuracy;
	sensor_data->data_unit_idx = base_data->data_unit_idx;
	sensor_data->values_num = base_data->values_num;
	memcpy(sensor_data->values, base_data->values,
			sizeof(sensor_data->values[0]) * base_data->values_num);

	delete[] (char *)cmd_get_data_done;
	delete packet;
	return true;
}

bool command_channel::cmd_send_sensorhub_data(int data_len, const char *buffer)
{
	cpacket *packet;
	cmd_send_sensorhub_data_t *cmd_send_sensorhub_data;
	cmd_done_t *cmd_done;

	packet = new cpacket(sizeof(cmd_send_sensorhub_data_t) + data_len);
	packet->set_cmd(CMD_SEND_SENSORHUB_DATA);
	cmd_send_sensorhub_data = (cmd_send_sensorhub_data_t *)packet->data();
	cmd_send_sensorhub_data->data_len = data_len;
	memcpy(cmd_send_sensorhub_data->data, buffer, data_len);

	INFO("%s send cmd_send_sensorhub_data(client_id=%d, data_len = %d, buffer = 0x%x)",
		get_client_name(), m_client_id, data_len, buffer);

	if (!command_handler(packet, (void **)&cmd_done)) {
		ERR("%s failed to send/receive command with client_id [%d]",
			get_client_name(), m_client_id);
		delete packet;
		return false;
	}

	if (cmd_done->value < 0) {
		ERR("%s got error[%d] from server with client_id [%d]",
			get_client_name(), cmd_done->value, m_client_id);
		delete[] (char *)cmd_done;
		delete packet;
		return false;
	}

	delete[] (char *)cmd_done;
	delete packet;
	return true;
}
