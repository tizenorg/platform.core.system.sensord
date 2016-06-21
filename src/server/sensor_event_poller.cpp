/*
 * sensord
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

#include <signal.h>
#include <sys/signalfd.h>
#include <sensor_base.h>
#include <physical_sensor.h>
#include <sensor_event_poller.h>
#include <sensor_loader.h>
#include <algorithm>
#include <vector>

sensor_event_poller::sensor_event_poller()
{
	init_sensor_map();
	init_fd();
	init_signal_fd();
}

sensor_event_poller::~sensor_event_poller()
{
	fd_sensors_t::iterator it;
	for (it = m_fd_sensors.begin(); it != m_fd_sensors.end(); it = m_fd_sensors.upper_bound(it->first))
		m_poller.del_fd(it->first);
}

void sensor_event_poller::init_sensor_map(void)
{
	int fd;
	physical_sensor *sensor;

	std::vector<sensor_base *> sensors;
	sensors = sensor_loader::get_instance().get_sensors(ALL_SENSOR);

	std::vector<sensor_base *>::iterator it;

	for (it = sensors.begin(); it != sensors.end(); ++it) {
		sensor = dynamic_cast<physical_sensor *>(*it);
		if (sensor == NULL)
			continue;

		fd = sensor->get_poll_fd();

		if (fd < 0)
			continue;

		m_fd_sensors.insert(std::make_pair(fd, sensor));
	}
}

void sensor_event_poller::init_fd(void)
{
	fd_sensors_t::iterator it;
	for (it = m_fd_sensors.begin(); it != m_fd_sensors.end(); it = m_fd_sensors.upper_bound(it->first)) {
		/* if fd is not valid, it is not added to poller */
		add_poll_fd(it->first);
	}
}

void sensor_event_poller::init_signal_fd(void)
{
	int sfd;
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGABRT);
	sigaddset(&mask, SIGINT);

	sfd = signalfd(-1, &mask, 0);
	m_poller.add_signal_fd(sfd);
}

bool sensor_event_poller::add_poll_fd(int fd)
{
	return m_poller.add_fd(fd);
}

bool sensor_event_poller::poll(void)
{
	std::vector<uint32_t> ids;
	while (true) {
		int fd;
		struct epoll_event poll_event;

		if (!m_poller.poll(poll_event))
			return false;

		fd = poll_event.data.fd;
		ids.clear();

		if (!read_fd(fd, ids))
			continue;

		if (!process_event(fd, ids))
			continue;
	}
}

bool sensor_event_poller::read_fd(int fd, std::vector<uint32_t> &ids)
{
	fd_sensors_t::iterator it;
	physical_sensor *sensor;

	it = m_fd_sensors.find(fd);
	sensor = dynamic_cast<physical_sensor *>(it->second);

	if (!sensor) {
		_E("Failed to get sensor");
		return false;
	}

	if (!sensor->read_fd(ids))
		return false;

	return true;
}

bool sensor_event_poller::process_event(int fd, const std::vector<uint32_t> &ids)
{
	physical_sensor *sensor;
	std::pair<fd_sensors_t::iterator, fd_sensors_t::iterator> ret;

	/* find sensors which is based on same device(fd) */
	ret = m_fd_sensors.equal_range(fd);

	for (auto it_sensor = ret.first; it_sensor != ret.second; ++it_sensor) {
		sensor_event_t *event;
		sensor_data_t *data;
		int data_length;
		int remains = 1;

		sensor = it_sensor->second;

		/* check whether the id of this sensor is in id list(parameter) or not */
		auto result = std::find(std::begin(ids), std::end(ids), sensor->get_hal_id());

		if (result == std::end(ids))
			continue;

		while (remains > 0) {
			remains = sensor->get_data(&data, &data_length);
			if (remains < 0) {
				_E("Failed to get sensor data");
				break;
			}

			if (!sensor->on_event(data, data_length, remains)) {
				free(data);
				continue;
			}

			event = (sensor_event_t *)malloc(sizeof(sensor_event_t));
			if (!event) {
				_E("Memory allocation failed");
				break;
			}

			event->sensor_id = sensor->get_id();
			event->event_type = sensor->get_event_type();
			event->data_length = data_length;
			event->data = data;

			if (!sensor->push(event)) {
				free(event);
				free(data);
			}
		}
	}

	return true;
}
