/*
 * sensord
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

#include <sys/epoll.h>
#include <sys/socket.h>

#include <systemd/sd-daemon.h>
#include <server.h>
#include <sensor_loader.h>
#include <command_common.h>
#include <command_worker.h>
#include <thread>
#include <sensor_event_poller.h>
#include <client_info_manager.h>

#define SYSTEMD_SOCKET_MAX    2

using std::thread;

server::server()
: m_mainloop(NULL)
, m_running(false)
{
}

server::~server()
{
}

int server::get_systemd_socket(const char *name)
{
	int type = SOCK_STREAM;
	int listening = 1;
	size_t length = 0;
	int fd = -1;
	int fd_env = 1;
	int fd_index;

	if (!strcmp(name, EVENT_CHANNEL_PATH)) {
		type = SOCK_SEQPACKET;
		listening = -1;
		fd_env = 0;
	}

	if (sd_listen_fds(fd_env) < 0) {
		_E("Failed to listen fds from systemd");
		return -1;
	}

	for (fd_index = 0; fd_index < SYSTEMD_SOCKET_MAX; ++fd_index) {
		fd = SD_LISTEN_FDS_START + fd_index;

		if (sd_is_socket_unix(fd, type, listening, name, length) > 0)
			return fd;
	}

	return -1;
}

void server::accept_command_channel(void)
{
	command_worker *cmd_worker;

	_I("Command channel acceptor is started");

	while (m_running) {
		csocket client_command_socket;

		if (!m_command_channel_accept_socket.is_valid()) {
			_E("Failed to accept, event_channel_accept_socket is closed");
			break;
		}

		if (!m_command_channel_accept_socket.accept(client_command_socket)) {
			_E("Failed to accept command channel from a client");
			continue;
		}

		if (!m_running) {
			_E("server die");
			break;
		}

		_D("New client (socket_fd : %d) connected", client_command_socket.get_socket_fd());

		/* TODO: if socket is closed, it should be erased */
		client_command_sockets.push_back(client_command_socket);

		cmd_worker = new(std::nothrow) command_worker(client_command_socket);

		if (!cmd_worker) {
			_E("Failed to allocate memory");
			break;
		}

		if (!cmd_worker->start())
			delete cmd_worker;
	}

	_I("Command channel acceptor is terminated");
}

void server::accept_event_channel(void)
{
	_I("Event channel acceptor is started!");

	while (m_running) {
		csocket client_event_socket;

		if (!m_event_channel_accept_socket.is_valid()) {
			_E("Failed to accept, event_channel_accept_socket is closed");
			break;
		}

		if (!m_event_channel_accept_socket.accept(client_event_socket)) {
			_E("Failed to accept event channel from a client");
			continue;
		}

		if (!m_running) {
			_E("server die");
			break;
		}

		/* TODO: if socket is closed, it should be erased */
		client_event_sockets.push_back(client_event_socket);

		_D("New client(socket_fd : %d) connected", client_event_socket.get_socket_fd());

		sensor_event_dispatcher::get_instance().accept_event_connections(client_event_socket);
	}

	_I("Event channel acceptor is terminated");
}

void server::poll_event(void)
{
	_I("Event poller is started");

	sensor_event_poller poller;

	if (!poller.poll()) {
		_E("Failed to poll event");
		return;
	}
}

bool server::listen_command_channel(void)
{
	int sock_fd = -1;
	const int MAX_PENDING_CONNECTION = 10;

	sock_fd = get_systemd_socket(COMMAND_CHANNEL_PATH);

	if (sock_fd >= 0) {
		_I("Succeeded to get systemd socket(%d)", sock_fd);
		m_command_channel_accept_socket = csocket(sock_fd);
		return true;
	}

	INFO("Failed to get systemd socket, create it by myself!");
	if (!m_command_channel_accept_socket.create(SOCK_STREAM)) {
		ERR("Failed to create command channel");
		return false;
	}

	if (!m_command_channel_accept_socket.bind(COMMAND_CHANNEL_PATH)) {
		ERR("Failed to bind command channel");
		m_command_channel_accept_socket.close();
		return false;
	}

	if (!m_command_channel_accept_socket.listen(MAX_PENDING_CONNECTION)) {
		ERR("Failed to listen command channel");
		return false;
	}

	return true;
}

bool server::listen_event_channel(void)
{
	int sock_fd = -1;
	const int MAX_PENDING_CONNECTION = 32;

	sock_fd = get_systemd_socket(EVENT_CHANNEL_PATH);

	if (sock_fd >= 0) {
		_I("Succeeded to get systemd socket(%d)", sock_fd);
		m_event_channel_accept_socket = csocket(sock_fd);
		return true;
	}

	INFO("Failed to get systemd socket, create it by myself!");

	if (!m_event_channel_accept_socket.create(SOCK_SEQPACKET)) {
		ERR("Failed to create event channel");
		return false;
	}

	if (!m_event_channel_accept_socket.bind(EVENT_CHANNEL_PATH)) {
		ERR("Failed to bind event channel");
		m_event_channel_accept_socket.close();
		return false;
	}

	if (!m_event_channel_accept_socket.listen(MAX_PENDING_CONNECTION)) {
		ERR("Failed to listen event channel");
		m_event_channel_accept_socket.close();
		return false;
	}

	return true;
}

void server::close_socket(void)
{
	int size;
	m_command_channel_accept_socket.close();
	m_event_channel_accept_socket.close();

	size = client_command_sockets.size();
	for (int i = 0; i < size; ++i)
		client_command_sockets[i].close();

	size = client_event_sockets.size();
	for (int i = 0; i < size; ++i)
		client_event_sockets[i].close();

	client_command_sockets.clear();
	client_event_sockets.clear();
}

void server::initialize(void)
{
	m_running = true;
	m_mainloop = g_main_loop_new(NULL, false);

	sensor_event_dispatcher::get_instance().run();

	listen_command_channel();
	listen_event_channel();

	std::thread event_channel_accepter(&server::accept_event_channel, this);
	event_channel_accepter.detach();

	std::thread command_channel_accepter(&server::accept_command_channel, this);
	command_channel_accepter.detach();

	std::thread event_poller(&server::poll_event, this);
	event_poller.detach();

	sd_notify(0, "READY=1");

	g_main_loop_run(m_mainloop);
}

void server::terminate(void)
{
	sensor_event_dispatcher::get_instance().stop();

	close_socket();
}

void server::run(void)
{
	initialize();
	terminate();
}

void server::stop(void)
{
	_I("Sensord server stopped");

	m_running = false;

	if (m_mainloop) {
		g_main_loop_quit(m_mainloop);
		g_main_loop_unref(m_mainloop);
		m_mainloop = NULL;
	}
}

server& server::get_instance()
{
	static server inst;
	return inst;
}
