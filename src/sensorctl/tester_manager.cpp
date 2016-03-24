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
#include <macro.h>
#include <sensorctl_log.h>
#include <sensor_internal.h>

#include "tester.h"
#include "tester_manager.h"
#include "tester_manual.h"

#define ARGC_BASE 3 /* e.g. {sensorctl, test, accelerometer} */

#define TEST_NAME_MANUAL "manual"

static std::vector<tester_info> tester_infos;

tester_manager::~tester_manager()
{
	int tester_count;

	tester_count = tester_infos.size();
	for (int i = 0; i < tester_count; ++i)
		delete tester_infos[i].tester;

	tester_common::exit_loop();
}

bool tester_manager::register_tester(tester_info info)
{
	tester_infos.push_back(info);
	return true;
}

tester_interface *tester_manager::get_tester(sensor_type_t type, const char *name)
{
	tester_interface *tester;
	int tester_count;

	/* manual tester */
	if (!strcmp(name, TEST_NAME_MANUAL)) {
		tester = new(std::nothrow) tester_manual;
		if (!tester)
			_E("ERROR: Failed to allocate memory");
		return tester;
	}

	/* automatic tester */
	tester_count = tester_infos.size();
	for (int i = 0; i < tester_count; ++i) {
		if (type == tester_infos[i].type &&
		    !strcmp(tester_infos[i].name, name))
			return tester_infos[i].tester;
	}
	return NULL;
}

bool tester_manager::run(int argc, char *argv[])
{
	sensor_type_t type;
	int option_count;
	char *options[8];
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
		_E("ERROR : failed to run tester\n");
		return false;
	}

	/* 2. set up tester */
	/* if there is no event option, "manual" is default event */
	if (argc == ARGC_BASE || is_number(argv[3]))
		event_name = TEST_NAME_MANUAL;
	else
		event_name = argv[3];

	tester_interface *tester = get_tester(type, event_name);
	if (!tester) {
		_E("ERROR: cannot find matched tester\n");
		return false;
	}

	/* 3. init tester */
	result = tester->init();
	if (!result) {
		_E("ERROR: failed to init tester\n");
		return false;
	}

	/* 4. test sensor with options */
	option_count = argc - ARGC_BASE;
	for (i = 0; i < option_count; ++i) {
		options[i] = new char[NAME_MAX_TEST];
		strcpy(options[i], argv[ARGC_BASE+i]);
	}

	result = tester->test(type, option_count, options);
	if (!result) {
		_E("ERROR: failed to run tester\n");
		for (i = 0; i < option_count; ++i)
			delete options[i];
		delete tester;

		return false;
	}

	for (i = 0; i < option_count; ++i)
		delete options[i];
	delete tester;

	return true;
}

void tester_manager::usage(void)
{
	PRINT("usage 1: sensorctl test <sensor_type> [interval] [batch_latency] [event_count] [test_count]\n");

	usage_sensors();

	PRINT("sensor_type: all:\n");
	PRINT("  test all sensors automatically.\n");
	PRINT("interval_ms:\n");
	PRINT("  interval. default value is 100ms.\n");
	PRINT("batch_latency_ms:\n");
	PRINT("  batch_latency. default value is 1000ms.\n");
	PRINT("event count(n):\n");
	PRINT("  test sensor until it gets n event. default is 999999(infinitly).\n");
	PRINT("test count(n):\n");
	PRINT("  test sensor in n times repetitively, default is 1.\n\n");
}

