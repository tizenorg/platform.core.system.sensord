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
#include <errno.h>
#include <string.h>
#include <sensor_log.h>
#include <poller.h>

#define EPOLL_MAX 32

poller::poller()
{
	init_poll_fd();
}

poller::poller(int fd)
{
	init_poll_fd();
	add_fd(fd);
}

poller::~poller()
{
	if (m_epfd)
		close(m_epfd);
}

void poller::init_poll_fd(void)
{
	m_epfd = epoll_create(EPOLL_MAX);
}

bool poller::add_fd(int fd)
{
	struct epoll_event event;

	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLERR | EPOLLHUP;

	if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event)) {
		_E("errno : %d , errstr : %s", errno, strerror(errno));
		return false;
	}

	return true;
}

bool poller::fill_event_queue(void)
{
	const int EPOLL_MAX_EVENT = 1;

	struct epoll_event event_items[EPOLL_MAX_EVENT];
	int nr_events = epoll_wait(m_epfd, event_items, EPOLL_MAX_EVENT, -1);

	if (nr_events < 0) {
		if (errno == EINTR)
			return true;

		_E("Epoll failed errrno : %d , errstr : %s", errno, strerror(errno));
		return false;
	}

	if (nr_events == 0) {
		_E("Epoll timeout!");
		return false;
	}

    for (int i = 0; i < nr_events; i++)
		m_event_queue.push(event_items[i]);

	return true;
}


bool poller::poll(struct epoll_event &event)
{
	while (true) {
		if (m_event_queue.empty()) {
			if (!fill_event_queue())
				return false;
		}

		if (!m_event_queue.empty()) {
			event = m_event_queue.front();
			m_event_queue.pop();

			if (event.events & EPOLLERR) {
				_D("Poll error!");
				return false;
			}

			if (event.events & EPOLLHUP) {
				_I("Poll: Connetion is closed from the other side");
				return false;
			}

			return true;
		}
	}
}
