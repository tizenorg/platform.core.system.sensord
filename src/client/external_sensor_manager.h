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
#ifndef _EXTERNAL_SENSOR_MANAGER_H_
#define _EXTERNAL_SENSOR_MANAGER_H_

#include <glib.h>
#include <sys/epoll.h>
#include <sensor_common.h>
#include <csocket.h>
#include <cmutex.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>

class external_data_channel;
class poller;

class sensor_ext_handle_info {
public:
	int m_handle;
	sensor_id_t m_sensor;
	std::string m_key;
	void *m_cb;
	void *m_user_data;
};

typedef struct {
	int handle;
	sensor_id_t sensor;
	char *data;
	int data_cnt;
	void *cb;
	void *user_data;
} command_cb_info;

class external_sensor_manager {
public:
	typedef void (*hup_observer_t)(void);

	static external_sensor_manager& get_instance(void);
	int create_handle(void);
	bool delete_handle(int handle);

	bool set_handle(int handle, sensor_id_t sensor, const std::string& key, void* cb, void* user_data);

	bool get_sensor(int handle, sensor_id_t &sensor_id);
	int get_handle(sensor_id_t sensor);
	bool get_handle_info(int handle, const sensor_ext_handle_info*& handle_info);
	std::string get_key(int handle);

	bool has_client_id(void);
	int get_client_id(void);
	void set_client_id(int client_id);

	bool add_data_channel(int handle, external_data_channel *channel);
	bool get_data_channel(int handle, external_data_channel **channel);
	bool close_data_channel(void);
	bool close_data_channel(int handle);

	bool is_valid(int handle);
	bool is_active(void);

	void get_all_handles(std::vector<int> &handles);

	bool start_command_listener(void);
	void stop_command_listener(void);
	void clear(void);

	void set_hup_observer(hup_observer_t observer);
private:
	enum thread_state {
		THREAD_STATE_START,
		THREAD_STATE_STOP,
		THREAD_STATE_TERMINATE,
	};

	typedef std::lock_guard<std::mutex> lock;
	typedef std::unique_lock<std::mutex> ulock;

	external_sensor_manager();
	~external_sensor_manager();

	external_sensor_manager(const external_sensor_manager&) {};
	external_sensor_manager& operator=(const external_sensor_manager&);

	bool create_command_channel(void);
	void close_command_channel(void);
	void set_thread_state(thread_state state);

	bool sensor_command_poll(void* buffer, int buffer_len, struct epoll_event &event);

	bool get_cb_info(sensor_id_t sensor, char* data, int data_cnt, command_cb_info &cb_info);
	void post_callback_to_main_loop(command_cb_info* cb_info);
	void handle_command(sensor_id_t sensor, char* data, int data_cnt);
	void listen_command(void);

	bool is_valid_callback(const command_cb_info *cb_info);
	static gboolean callback_dispatcher(gpointer data);

	int m_client_id;

	csocket m_command_socket;
	poller *m_poller;

	cmutex m_handle_info_lock;

	thread_state m_thread_state;
	std::mutex m_thread_mutex;
	std::condition_variable m_thread_cond;

	hup_observer_t m_hup_observer;

	std::unordered_map<int, sensor_ext_handle_info> m_sensor_handle_infos;
	std::unordered_map<sensor_id_t, int> m_sensor_handle_map;
	std::unordered_map<int, external_data_channel*> m_data_channels;
};
#endif /* _EXTERNAL_SENSOR_MANAGER_H_ */
