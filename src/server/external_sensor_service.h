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

#ifndef _EXTERNAL_SENSOR_SERVICE_H_
#define _EXTERNAL_SENSOR_SERVICE_H_

#include <external_sensor.h>
#include <csocket.h>
#include <string>
#include <unordered_map>

class external_client_manager;

class external_sensor_service {
public:
	static external_sensor_service& get_instance();
	bool register_sensor(external_sensor *sensor);
	bool unregister_sensor(external_sensor *sensor);
	external_sensor* get_sensor(const std::string& key);

	void accept_command_channel(csocket client_socket);
	bool run(void);
private:
	external_sensor_service();
	~external_sensor_service();
	external_sensor_service(const external_sensor_service&) {};
	external_sensor_service& operator=(const external_sensor_service&);

	static external_client_manager& get_client_manager(void);

	void dispatch_command(void);

	std::unordered_map<std::string, external_sensor*> m_external_sensors;
};
#endif /* _EXTERNAL_SENSOR_SERVICE_H_ */

