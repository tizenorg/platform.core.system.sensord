/*
 * libsensord-share
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <vector>
#include <sensor_base.h>
#include <physical_sensor.h>
#include <sensor_event_poller.h>
#include <sensor_plugin_loader.h>

#define EPOLL_MAX_FD 32

sensor_event_poller::sensor_event_poller()
{
	init_fd();
	init_sensor_map();
}

sensor_event_poller::~sensor_event_poller()
{
}

void sensor_event_poller::init_fd()
{
	std::vector<sensor_hal *> hals;
	hals = sensor_plugin_loader::get_instance().get_sensor_hals();

	auto it_sensor = hals.begin();

	while (it_sensor != hals.end()) {
		int fd = (*it_sensor)->get_poll_fd();

		add_poll_fd(fd);

		++it_sensor;
	}
}

void sensor_event_poller::init_sensor_map()
{
	int fd;
	physical_sensor *sensor;

	std::vector<sensor_base *> sensors;
	sensors = sensor_plugin_loader::get_instance().get_sensors(ALL_SENSOR);

	auto it_sensor = sensors.begin();

	while (it_sensor != sensors.end()) {
		if ((*it_sensor)->is_virtual()) {
			++it_sensor;
			continue;
		}

		sensor = reinterpret_cast<physical_sensor *>(*it_sensor);
		fd = sensor->get_poll_fd();

		m_fd_sensors.insert(std::make_pair(fd, sensor));
		++it_sensor;
	}
}

bool sensor_event_poller::add_poll_fd(int fd)
{
	m_poller.add_fd(fd);
	return true;
}

bool sensor_event_poller::poll()
{
	while (true) {
		int fd;
		struct epoll_event poll_event;

		if (!m_poller.poll(poll_event))
			continue;

		fd = poll_event.data.fd;

		if (!ready_event(fd))
			continue;

		process_event(fd);
	}

	return true;
}

bool sensor_event_poller::ready_event(int fd)
{
	sensor_hal *hal;
	
	hal = sensor_plugin_loader::get_instance().get_sensor_hals(fd);

	if (!hal) {
		ERR("Failed to get hal plugin");
		return false;
	}

	if (!hal->is_data_ready())
		return false;

	return true;
}

bool sensor_event_poller::process_event(int fd)
{
	physical_sensor *sensor;
	std::pair<fd_sensor_plugins::iterator, fd_sensor_plugins::iterator> ret;

	ret = m_fd_sensors.equal_range(fd);

	for (auto it_sensor = ret.first; it_sensor != ret.second; ++it_sensor) {
		sensor_event_t event;
		
		sensor = it_sensor->second;

		sensor->get_sensor_data(event.data);
		event.sensor_id = sensor->get_id();
		event.event_type = sensor->get_event_type();

		sensor->push(event);
	}

	return true;
}
