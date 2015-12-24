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
#include <sensor_internal.h>
#include "sensor_log.h"
#include "injector.h"
#include "injector_manager.h"
#include "injector_dbus.h"

bool injector_manager::process(int argc, char *argv[])
{
	sensor_type_t type;

	if (argc < 4) {
		usage();
		return false;
	}

	type = get_sensor_type(argv[2]);
	if (type == UNKNOWN_SENSOR)
		return false;

	injector_interface *injector = new injector_dbus();
	injector->init(type, argc, argv);
	injector->inject();

	return true;
}

void injector_manager::usage(void)
{
	PRINT("usage: sensorctl inject <sensor_type> [<event_type>] [options]\n");
	PRINT("\n");

	usage_sensors();
}

