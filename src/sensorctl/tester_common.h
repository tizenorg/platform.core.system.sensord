/*
 * sensorctl
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

#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>

class tester_common {
public:
	static bool is_running_loop(void);
	static bool run_loop(void);
	static bool exit_loop(void);

	void reset(void);
	void pass(const char *name);
	void fail(const char *name);
	void result(int &pass, int &fail);

	bool is_supported(sensor_type_t type);
	bool scenario_basic_p(sensor_type_t type, int interval, int latency, sensor_cb_t cb, void *user_data);
private:
	static GMainLoop *mainloop;

	int handle;
	sensor_data_t data;

	int m_pass;
	int m_fail;
};
