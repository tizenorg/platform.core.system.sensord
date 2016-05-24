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

#ifndef _EXTERNAL_SENSOR_RECORD_H_
#define _EXTERNAL_SENSOR_RECORD_H_

#include <csocket.h>
#include <sensor_common.h>
#include <string>
#include <unordered_set>

class external_sensor_record {
public:
	external_sensor_record();
	~external_sensor_record();

	void set_client_id(int client_id);

	void set_client_info(pid_t pid, const std::string &name);
	const char* get_client_info(void);

	bool has_usage(void);
	bool has_usage(sensor_id_t sensor);

	bool add_usage(sensor_id_t sensor);
	bool remove_usage(sensor_id_t sensor);

	void set_command_socket(const csocket &socket);
	void get_command_socket(csocket &socket);
	bool close_command_socket(void);

private:
	int m_client_id;
	pid_t m_pid;
	std::string m_client_info;
	csocket m_command_socket;
	std::unordered_set<sensor_id_t> m_usages;
};

#endif /* _EXTERNAL_SENSOR_RECORD_H_ */
