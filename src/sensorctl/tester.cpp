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

#include "log.h"
#include "macro.h"
#include "tester.h"
#include "tester_manual.h"

#define ARGC_BASE 3 /* e.g. {sensorctl, test, accelerometer} */

#define TEST_NAME_MANUAL "manual"

std::vector<tester *> sensorctl::tester_manager::testers;

/*
 * Implemenation of tester
 */
tester::tester(sensor_type_t sensor_type, const char *event_name)
: m_type(sensor_type)
, m_name(event_name)
{
	tester_manager::register_tester(this);
}

/*
 * Implementation of tester_manager
 */
void tester_manager::register_tester(tester *test)
{
	testers.push_back(test);
}

bool tester_manager::run(int argc, char *argv[])
{
	sensor_type_t type;
	char *event_name;
	bool result;

	if (argc < ARGC_BASE) {
		usage();
		return false;
	}

	/* 1. get sensor type */
	type = get_sensor_type(argv[2]);
	RETVM_IF(type == UNKOWN_SENSOR, false, "Invalid argument\n");

	/* 2. set up tester */
	/* if there is no option, "manual" is default event */
	if (argc == ARGC_BASE || is_number(argv[3]))
		event_name = TEST_NAME_MANUAL;
	else
		event_name = argv[3];

	tester *_tester = get_tester(type, event_name);
	RETVM_IF(!_tester, false, "cannot find matched tester\n");

	/* 3. set up tester */
	result = _tester->setup();
	RETVM_IF(!result, false, "Failed to setup tester\n");

	/* 4. test sensor with options */
	result = _tester->test(type, option_count, options);
	RETVM_IF(!result, false, "Failed to run tester\n");

	/* 5. tear down tester */
	result = _tester->teardown();
	RETVM_IF(!result, false, "Failed to tear down tester\n");

	return true;
}

tester *tester_manager::get_tester(sensor_type_t type, const char *name)
{
	tester *_tester;
	int tester_count;

	/* manual tester */
	if (!strcmp(name, TEST_NAME_MANUAL)) {
		_tester = new(std::nothrow) tester_manual(type, name);
		RETVM_IF(!_tester, NULL, "Failed to allocate memory");

		return _tester;
	}

	/* auto tester */
	tester_count = testers.size();
	for (int i = 0; i < tester_count; ++i) {
		if (type == testers[i].type() &&
		    !strcmp(testers[i].name(), name))
			return testers[i];
	}
	return NULL;
}

void tester_manager::usage(void)
{
	PRINT("usage: sensorctl test <sensor_type> [interval] [batch_latency] [event_count] [test_count]\n");

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

/*
 * Implementation of tester_bench
 */
void tester_bench::register_testcase(tester_case *testcase)
{
	instance().add_testcase(testcase);
}

void tester_bench::push_failure(const std::string &function, long line, const std::string &msg)
{
	instance().add_failure(function, line, msg);
}

void tester_bench::show_result(void)
{
	instance().show_failures();
}

void tester_bench::run_all_testcase(void)
{
	instance().run();
}

tester_bench& tester_bench::instance()
{
	static tester_bench tester;
	return tester;
}

void tester_bench::add_testcase(tester_case *testcase)
{
	tests.push_back(testcase);
}

void tester_bench::add_failure(const std::string &function, long line, const std::string &msg)
{
	struct result fail;
	fail.function = function;
	fail.line = line;
	fail.msg = msg;

	failures.push_back(fail);
	m_failure_count++;
}

void tester_bench::show_failures(void)
{
	PRINT("================================\n");

	if (m_failer_count == 0) {
		PRINT("there was no fail case\n");
		return;
	}

	PRINT("%d case(s) are failed, listed below:\n", m_failure_count);

	for (int i = 0; i < failures.size(); ++i) {
		PRINT("%s(%d) -> %s\n",
				failures[i].m_function.c_str(),
				failures[i].line,
				failures[i].msg.c_str());
	}
	m_failure_count = 0;
}

void tester_bench::run(void)
{
	int count = tests.size();

	_I("[==========] ");
	PRINT("Running %d tests\n", count);

	for (int i = 0; i < count; ++i)
		tests[i].run_testcase();

	_I("[==========] ");
	PRINT("%d tests ran\n", count);
	show_result();
}

/*
 * Implementation of tester_case
 */
tester_case::tester_case(const std::string &name)
: m_name(name)
{
	test_bench::register_testcase(this);
}

void tester_case::started(void)
{
	_I("[----------]\n");
	_I("[ RUN      ] ");
	PRINT("%s\n", m_name.c_str());
}

void tester_case::stopped(void)
{
	_I("[       OK ] ");
	PRINT("%s\n", m_name.c_str());
	_I("[----------]\n");
}

void tester_case::run_testcase(void)
{
	bool result;

	start_testcase();
	result = tc_func();
	stop_testcase();

	if (result)
		_I("[  PASSED  ] ");
	else
		_E("[  FAILED  ] ");
	PRINT("%s\n", tc_name);
}

void tester_case::register_func(test_func func)
{
	m_func = tc_func;
}
