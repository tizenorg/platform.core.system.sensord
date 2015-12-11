/*
 * libsensord
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

#ifndef CSENSOR_CLIENT_INFO_H_
#define CSENSOR_CLIENT_INFO_H_

#include <glib.h>
#include <sys/types.h>
#include <csensor_handle_info.h>
#include <unistd.h>
#include <csocket.h>
#include <string.h>
#include <sf_common.h>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cmutex.h>
#include <poller.h>

using std::unordered_map;
using std::vector;
using std::string;
using std::queue;
using std::mutex;
using std::lock_guard;
using std::unique_lock;
using std::condition_variable;

typedef vector<unsigned int> handle_vector;
typedef vector<sensor_id_t> sensor_id_vector;
typedef unordered_map<int,csensor_handle_info> sensor_handle_info_map;
typedef unordered_map<sensor_id_t, command_channel*> sensor_command_channel_map;

typedef struct sensor_rep
{
	bool active;
	int option;
	unsigned int interval;
	unsigned int latency;
	event_type_vector event_types;
} sensor_rep;

class csensor_client_info {
public:
	static csensor_client_info& get_instance(void);
	int create_handle(sensor_id_t sensor_id);
	bool delete_handle(int handle);
	bool register_event(int handle, unsigned int event_type,
			unsigned int interval, unsigned int latency, int cb_type, void *cb, void* user_data);
	bool unregister_event(int handle, unsigned int event_type);

	bool register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void* user_data);
	bool unregister_accuracy_cb(int handle);

	bool set_sensor_params(int handle, int sensor_state, int sensor_option);
	bool get_sensor_params(int handle, int &sensor_state, int &sensor_option);
	bool set_sensor_state(int handle, int sensor_state);
	bool set_sensor_option(int handle, int sensor_option);
	bool set_event_batch(int handle, unsigned int event_type, unsigned int interval, unsigned int latency);
	bool set_event_maincontext(int handle, unsigned int event_type, GMainContext *maincontext);
	bool set_accuracy(int handle, int accuracy);
	bool set_bad_accuracy(int handle, int bad_accuracy);
	bool get_event_info(int handle, unsigned int event_type, unsigned int &interval, unsigned int &latency, int &cb_type, void* &cb, void* &user_data);
	void get_listening_sensors(sensor_id_vector &sensors);
	void get_sensor_rep(sensor_id_t sensor, sensor_rep& rep);

	bool get_active_batch(sensor_id_t sensor, unsigned int &interval, unsigned int &latency);
	unsigned int get_active_option(sensor_id_t sensor_id);
	void get_active_event_types(sensor_id_t sensor_id, event_type_vector &active_event_types);

	bool get_sensor_id(int handle, sensor_id_t &sensor_id);
	bool get_sensor_state(int handle, int &state);
	bool get_sensor_wakeup(int handle, int &sensor_wakeup);
	bool set_sensor_wakeup(int handle, int sensor_wakeup);

	bool has_client_id(void);
	int get_client_id(void);
	void set_client_id(int client_id);

	bool is_active(void);
	bool is_sensor_registered(sensor_id_t sensor_id);
	bool is_sensor_active(sensor_id_t sensor_id);
	bool is_event_active(int handle, unsigned int event_type, unsigned long long event_id);

	bool add_command_channel(sensor_id_t sensor_id, command_channel *cmd_channel);
	bool get_command_channel(sensor_id_t sensor_id, command_channel **cmd_channel);
	bool close_command_channel(void);
	bool close_command_channel(sensor_id_t sensor_id);

	void get_all_handles(handle_vector &handles);
	void get_sensor_handle_info(sensor_id_t sensor, sensor_handle_info_map &handles_info);
	void get_all_handle_info(sensor_handle_info_map &handles_info);

	void clear(void);

	csensor_client_info();
	~csensor_client_info();

private:
	sensor_handle_info_map m_sensor_handle_infos;
	sensor_command_channel_map m_command_channels;

	int m_client_id;

	cmutex m_handle_info_lock;
};
#endif /* CSENSOR_CLIENT_INFO_H_ */
