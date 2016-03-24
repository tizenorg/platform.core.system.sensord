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

#pragma once // _INJECTOR_H_

#include <string>
#include <sensor_internal.h>
#include "sensor_manager.h"

#define NAME_MAX_TEST 32
#define ARGC_BASE 4 /* e.g. {sensorctl, inject, wristup, conf} */

#define SENSORD_BUS_NAME 		"org.tizen.system.sensord"
#define SENSORD_OBJ_PATH		"/Org/Tizen/System/SensorD"
#define SENSORD_INTERFACE_NAME	"org.tizen.system.sensord"

#define REGISTER_INJECTOR(sensor_type, sensor_name, injector_type) \
static injector_interface injector(sensor_type, sensor_name);

namespace sensorctl {

class injector_manager;

class injector_interface {
public:
	injector_interface(sensor_type_t sensor_type, const char *event_name)
	: type(sensor_type)
	, name(event_name)
	{
		injector_manager::register_injector(this);
	}
	virtual ~injector_interface() {}

	virtual bool setup(void) { return true; }
	virtual bool teardown(void) { return true; }

	const std::string& name() const { return name; }
	const sensor_type_t type() const { return type; }

	virtual bool inject(int argc, char *argv[]) = 0;

private:
	sensor_type_t type;
	std::string name;
};

class injector_manager {
public:
	static bool register_injector(injector_interface *injector);

	injector_manager();
	virtual ~injector_manager();
	bool run(int argc, char *argv[]);
	
private:
	static std::vector<injector_interface *> injectors; 

	injector_interface *get_injector(sensor_type_t type, const char *name);
	bool run_injector(int argc, char *argv[]);

	void usage(void);
};

} /* namespace sensorctl */
