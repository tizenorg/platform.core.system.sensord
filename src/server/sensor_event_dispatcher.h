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

#ifndef _SENSOR_EVENT_DISPATCHER_H_
#define _SENSOR_EVENT_DISPATCHER_H_

#include <sf_common.h>
#include <sensor_event_queue.h>
#include <client_info_manager.h>
#include <csocket.h>
#include <virtual_sensor.h>
#include <unordered_map>
#include <list>
#include <vector>


typedef std::unordered_map<unsigned int, sensor_event_t> event_type_last_event_map;
typedef std::list<virtual_sensor *> virtual_sensors;

class sensor_event_dispatcher {
private:
	bool m_lcd_on;
	csocket m_accept_socket;
	cmutex m_mutex;
	cmutex m_last_events_mutex;
	event_type_last_event_map m_last_events;
	virtual_sensors m_active_virtual_sensors;
	cmutex m_active_virtual_sensors_mutex;

	sensor_event_dispatcher();
	~sensor_event_dispatcher();
	sensor_event_dispatcher(sensor_event_dispatcher const&) {};
	sensor_event_dispatcher& operator=(sensor_event_dispatcher const&);

	void accept_connections(void);
	void accept_event_channel(csocket client_socket);

	void dispatch_event(void);
	void send_sensor_events(std::vector<void *> &events);
	static client_info_manager& get_client_info_manager(void);
	static sensor_event_queue& get_event_queue(void);

	bool is_record_event(unsigned int event_type);
	void put_last_event(unsigned int event_type, const sensor_event_t &event);
	bool get_last_event(unsigned int event_type, sensor_event_t &event);

	bool has_active_virtual_sensor(virtual_sensor *sensor);
	virtual_sensors get_active_virtual_sensors(void);

	void sort_sensor_events(std::vector<void *> &events);
public:
	static sensor_event_dispatcher& get_instance();
	bool run(void);
	void request_last_event(int client_id, sensor_id_t sensor_id);

	bool add_active_virtual_sensor(virtual_sensor *sensor);
	bool delete_active_virtual_sensor(virtual_sensor *sensor);
};

#endif /* _SENSOR_EVENT_DISPATCHER_H_ */
