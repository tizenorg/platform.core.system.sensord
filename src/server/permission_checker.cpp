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

#include <cynara-client.h>
#include <cynara-creds-socket.h>
#include <cynara-session.h>
#include <permission_checker.h>
#include <sensor_log.h>
#include <sensor_loader.h>
#include <sensor_base.h>

static cynara *cynara_env = NULL;

static bool check_privilege_by_sockfd(int sock_fd, const char *priv)
{
	retvm_if(cynara_env == NULL, false, "Cynara not initialized");

	int ret;
	int pid = -1;
	char *client = NULL;
	char *session = NULL;
	char *user = NULL;

	retvm_if(cynara_creds_socket_get_pid(sock_fd, &pid) != CYNARA_API_SUCCESS, false, "Getting PID failed");

	if (cynara_creds_socket_get_client(sock_fd, CLIENT_METHOD_DEFAULT, &client) != CYNARA_API_SUCCESS ||
			cynara_creds_socket_get_user(sock_fd, USER_METHOD_DEFAULT, &user) != CYNARA_API_SUCCESS ||
			(session = cynara_session_from_pid(pid)) == NULL) {
		_E("Getting client info failed");
		free(client);
		free(user);
		free(session);
		return false;
	}

	ret = cynara_check(cynara_env, client, session, user, priv);

	free(client);
	free(session);
	free(user);

	return (ret == CYNARA_API_ACCESS_ALLOWED);
}

permission_checker::permission_checker()
: m_permission_set(0)
{
	init();
}

permission_checker::~permission_checker()
{
	deinit();
}

permission_checker& permission_checker::get_instance()
{
	static permission_checker inst;
	return inst;
}

void permission_checker::init()
{
	m_permission_infos.push_back(std::make_shared<permission_info> (SENSOR_PERMISSION_STANDARD, false, ""));
	m_permission_infos.push_back(std::make_shared<permission_info> (SENSOR_PERMISSION_BIO, true, "http://tizen.org/privilege/healthinfo"));

	std::vector<sensor_base *> sensors;
	sensors = sensor_loader::get_instance().get_sensors(ALL_SENSOR);

	for (unsigned int i = 0; i < sensors.size(); ++i)
		m_permission_set |= sensors[i]->get_permission();

	_I("Permission Set = %d", m_permission_set);

	if (cynara_initialize(&cynara_env, NULL) != CYNARA_API_SUCCESS) {
		cynara_env = NULL;
		_E("Cynara initialization failed");
	}
}

void permission_checker::deinit()
{
	if (cynara_env)
		cynara_finish(cynara_env);

	cynara_env = NULL;
}

int permission_checker::get_permission(int sock_fd)
{
	int permission = SENSOR_PERMISSION_NONE;

	for (unsigned int i = 0; i < m_permission_infos.size(); ++i) {
		if (!m_permission_infos[i]->need_to_check) {
			permission |= m_permission_infos[i]->permission;
		} else if (m_permission_set & m_permission_infos[i]->permission) {
			if (check_privilege_by_sockfd(sock_fd, m_permission_infos[i]->privilege.c_str())) {
				permission |= m_permission_infos[i]->permission;
			}
		}
	}

	return permission;
}
