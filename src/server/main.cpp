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

#include <signal.h>
#include <sensor_logs.h>
#include <server.h>
#include <dbus_util.h>
#include <sensor_plugin_loader.h>
#include <string>

using std::string;

static void sig_term_handler(int signo, siginfo_t *info, void *data)
{
	char proc_name[NAME_MAX];

	get_proc_name(info->si_pid, proc_name);

	ERR("Received SIGTERM(%d) from %s(%d)\n", signo, proc_name, info->si_pid);
	exit(EXIT_SUCCESS);
}

static void signal_init(void)
{
	struct sigaction sig_act;
	memset(&sig_act, 0, sizeof(struct sigaction));

	sig_act.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sig_act, NULL);
	sigaction(SIGPIPE, &sig_act, NULL);

	sig_act.sa_handler = NULL;
	sig_act.sa_sigaction = sig_term_handler;
	sig_act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &sig_act, NULL);
}

int main(int argc, char *argv[])
{
	INFO("Sensord started");

	signal_init();

	sensor_plugin_loader::get_instance().load_plugins();

	server::get_instance().run();

	server::get_instance().stop();

	INFO("Sensord terminated");
	return 0;
}
