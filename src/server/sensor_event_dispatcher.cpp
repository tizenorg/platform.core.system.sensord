/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <sensor_common.h>
#include <command_common.h>
#include <sensor_event_dispatcher.h>
#include <sensor_log.h>
#include <thread>

using std::thread;
using std::vector;
using std::pair;

#define MAX_PENDING_CONNECTION 32

sensor_event_dispatcher::sensor_event_dispatcher()
: m_running(false)
{
}

sensor_event_dispatcher::~sensor_event_dispatcher()
{
}

sensor_event_dispatcher& sensor_event_dispatcher::get_instance()
{
	static sensor_event_dispatcher inst;
	return inst;
}

bool sensor_event_dispatcher::run(void)
{
	m_running = true;

	thread dispatcher(&sensor_event_dispatcher::dispatch_event, this);
	dispatcher.detach();

	return true;
}

bool sensor_event_dispatcher::stop(void)
{
	m_running = false;
}

void sensor_event_dispatcher::accept_event_channel(csocket client_socket)
{
	int client_id;
	channel_ready_t event_channel_ready;
	client_info_manager& client_info_manager = get_client_info_manager();

	client_socket.set_connection_mode();

	if (client_socket.recv(&client_id, sizeof(client_id)) <= 0) {
		_E("Failed to receive client id on socket fd[%d]", client_socket.get_socket_fd());
		return;
	}

	client_socket.set_transfer_mode();

	if(!get_client_info_manager().set_event_socket(client_id, client_socket)) {
		_E("Failed to store event socket[%d] for %s", client_socket.get_socket_fd(),
			client_info_manager.get_client_info(client_id));
		return;
	}

	event_channel_ready.magic = CHANNEL_MAGIC_NUM;
	event_channel_ready.client_id = client_id;

	_I("Event channel is accepted for %s on socket[%d]",
		client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());

	if (client_socket.send(&event_channel_ready, sizeof(event_channel_ready)) <= 0) {
		_E("Failed to send event_channel_ready packet to %s on socket fd[%d]",
			client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
		return;
	}
}

void sensor_event_dispatcher::accept_event_connections(csocket client_socket)
{
	thread event_channel_creator(&sensor_event_dispatcher::accept_event_channel, this, client_socket);
	event_channel_creator.detach();
}

void sensor_event_dispatcher::dispatch_event(void)
{
	const int MAX_SYNTH_PER_SENSOR = 5;

	vector<sensor_event_t> v_sensor_events(MAX_SYNTH_PER_SENSOR);

	_I("Event Dispatcher started");

	while (m_running) {
		void *seed_event = get_event_queue().pop();

		vector<void *> sensor_events;
		sensor_events.push_back(seed_event);

		virtual_sensors v_sensors = get_active_virtual_sensors();

		auto it_v_sensor = v_sensors.begin();

		while (it_v_sensor != v_sensors.end()) {
			int synthesized_cnt;
			v_sensor_events.clear();
			(*it_v_sensor)->synthesize(*((sensor_event_t *)seed_event));
			synthesized_cnt = v_sensor_events.size();

			for (int i = 0; i < synthesized_cnt; ++i) {
				sensor_event_t *v_event = (sensor_event_t*)malloc(sizeof(sensor_event_t));
				if (!v_event) {
					_E("Failed to allocate memory");
					continue;
				}

				memcpy(v_event, &v_sensor_events[i], sizeof(sensor_event_t));
				sensor_events.push_back(v_event);
			}

			++it_v_sensor;
		}

		sort_sensor_events(sensor_events);

		for (unsigned int i = 0; i < sensor_events.size(); ++i) {
			if (is_record_event(((sensor_event_t *)(sensor_events[i]))->event_type))
				put_last_event(((sensor_event_t *)(sensor_events[i]))->event_type, *((sensor_event_t *)(sensor_events[i])));
		}

		send_sensor_events(sensor_events);
	}
}

void sensor_event_dispatcher::send_sensor_events(vector<void *> &events)
{
	sensor_event_t *sensor_event = NULL;
	client_info_manager& client_info_manager = get_client_info_manager();

	const int RESERVED_CLIENT_CNT = 20;
	static client_id_vec id_vec(RESERVED_CLIENT_CNT);

	for (unsigned int i = 0; i < events.size(); ++i) {
		sensor_id_t sensor_id;
		unsigned int event_type;

		sensor_event = (sensor_event_t*)events[i];
		sensor_id = sensor_event->sensor_id;
		event_type = sensor_event->event_type;

		id_vec.clear();
		client_info_manager.get_listener_ids(sensor_id, event_type, id_vec);

		auto it_client_id = id_vec.begin();

		while (it_client_id != id_vec.end()) {
			csocket client_socket;
			client_info_manager.get_event_socket(*it_client_id, client_socket);
			bool ret = (client_socket.send(sensor_event, sizeof(sensor_event_t)) > 0);

			ret = (ret & (client_socket.send(sensor_event->data, sensor_event->data_length) > 0));

			if (!ret)
				_E("Failed to send event[%#x] to %s on socket[%d]", event_type, client_info_manager.get_client_info(*it_client_id), client_socket.get_socket_fd());

			++it_client_id;
		}

		free(sensor_event->data);
		free(sensor_event);
	}
}

client_info_manager& sensor_event_dispatcher::get_client_info_manager(void)
{
	return client_info_manager::get_instance();
}

sensor_event_queue& sensor_event_dispatcher::get_event_queue(void)
{
	return sensor_event_queue::get_instance();
}

bool sensor_event_dispatcher::is_record_event(unsigned int event_type)
{
	return false;
}

void sensor_event_dispatcher::put_last_event(unsigned int event_type, const sensor_event_t &event)
{
	AUTOLOCK(m_last_events_mutex);
	m_last_events[event_type] = event;
}

bool sensor_event_dispatcher::get_last_event(unsigned int event_type, sensor_event_t &event)
{
	AUTOLOCK(m_last_events_mutex);

	auto it_event = m_last_events.find(event_type);

	if (it_event == m_last_events.end())
		return false;

	event = it_event->second;
	return true;
}

bool sensor_event_dispatcher::has_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);

	auto it_v_sensor = find(m_active_virtual_sensors.begin(), m_active_virtual_sensors.end(), sensor);

	return (it_v_sensor != m_active_virtual_sensors.end());
}

virtual_sensors sensor_event_dispatcher::get_active_virtual_sensors(void)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);

	return m_active_virtual_sensors;
}

struct sort_comp {
	bool operator()(const void *left, const void *right) {
		return ((sensor_event_t *)left)->data->timestamp < ((sensor_event_t *)right)->data->timestamp;
	}
};

void sensor_event_dispatcher::sort_sensor_events(vector<void *> &events)
{
	std::sort(events.begin(), events.end(), sort_comp());
}

void sensor_event_dispatcher::request_last_event(int client_id, sensor_id_t sensor_id)
{
	client_info_manager& client_info_manager = get_client_info_manager();
	event_type_vector event_vec;
	csocket client_socket;

	if (client_info_manager.get_registered_events(client_id, sensor_id, event_vec)) {
		if (!client_info_manager.get_event_socket(client_id, client_socket)) {
			_E("Failed to get event socket from %s",
					client_info_manager.get_client_info(client_id));
			return;
		}

		auto it_event = event_vec.begin();
		while (it_event != event_vec.end()) {
			sensor_event_t event;
			if (is_record_event(*it_event) && get_last_event(*it_event, event)) {
				if (client_socket.send(&event, sizeof(event)) > 0)
					_I("Send the last event[%#x] to %s on socket[%d]", event.event_type,
						client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
				else
					_E("Failed to send event[%#x] to %s on socket[%d]", event.event_type,
						client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
			}
			++it_event;
		}
	}
}

bool sensor_event_dispatcher::add_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);

	if (has_active_virtual_sensor(sensor)) {
		_E("[%s] sensor is already added on active virtual sensors", sensor->get_name());
		return false;
	}

	m_active_virtual_sensors.push_back(sensor);

	return true;
}

bool sensor_event_dispatcher::delete_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);

	/*
	auto it_v_sensor = find(m_active_virtual_sensors.begin(), m_active_virtual_sensors.end(), sensor);

	if (it_v_sensor == m_active_virtual_sensors.end()) {
		_E("Fail to delete non-existent [%s] sensor on active virtual sensors", sensor->get_name());
		return false;
	}
	*/

	//m_active_virtual_sensors.remove(it_v_sensor);
	if (sensor)
		m_active_virtual_sensors.remove(sensor);

	return true;
}
