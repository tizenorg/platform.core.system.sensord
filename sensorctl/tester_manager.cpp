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
#include "sensor_log.h"
#include "tester.h"
#include "tester_manager.h"
#include "tester_sensor.h"

bool tester_manager::process(int argc, char *argv[])
{
	sensor_type_t type;
	if (argc == 2) {
		usage();
		return false;
	}

	if (!strcmp(argv[2], "help")) {
		usage();
		return false;
	}

	if (!strcmp(argv[2], "all")) {
		// TODO: auto_tester
		return false;
	}

	type = get_sensor_type(argv[2]);
	if (type == UNKNOWN_SENSOR)
		return false;

	tester_interface *tester = new tester_sensor();
	tester->init(type, argc, argv);
	tester->test();

	return true;
}

void tester_manager::usage(void)
{
	PRINT("usage: sensorctl test <sensor_type> [interval] [event_count] [test_count]\n");
	PRINT("\n");

	usage_sensors();

	PRINT("interval_ms:\n");
	PRINT("  interval. default value is 100ms.\n");
	PRINT("event count(n):\n");
	PRINT("  test sensor until it gets n event. default is 999999(infinitly).\n");
	PRINT("test count(n):\n");
	PRINT("  test sensor in n times repetitively, default is 1.\n");
	PRINT("\n");
}

