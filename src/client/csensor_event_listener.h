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

#ifndef CSENSOR_EVENT_LISTENER_H_
#define CSENSOR_EVENT_LISTENER_H_

#include <glib.h>
#include <sys/types.h>
#include <csensor_handle_info.h>
#include <csensor_client_info.h>
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

typedef std::vector<unsigned int> handle_vector;
typedef std::vector<sensor_id_t> sensor_id_vector;
typedef std::unordered_map<int,csensor_handle_info> sensor_handle_info_map;
typedef std::unordered_map<sensor_id_t, command_channel*> sensor_command_channel_map;

typedef struct {
	unsigned long long event_id;
	int handle;
	sensor_t sensor;
	unsigned int event_type;
	int cb_type;
	void *cb;
	void *sensor_data;
	void *user_data;
	sensor_accuracy_changed_cb_t accuracy_cb;
	unsigned long long timestamp;
	int accuracy;
	void *accuracy_user_data;
	GMainContext *maincontext;
	void *buffer;
} client_callback_info;

typedef void (*hup_observer_t)(void);

class csensor_event_listener {
public:
	static csensor_event_listener& get_instance(void);
	bool start_handle(int handle);
	bool stop_handle(int handle);

	void operate_sensor(sensor_id_t sensor, int power_save_state);
	void get_listening_sensors(sensor_id_vector &sensors);

	bool start_event_listener(void);
	void stop_event_listener(void);
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

	csocket m_event_socket;
	poller *m_poller;

	thread_state m_thread_state;
	std::mutex m_thread_mutex;
	std::condition_variable m_thread_cond;

	hup_observer_t m_hup_observer;

	csensor_event_listener();
	~csensor_event_listener();

	csensor_event_listener(const csensor_event_listener&);
	csensor_event_listener& operator=(const csensor_event_listener&);

	bool create_event_channel(void);
	void close_event_channel(void);

	ssize_t sensor_event_poll(void* buffer, int buffer_len, int &event);

	void listen_events(void);
	client_callback_info* handle_calibration_cb(csensor_handle_info &handle_info, unsigned event_type, unsigned long long time, int accuracy);
	void handle_events(void* event);

	client_callback_info* get_callback_info(sensor_id_t sensor_id, const creg_event_info *event_info, void *sensor_data, void *buffer);

	unsigned long long renew_event_id(void);

	void post_callback_to_main_loop(client_callback_info *cb_info);

	bool is_valid_callback(client_callback_info *cb_info);
	static gboolean callback_dispatcher(gpointer data);

	void set_thread_state(thread_state state);

	csensor_client_info &m_client_info;
};
#endif /* CSENSOR_EVENT_LISTENER_H_ */
