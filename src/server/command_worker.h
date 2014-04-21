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

#ifndef COMMAND_WORKER_H_
#define COMMAND_WORKER_H_

#include <worker_thread.h>
#include <cclient_info_manager.h>
#include <csensor_event_dispatcher.h>
#include <sensor_base.h>

class command_worker
{
private:
	typedef bool (command_worker::*cmd_handler_t)(void *payload);

	static const int OP_ERROR = -1;
	static const int OP_SUCCESS = 0;

	int m_client_id;
	csocket m_socket;
	worker_thread m_worker;
	sensor_base *m_module;
	sensor_type_t m_sensor_type;

	static cmd_handler_t m_cmd_handlers[CMD_CNT];

	static void init_cmd_handlers(void);

	static bool working(void *ctx);
	static bool stopped(void *ctx);

	bool dispatch_command(int cmd, void *payload);

	bool send_cmd_done(long value);
	bool send_cmd_get_id_done(int client_id);
	bool send_cmd_properties_done(int state, sensor_properties_t *properties);
	bool send_cmd_get_data_done(int state, sensor_data_t *data);

	bool cmd_get_id(void *payload);
	bool cmd_hello(void *payload);
	bool cmd_byebye(void *payload);
	bool cmd_get_value(void *payload);
	bool cmd_start(void *payload);
	bool cmd_stop(void *payload);
	bool cmd_register_event(void *payload);
	bool cmd_unregister_event(void *payload);
	bool cmd_check_event(void *payload);
	bool cmd_set_interval(void *payload);
	bool cmd_unset_interval(void *payload);
	bool cmd_set_option(void *payload);
	bool cmd_set_command(void *payload);
	bool cmd_get_properties(void *payload);
	bool cmd_get_data(void *payload);
	bool cmd_send_sensorhub_data(void *payload);

	const char *get_info(void);

	static cclient_info_manager &get_client_info_manager(void);
	static csensor_event_dispatcher &get_event_dispathcher(void);
public:
	command_worker(const csocket &socket);
	virtual ~command_worker();

	bool start(void);
};
#endif /* COMMAND_WORKER_H_ */
