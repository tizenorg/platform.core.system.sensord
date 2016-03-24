/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#pragma once // _SENSOR_MANAGER_H_

#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>

namespace sensorctl {

class mainloop {
public:
	static void run(void) { instance().start_loop(); }
	static void stop(void) { instance().stop_loop(); }
	static bool is_running(void) { return instance().is_loop_running(); }

private:
	void start_loop(void);
	void stop_loop(void);
	void is_loop_running(void);

private:
	static mainloop& instance();
	GMainLoop *m_mainloop;
	bool m_running;
}

class sensor_manager {
public:
	virtual void run(int argc, char *argv[]);

private:
	sensor_type_t get_sensor_type(const char *name);
	const char *get_sensor_name(sensor_type_t type);

	bool is_number(const char *value);
	bool is_hex(const char *value);

	void usage_sensors(void);
};

} /* namespace sensorctl */
