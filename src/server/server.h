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

#ifndef _SERVER_H_
#define _SERVER_H_

#include <glib.h>
#include <csocket.h>
#include <vector>
#include <thread>

class server {
public:
	static server& get_instance(void);

public:
	void run(void);
	void stop(void);

private:
	GMainLoop *m_mainloop;
	csocket m_command_channel_accept_socket;
	csocket m_event_channel_accept_socket;

	std::vector<csocket> client_command_sockets;
	std::vector<csocket> client_event_sockets;

	bool m_running;

private:
	server();
	virtual ~server();

	void initialize(void);
	void terminate(void);

	void poll_event(void);

	bool listen_command_channel(void);
	bool listen_event_channel(void);

	void accept_command_channel(void);
	void accept_event_channel(void);

	void close_socket(void);

	/* TODO: move to socket class */
	int get_systemd_socket(const char *name);
};

#endif /* _SERVER_H_ */
