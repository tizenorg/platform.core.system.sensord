/*
 * sensord
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

#include <command_worker.h>
#include <sensor_plugin_loader.h>
#include <thread>
#include <string>
#include <set>

using namespace std;
using std::string;
using std::set;

set<unsigned int> priority_list;

command_worker::cmd_handler_t command_worker::m_cmd_handlers[];

command_worker::command_worker(const csocket &socket)
: m_client_id(CLIENT_ID_INVALID)
, m_socket(socket)
, m_module(NULL)
, m_sensor_type(UNKNOWN_SENSOR)
{
	init_cmd_handlers();

	m_worker.set_context(this);
	m_worker.set_working(working);
	m_worker.set_stopped(stopped);
}

command_worker::~command_worker()
{
	m_socket.close();
}

bool command_worker::start(void)
{
	return m_worker.start();
}

void command_worker::init_cmd_handlers(void)
{
	static bool init = false;

	if (!init) {
		m_cmd_handlers[CMD_GET_ID]					= &command_worker::cmd_get_id;
		m_cmd_handlers[CMD_HELLO]					= &command_worker::cmd_hello;
		m_cmd_handlers[CMD_BYEBYE]					= &command_worker::cmd_byebye;
		m_cmd_handlers[CMD_START]					= &command_worker::cmd_start;
		m_cmd_handlers[CMD_STOP]					= &command_worker::cmd_stop;
		m_cmd_handlers[CMD_REG]					= &command_worker::cmd_register_event;
		m_cmd_handlers[CMD_UNREG]					= &command_worker::cmd_unregister_event;
		m_cmd_handlers[CMD_CHECK_EVENT]			= &command_worker::cmd_check_event;
		m_cmd_handlers[CMD_SET_OPTION]			= &command_worker::cmd_set_option;
		m_cmd_handlers[CMD_SET_INTERVAL]			= &command_worker::cmd_set_interval;
		m_cmd_handlers[CMD_UNSET_INTERVAL]		= &command_worker::cmd_unset_interval;
		m_cmd_handlers[CMD_SET_COMMAND]			= &command_worker::cmd_set_command;
		m_cmd_handlers[CMD_GET_PROPERTIES]		= &command_worker::cmd_get_properties;
		m_cmd_handlers[CMD_GET_DATA]				= &command_worker::cmd_get_data;
		m_cmd_handlers[CMD_SEND_SENSORHUB_DATA]	= &command_worker::cmd_send_sensorhub_data;
		init = true;
	}
}

bool command_worker::working(void *ctx)
{
	int ret;
	command_worker *inst = (command_worker *)ctx;
	packet_header header;
	char *payload;

	if (inst->m_socket.recv(&header, sizeof(header)) <= 0) {
		DBG("%s failed to receive header", inst->get_info());
		return false;
	}

	if (header.size > 0) {
		payload = new char[header.size];

		if (inst->m_socket.recv(payload, header.size) <= 0) {
			DBG("%s failed to receive data of packet", inst->get_info());
			delete[] payload;
			return false;
		}
	} else {
		payload = NULL;
	}

	ret = inst->dispatch_command(header.cmd, payload);

	if (payload)
		delete[] payload;

	return ret;
}

bool command_worker::stopped(void *ctx)
{
	event_type_vector event_vec;
	command_worker *inst = (command_worker *)ctx;

	INFO("%s is stopped", inst->get_info());

	if ((inst->m_module) && (inst->m_client_id != CLIENT_ID_INVALID)) {
		get_client_info_manager().get_registered_events(inst->m_client_id, inst->m_sensor_type, event_vec);
		event_type_vector::iterator it_event;
		it_event = event_vec.begin();

		while (it_event != event_vec.end()) {
			WARN("Does not unregister event[0x%x] before connection broken for [%s]!!", *it_event, inst->m_module->get_name());

			if (!inst->m_module->delete_client(*it_event))
				ERR("Unregistering event[0x%x] failed", *it_event);

			++it_event;
		}

		if (get_client_info_manager().is_started(inst->m_client_id, inst->m_sensor_type)) {
			WARN("Does not receive cmd_stop before connection broken for [%s]!!", inst->m_module->get_name());
			inst->m_module->delete_interval(inst->m_client_id, false);
			inst->m_module->stop();
		}

		if (inst->m_sensor_type) {
			if (get_client_info_manager().has_sensor_record(inst->m_client_id, inst->m_sensor_type)) {
				INFO("Removing sensor[0x%x] record for client_id[%d]", inst->m_sensor_type, inst->m_client_id);
				get_client_info_manager().remove_sensor_record(inst->m_client_id, inst->m_sensor_type);
			}
		}
	}

	delete inst;
	return true;
}

bool command_worker::dispatch_command(int cmd, void *payload)
{
	int ret = false;

	if (!(cmd > 0 && cmd < CMD_CNT)) {
		ERR("Unknown command: %d", cmd);
	} else {
		cmd_handler_t cmd_handler;
		cmd_handler = command_worker::m_cmd_handlers[cmd];

		if (cmd_handler)
			ret = (this->*cmd_handler)(payload);
	}

	return ret;
}

bool command_worker::send_cmd_done(long value)
{
	cpacket *ret_packet;
	cmd_done_t *cmd_done;

	ret_packet = new cpacket(sizeof(cmd_done_t));
	ret_packet->set_cmd(CMD_DONE);
	cmd_done = (cmd_done_t *)ret_packet->data();
	cmd_done->value = value;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		ERR("Failed to send a cmd_done to client_id [%d] with value [%ld]", m_client_id, value);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_get_id_done(int client_id)
{
	cpacket *ret_packet;
	cmd_get_id_done_t *cmd_get_id_done;

	ret_packet = new cpacket(sizeof(cmd_get_id_done_t));
	ret_packet->set_cmd(CMD_GET_ID);
	cmd_get_id_done = (cmd_get_id_done_t *)ret_packet->data();
	cmd_get_id_done->client_id = client_id;

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		ERR("Failed to send a cmd_get_id_done with client_id [%d]", client_id);
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_properties_done(int state, sensor_properties_t *properties)
{
	cpacket *ret_packet;
	cmd_properties_done_t *cmd_properties_done;

	ret_packet = new cpacket(sizeof(cmd_properties_done_t));
	ret_packet->set_cmd(CMD_GET_PROPERTIES);
	cmd_properties_done = (cmd_properties_done_t *)ret_packet->data();
	cmd_properties_done->state = state;
	memcpy(&cmd_properties_done->properties, properties , sizeof(sensor_properties_t));

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		ERR("Failed to send a cmd_get_properties");
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::send_cmd_get_data_done(int state, sensor_data_t *data)
{
	cpacket *ret_packet;
	cmd_get_data_done_t *cmd_get_data_done;

	ret_packet = new cpacket(sizeof(cmd_get_data_done_t));
	ret_packet->set_cmd(CMD_GET_DATA);
	cmd_get_data_done = (cmd_get_data_done_t *)ret_packet->data();
	cmd_get_data_done->state = state;
	memcpy(&cmd_get_data_done->base_data , data, sizeof(sensor_data_t));

	if (m_socket.send(ret_packet->packet(), ret_packet->size()) <= 0) {
		ERR("Failed to send a cmd_get_data_done");
		delete ret_packet;
		return false;
	}

	delete ret_packet;
	return true;
}

bool command_worker::cmd_get_id(void *payload)
{
	DBG("CMD_GET_ID Handler invoked");

	cmd_get_id_t *cmd;
	int client_id;

	cmd = (cmd_get_id_t *)payload;
	client_id = get_client_info_manager().create_client_record();
	get_client_info_manager().set_client_info(client_id, cmd->pid);
	INFO("New client id [%d] created", client_id);

	if (!send_cmd_get_id_done(client_id))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_hello(void *payload)
{
	DBG("CMD_HELLO Handler invoked");

	cmd_hello_t *cmd;
	long ret_value = OP_ERROR;

	cmd = (cmd_hello_t *)payload;
	m_sensor_type = static_cast<sensor_type_t>(cmd->sensor);
	m_client_id = cmd->client_id;
	DBG("Hello sensor [0x%x], client id [%d]", m_sensor_type, m_client_id);
	m_module = (sensor_base *)sensor_plugin_loader::get_instance().get_sensor(m_sensor_type);

	if (m_module) {
		get_client_info_manager().create_sensor_record(m_client_id, m_sensor_type);
		INFO("New sensor record created for sensor [0x%x], sensor name [%s] on client id [%d]", m_sensor_type, m_module->get_name(), m_client_id);
		ret_value = OP_SUCCESS;
	} else {
		ERR("Sensor type[0x%x] is not supported", m_sensor_type);

		if (!get_client_info_manager().has_sensor_record(m_client_id))
			get_client_info_manager().remove_client_record(m_client_id);
	}

	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_byebye(void *payload)
{
	long ret_value;
	DBG("CMD_BYEBYE for client [%d], sensor [0x%x]", m_client_id, m_sensor_type);

	if (!get_client_info_manager().remove_sensor_record(m_client_id, m_sensor_type)) {
		ERR("Error removing sensor_record for client [%d]", m_client_id);
		ret_value = OP_ERROR;
	} else {
		m_client_id = CLIENT_ID_INVALID;
		ret_value = OP_SUCCESS;
	}

	if (!send_cmd_done(ret_value))
		ERR("Failed to send cmd_done to a client");

	if (ret_value == OP_SUCCESS)
		return false;

	return true;
}

bool command_worker::cmd_start(void *payload)
{
	long value = OP_SUCCESS;
	DBG("START Sensor [0x%x], called from client [%d]", m_sensor_type, m_client_id);
	DBG("Invoke Module start for []");

	if (m_module->start()) {
		get_client_info_manager().set_start(m_client_id, m_sensor_type, true);

		/*
		 *	Rotation could be changed even LCD is off by pop sync rotation
		 *	and a client listening rotation event with always-on option.
		 *	To reflect the last rotation state, request it to event dispatcher.
		 */
		get_event_dispathcher().request_last_event(m_client_id, m_sensor_type);
	} else {
		value = OP_ERROR;
	}

	if (!send_cmd_done(value))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_stop(void *payload)
{
	long ret_val = OP_SUCCESS;
	DBG("STOP Sensor [0x%x], called from client [%d]", m_sensor_type, m_client_id);

	if (m_module->stop()) {
		get_client_info_manager().set_start(m_client_id, m_sensor_type, false);
	}

	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_register_event(void *payload)
{
	cmd_reg_t *cmd;
	long ret_val = OP_ERROR;

	cmd = (cmd_reg_t *)payload;
	
	if (!get_client_info_manager().register_event(m_client_id, cmd->event_type)) {
		INFO("Failed to register event [0x%x] for client [%d] to client info manager",
			cmd->event_type, m_client_id);
		goto out;
	}
	if (cmd->event_type == GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME || cmd->event_type ==  LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME || cmd->event_type == ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME) {
			priority_list.insert(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
			priority_list.insert(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
			priority_list.insert(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);
			
	}			
	
	m_module->add_client(cmd->event_type);
	ret_val = OP_SUCCESS;
	DBG("Registering Event [0x%x] is done for client [%d]", cmd->event_type, m_client_id);

 out:

	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unregister_event(void *payload)
{
	cmd_unreg_t *cmd;
	long ret_val = OP_ERROR;
	cmd = (cmd_unreg_t *)payload;

	if (!get_client_info_manager().unregister_event(m_client_id, cmd->event_type)) {
		ERR("Failed to unregister event [0x%x] for client [%d from client info manager",
			cmd->event_type, m_client_id);
		goto out;
	}

	if (!m_module->delete_client(cmd->event_type)) {
		ERR("Failed to unregister event [0x%x] for client [%d]",
			cmd->event_type, m_client_id);
		goto out;
	}

	ret_val = OP_SUCCESS;
	DBG("Unregistering Event [0x%x] is done for client [%d]",
		cmd->event_type, m_client_id);

out:
	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_check_event(void *payload)
{
	cmd_check_event_t *cmd;
	long ret_val = OP_ERROR;
	cmd = (cmd_check_event_t *)payload;

	if (m_module->is_supported(cmd->event_type)) {
		ret_val = OP_SUCCESS;
		DBG("Event[0x%x] is supported for client [%d], for sensor [0x%x]", cmd->event_type, m_client_id, (cmd->event_type >> 16));
	}

	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_interval(void *payload)
{
	cmd_set_interval_t *cmd;
	long ret_val = OP_ERROR;
	cmd = (cmd_set_interval_t *)payload;

	if (!get_client_info_manager().set_interval(m_client_id, m_sensor_type, cmd->interval)) {
		ERR("Failed to register interval for client [%d], for sensor [0x%x] with interval [%d] to client info manager",
			m_client_id, m_sensor_type, cmd->interval);
		goto out;
	}

	if (!m_module->add_interval(m_client_id, cmd->interval, false)) {
		ERR("Failed to set interval for client [%d], for sensor [0x%x] with interval [%d]",
			m_client_id, m_sensor_type, cmd->interval);
		goto out;
	}

	ret_val = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_unset_interval(void *payload)
{
	long ret_val = OP_ERROR;

	if (!get_client_info_manager().set_interval(m_client_id, m_sensor_type, 0)) {
		ERR("Failed to unregister interval for client [%d], for sensor [0x%x] to client info manager",
			m_client_id, m_sensor_type);
		goto out;
	}

	if (!m_module->delete_interval(m_client_id, false)) {
		ERR("Failed to delete interval for client [%d]", m_client_id);
		goto out;
	}

	ret_val = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_option(void *payload)
{
	cmd_set_option_t *cmd;
	long ret_val = OP_ERROR;
	cmd = (cmd_set_option_t *)payload;

	if (!get_client_info_manager().set_option(m_client_id, m_sensor_type, cmd->option)) {
		ERR("Failed to register interval for client [%d], for sensor [0x%x] with option [%d] to client info manager",
			m_client_id, m_sensor_type, cmd->option);
		goto out;
	}

	ret_val = OP_SUCCESS;

out:
	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_set_command(void *payload)
{
	DBG("CMD_SET_COMMAND  Handler invoked");

	cmd_set_command_t *cmd;
	long ret_val = OP_ERROR;
	cmd = (cmd_set_command_t *)payload;
	ret_val = m_module->set_command(cmd->cmd, cmd->value);

	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_get_properties(void *payload)
{
	DBG("CMD_GET_PROPERTIES Handler invoked");
	int state = OP_ERROR;
	cmd_get_properties_t *cmd;
	sensor_properties_t sensor_properties;

	cmd = (cmd_get_properties_t *) payload;
	memset(&sensor_properties, 0, sizeof(sensor_properties));
	state = m_module->get_properties(cmd->type, sensor_properties);

	if (state != 0)
		ERR("processor_module get_property fail");

	if (!send_cmd_properties_done(state, &sensor_properties))
		ERR("Failed to send cmd_done to a client");

	return true;
}

bool command_worker::cmd_get_data(void *payload)
{
	DBG("CMD_GET_VALUE Handler invoked");
	cmd_get_data_t *cmd;
	int state = OP_ERROR;
	sensor_data_t base_data;

	cmd = (cmd_get_data_t *)payload;
	state = m_module->get_sensor_data(cmd->type, base_data);

	if (state != 0)
		ERR("processor_module cmd_get_data fail");

	send_cmd_get_data_done(state, &base_data);
	return true;
}

bool command_worker::cmd_send_sensorhub_data(void *payload)
{
	DBG("CMD_SEND_SENSORHUB_DATA Handler invoked");
	cmd_send_sensorhub_data_t *cmd;
	long ret_val = OP_ERROR;

	cmd = (cmd_send_sensorhub_data_t *)payload;
	ret_val = m_module->send_sensorhub_data(cmd->data, cmd->data_len);

	if (!send_cmd_done(ret_val))
		ERR("Failed to send cmd_done to a client");

	return true;
}

const char *command_worker::get_info(void)
{
	static string info;
	const char *client_info = NULL;
	const char *sensor_info = NULL;

	if (m_client_id != CLIENT_ID_INVALID)
		client_info = get_client_info_manager().get_client_info(m_client_id);

	if (m_module)
		sensor_info = m_module->get_name();

	info = string("Command worker for ") + (client_info ? client_info : "Unknown") + "'s "
		   + (sensor_info ? sensor_info : "Unknown");
	return info.c_str();
}

cclient_info_manager &command_worker::get_client_info_manager(void)
{
	return cclient_info_manager::get_instance();
}

csensor_event_dispatcher &command_worker::get_event_dispathcher(void)
{
	return csensor_event_dispatcher::get_instance();
}

