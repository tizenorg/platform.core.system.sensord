/*
 * libsensord-share
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

#ifndef _CCLIENT_SENSOR_RECORD_H_
#define _CCLIENT_SENSOR_RECORD_H_

#include <sensor.h>
#include <sf_common.h>
#include <csensor_usage.h>
#include <csocket.h>
#include <map>

using std::map;

typedef map<sensor_type_t, csensor_usage> sensor_usage_map;

class cclient_sensor_record
{
public:
	cclient_sensor_record();
	~cclient_sensor_record();

	void set_client_id(int client_id);

	void set_client_info(pid_t pid);
	const char *get_client_info(void);

	bool register_event(const unsigned int event_type);
	bool unregister_event(const unsigned int event_type);

	bool set_interval(const sensor_type_t sensor, const unsigned int interval);
	unsigned int get_interval(const sensor_type_t sensor);
	bool set_option(const sensor_type_t sensor, const int option);

	bool set_start(const sensor_type_t sensor, bool start);
	bool is_started(const sensor_type_t sensor);

	bool is_sensor_used(const sensor_type_t sensor, const event_situation mode);
	bool is_listening_event(const unsigned int event_type, const event_situation mode);
	bool has_sensor_usage(void);
	bool has_sensor_usage(const sensor_type_t sensor);

	bool get_registered_events(const sensor_type_t sensor, event_type_vector &event_vec);

	bool add_sensor_usage(const sensor_type_t sensor);
	bool remove_sensor_usage(const sensor_type_t sensor);

	void set_event_socket(const csocket &socket);
	void get_event_socket(csocket &socket);
	bool close_event_socket(void);

private:
	int m_client_id;
	pid_t m_pid;
	string m_client_info;
	csocket m_event_socket;
	sensor_usage_map m_sensor_usages;
};

#endif /*_CCLIENT_SENSOR_RECORD_H_*/
