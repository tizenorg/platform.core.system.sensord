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

#include <stdio.h>
#include <string.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "dbus_util.h"
#include "injector.h"
#include "injector_manager.h"

static std::vector<injector_interface *> injector_manager::injectors;

injector_manager::injector_manager()
{
	if (!dbus_init()) {
		_E("Failed to init dbus");
		throw;
	}
}

injector_manager::~injector_manager()
{
	dbus_fini();
}

void injector_manager::register_injector(injector_interface *injector)
{
	injectors.push_back(injector);
}

injector_interface *injector_manager::get_injector(sensor_type_t type, const char *name)
{
	int count;

	count = injectors.size();

	for (int i = 0; i < count; ++i) {
		if (type == injectors[i].type() &&
		    !strcmp(injectors[i].name(), name))
			return injectors[i];
	}
	return NULL;
}

bool injector_manager::run(int argc, char *argv[])
{
	sensor_type_t type;
	char *event_name;
	bool result;
	int i;

	if (argc < ARGC_BASE) {
		usage();
		return false;
	}

	/* 1. get sensor type */
	type = get_sensor_type(argv[2]);
	if (type == UNKNOWN_SENSOR) {
		_E("ERROR : failed to run injector\n");
		return false;
	}

	/* 2. set up injector */
	event_name = argv[3];

	injector_interface *injector = get_injector(type, event_name);
	if (injector == NULL) {
		_E("ERROR: cannot find matched injector\n");
		return false;
	}

	/* 3. init injector */
	result = injector->init();
	if (!result) {
		_E("ERROR: failed to init injector\n");
		return false;
	}

	/* 4. inject event */
	result = injector->inject(argc, argv);
	if (!result) {
		_E("ERROR : failed to run injector\n");
		return false;
	}

	return true;
}

void injector_manager::usage(void)
{
	PRINT("usage: sensorctl inject <sensor_type> [<event_type>] [<options>]\n\n");

	usage_sensors();
}
