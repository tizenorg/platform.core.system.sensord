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

#include <csensor_event_dispatcher.h>
#include <sensor_plugin_loader.h>
#include <common.h>
#include <sf_common.h>
#include <vconf.h>
#include <thread>

using std::thread;

#define MAX_PENDING_CONNECTION 32

csensor_event_dispatcher::csensor_event_dispatcher()
: m_lcd_on(true)
{
	m_sensor_fusion = sensor_plugin_loader::get_instance().get_fusion();
}

csensor_event_dispatcher::~csensor_event_dispatcher() { }

bool csensor_event_dispatcher::run(void)
{
	INFO("Starting Event Dispatcher");

	if (!m_accept_socket.create(SOCK_SEQPACKET)) {
		ERR("Listener Socket Creation failed in Server");
		return false;
	}

	if (!m_accept_socket.bind(EVENT_CHANNEL_PATH)) {
		ERR("Listener Socket Binding failed in Server");
		m_accept_socket.close();
		return false;
	}

	if (!m_accept_socket.listen(MAX_PENDING_CONNECTION)) {
		ERR("Socket Listen failed in Server");
		return false;
	}

	thread accepter(&csensor_event_dispatcher::accept_connections, this);
	accepter.detach();

	thread dispatcher(&csensor_event_dispatcher::dispatch_event, this);
	dispatcher.detach();

	return true;
}

void csensor_event_dispatcher::accept_event_channel(csocket client_socket)
{
	int client_id;
	event_channel_ready_t event_channel_ready;
	cclient_info_manager &client_info_manager = get_client_info_manager();
	client_socket.set_connection_mode();

	if (client_socket.recv(&client_id, sizeof(client_id)) <= 0) {
		ERR("Failed to receive client id on socket fd[%d]", client_socket.get_socket_fd());
		return;
	}

	client_socket.set_transfer_mode();
	AUTOLOCK(m_mutex);

	if (!get_client_info_manager().set_event_socket(client_id, client_socket)) {
		ERR("Failed to store event socket[%d] for %s", client_socket.get_socket_fd(),
			client_info_manager.get_client_info(client_id));
		return;
	}

	event_channel_ready.magic = EVENT_CHANNEL_MAGIC;
	event_channel_ready.client_id = client_id;

	INFO("Event channel is accepted for %s on socket[%d]",
		 client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());

	if (client_socket.send(&event_channel_ready, sizeof(event_channel_ready)) <= 0) {
		ERR("Failed to send event_channel_ready packet to %s on socket fd[%d]",
			client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
		return;
	}
}

void csensor_event_dispatcher::accept_connections(void)
{
	INFO("Event channel acceptor is started.");

	while (true) {
		csocket client_socket;

		if (!m_accept_socket.accept(client_socket)) {
			ERR("Accepting socket failed in Server");
			continue;
		}

		INFO("New client connected (socket_fd : %d)", client_socket.get_socket_fd());
		thread event_channel_creator(&csensor_event_dispatcher::accept_event_channel, this, client_socket);
		event_channel_creator.detach();
	}
}

void csensor_event_dispatcher::dispatch_event(void)
{
	const int MAX_EVENT_PER_SENSOR = 16;
	const int MAX_SENSOR_EVENT = 1 + (sensor_plugin_loader::get_instance().get_virtual_sensors().size()
									* MAX_EVENT_PER_SENSOR);
	const int MAX_SYNTH_PER_SENSOR = 5;
	INFO("Event Dispatcher started");
	m_lcd_on = is_lcd_on();

	if (vconf_notify_key_changed(VCONFKEY_PM_STATE, situation_watcher, this) != 0)
		ERR("Fail to set notify callback for %s", VCONFKEY_PM_STATE);

	while (true) {
		bool is_hub_event = false;
		event_situation situation;
		void *seed_event = get_event_queue().pop();
		unsigned int event_type = *((unsigned int *)(seed_event));

		if (is_sensorhub_event(event_type))
			is_hub_event = true;

		if (m_lcd_on)
			situation = SITUATION_LCD_ON;
		else
			situation = SITUATION_LCD_OFF;

		if (is_hub_event) {
			sensorhub_event_t *sensorhub_event = (sensorhub_event_t *)seed_event;
			sensorhub_event->situation = situation;
			send_sensor_events(sensorhub_event, 1, true, situation);
		} else {
			sensor_event_t sensor_events[MAX_SENSOR_EVENT];
			unsigned int event_cnt = 0;
			sensor_events[event_cnt++] = *((sensor_event_t *)seed_event);

			virtual_sensors v_sensors = get_active_virtual_sensors();
			virtual_sensors::iterator it_v_sensor;
			it_v_sensor = v_sensors.begin();
			vector<sensor_event_t> v_sensor_events;
			v_sensor_events.reserve(MAX_SYNTH_PER_SENSOR);

			while (it_v_sensor != v_sensors.end()) {
				int synthesized_cnt;
				v_sensor_events.clear();
				(*it_v_sensor)->synthesize(*((sensor_event_t *)seed_event), v_sensor_events);
				synthesized_cnt = v_sensor_events.size();

				for (int i = 0; i < synthesized_cnt; ++i)
					sensor_events[event_cnt++] = v_sensor_events[i];

				++it_v_sensor;
			}

			sort_sensor_events(sensor_events, event_cnt);

			for (int i = 0; i < event_cnt; ++i) {
				sensor_events[i].situation = situation;

				if (is_record_event(sensor_events[i].event_type))
					put_last_event(sensor_events[i].event_type, sensor_events[i]);
			}

			send_sensor_events(sensor_events, event_cnt, false, situation);
		}

		if (is_hub_event)
			delete (sensorhub_event_t *)seed_event;
		else
			delete (sensor_event_t *)seed_event;
	}
}

void csensor_event_dispatcher::send_sensor_events(void *events, int event_cnt, bool is_hub_event, event_situation situation)
{
	sensor_event_t *sensor_events;
	sensorhub_event_t *sensor_hub_events;
	cclient_info_manager &client_info_manager = get_client_info_manager();

	if (is_hub_event)
		sensor_hub_events = (sensorhub_event_t *)events;
	else
		sensor_events = (sensor_event_t *)events;

	for (int i = 0; i < event_cnt; ++i) {
		client_id_vec id_vec;
		unsigned int event_type;

		if (is_hub_event)
			event_type = sensor_hub_events[i].event_type;
		else
			event_type = sensor_events[i].event_type;

		client_info_manager.get_listener_ids(event_type, situation, id_vec);
		client_id_vec::iterator it_client_id;
		it_client_id = id_vec.begin();

		while (it_client_id != id_vec.end()) {
			csocket client_socket;
			client_info_manager.get_event_socket(*it_client_id, client_socket);
			bool ret;

			if (is_hub_event)
				ret = (client_socket.send(sensor_hub_events + i, sizeof(sensorhub_event_t)) > 0);
			else
				ret = (client_socket.send(sensor_events + i, sizeof(sensor_event_t)) > 0);

			if (ret)
				DBG("Event[0x%x] sent to %s on socket[%d]", event_type, client_info_manager.get_client_info(*it_client_id), client_socket.get_socket_fd());
			else
				ERR("Failed to send event[0x%x] to %s on socket[%d]", event_type, client_info_manager.get_client_info(*it_client_id), client_socket.get_socket_fd());

			++it_client_id;
		}
	}
}

bool csensor_event_dispatcher::is_lcd_on(void)
{
	int lcd_state;

	if (vconf_get_int(VCONFKEY_PM_STATE, &lcd_state) != 0) {
		ERR("Can't get the value of VCONFKEY_PM_STATE");
		return true;
	}

	if (lcd_state == VCONFKEY_PM_STATE_LCDOFF)
		return false;

	return true;
}

cclient_info_manager &csensor_event_dispatcher::get_client_info_manager(void)
{
	return cclient_info_manager::get_instance();
}

csensor_event_queue &csensor_event_dispatcher::get_event_queue(void)
{
	return csensor_event_queue::get_instance();
}

bool csensor_event_dispatcher::is_record_event(unsigned int event_type)
{
	switch (event_type) {
	case ACCELEROMETER_EVENT_ROTATION_CHECK:
		return true;
		break;
	default:
		break;
	}

	return false;
}

void csensor_event_dispatcher::put_last_event(unsigned int event_type, const sensor_event_t &event)
{
	AUTOLOCK(m_last_events_mutex);
	m_last_events[event_type] = event;
}

bool csensor_event_dispatcher::get_last_event(unsigned int event_type, sensor_event_t &event)
{
	AUTOLOCK(m_last_events_mutex);
	event_type_last_event_map::iterator it_event;
	it_event = m_last_events.find(event_type);

	if (it_event == m_last_events.end())
		return false;

	event = it_event->second;
	return true;
}

bool csensor_event_dispatcher::has_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);
	virtual_sensors::iterator it_v_sensor;
	it_v_sensor = find(m_active_virtual_sensors.begin(), m_active_virtual_sensors.end(), sensor);
	return (it_v_sensor != m_active_virtual_sensors.end());
}

virtual_sensors csensor_event_dispatcher::get_active_virtual_sensors(void)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);
	return m_active_virtual_sensors;
}

bool csensor_event_dispatcher::compare_by_timestamp(const sensor_event_t &a, const sensor_event_t &b)
{
	return a.data.timestamp < b.data.timestamp;
}

void csensor_event_dispatcher::sort_sensor_events(sensor_event_t *events, unsigned int cnt)
{
	std::sort(events, events + cnt, compare_by_timestamp);
}

void csensor_event_dispatcher::request_last_event(int client_id, const sensor_type_t sensor)
{
	cclient_info_manager &client_info_manager = get_client_info_manager();
	event_type_vector event_vec;
	csocket client_socket;

	if (client_info_manager.get_registered_events(client_id, sensor, event_vec)) {
		client_info_manager.get_event_socket(client_id, client_socket);
		event_type_vector::iterator it_event;
		it_event = event_vec.begin();

		while (it_event != event_vec.end()) {
			sensor_event_t event;

			if (is_record_event(*it_event) && get_last_event(*it_event, event)) {
				if (client_socket.send(&event, sizeof(event)) > 0)
					INFO("Send the last event[0x%x] to %s on socket[%d]", event.event_type,
						client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
				else
					ERR("Failed to send event[0x%x] to %s on socket[%d]", event.event_type,
						client_info_manager.get_client_info(client_id), client_socket.get_socket_fd());
			}

			++it_event;
		}
	}
}

bool csensor_event_dispatcher::add_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);

	if (has_active_virtual_sensor(sensor)) {
		ERR("[%s] sensor is already added on active virtual sensors", sensor->get_name());
		return false;
	}

	m_active_virtual_sensors.push_back(sensor);
	return true;
}

bool csensor_event_dispatcher::delete_active_virtual_sensor(virtual_sensor *sensor)
{
	AUTOLOCK(m_active_virtual_sensors_mutex);
	virtual_sensors::iterator it_v_sensor;
	it_v_sensor = find(m_active_virtual_sensors.begin(), m_active_virtual_sensors.end(), sensor);

	if (it_v_sensor == m_active_virtual_sensors.end()) {
		ERR("Fail to delete non-existent [%s] sensor on active virtual sensors", sensor->get_name());
		return false;
	}

	m_active_virtual_sensors.erase(it_v_sensor);
	return true;
}

void csensor_event_dispatcher::situation_watcher(keynode_t *node, void *user_data)
{
	csensor_event_dispatcher *dispatcher = (csensor_event_dispatcher *) user_data;

	if (!strcmp(vconf_keynode_get_name(node), VCONFKEY_PM_STATE))
		dispatcher->m_lcd_on = dispatcher->is_lcd_on();
}
