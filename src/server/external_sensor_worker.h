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

#ifndef _EXTERNAL_SENSOR_WORKER_H_
#define _EXTERNAL_SENSOR_WORKER_H_

#include <worker_thread.h>
#include <csocket.h>
#include <sensor_common.h>
#include <string>

class external_client_manager;
class external_sensor;

class external_sensor_worker {
private:
	typedef bool (external_sensor_worker::*cmd_handler_t)(void *payload);

	int m_client_id;
	csocket m_socket;
	worker_thread m_worker;
	external_sensor *m_sensor;
	sensor_id_t m_sensor_id;
	static cmd_handler_t m_cmd_handlers[CMD_EXT_CNT];

	static void init_cmd_handlers(void);
	static bool working(void *ctx);
	static bool stopped(void *ctx);

	bool dispatch_command(int cmd, void *payload);

	bool send_cmd_done(long value);
	bool send_cmd_get_id_done(int client_id);
	bool send_cmd_connect_done(sensor_id_t sensor_id);

	bool cmd_get_id(void *payload);
	bool cmd_connect(void *payload);
	bool cmd_disconnect(void *payload);
	bool cmd_post(void *payload);

	static external_client_manager& get_client_manager(void);

	void get_info(std::string &info);
public:
	external_sensor_worker(const csocket& socket);
	~external_sensor_worker();

	bool start(void);
};

#endif /* _EXTERNAL_SENSOR_WORKER_H_ */
