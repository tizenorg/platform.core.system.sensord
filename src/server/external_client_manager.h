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

#ifndef _EXTERNAL_CLIENT_MANAGER_H_
#define _EXTERNAL_CLIENT_MANAGER_H_

#include <external_sensor_record.h>
#include <cmutex.h>
#include <unordered_map>
#include <memory>

class external_client_manager {
public:
	static external_client_manager& get_instance(void);
	int create_client_record(void);
	bool remove_client_record(int client_id);
	bool has_client_record(int client_id);

	void set_client_info(int client_id, pid_t pid, const std::string &name);
	const char* get_client_info(int client_id);

	bool create_sensor_record(int client_id, sensor_id_t sensor);
	bool remove_sensor_record(int client_id, sensor_id_t sensor);
	bool has_sensor_record(int client_id, sensor_id_t sensor);
	bool has_sensor_record(int client_id);

	bool get_listener_socket(sensor_id_t sensor, csocket &sock);
	bool get_command_socket(int client_id, csocket &sock);
	bool set_command_socket(int client_id, const csocket &sock);
private:
	external_client_manager();
	~external_client_manager();
	external_client_manager(const external_client_manager&) {};
	external_client_manager& operator=(const external_client_manager&);

	std::unordered_map<int, std::shared_ptr<external_sensor_record>> m_clients;
	cmutex m_mutex;
};
#endif /* _EXTERNAL_CLIENT_MANAGER_H_ */
