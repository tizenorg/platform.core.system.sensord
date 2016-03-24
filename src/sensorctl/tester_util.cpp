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
#include <stdlib.h>
#include <sensor_internal.h>

#include "log.h"

#define ASSERT_IF(condition, testsuit, testcase) \
{ \
	if ((condition)) { \
		_I("[   PASS   ] "); \
		PRINT("%s(%d) -> %s.%s: %s", __FUNCTION__, __LINE__, testsuit, testcase, condition); \
	} else { \
		_E("[   FAIL   ] "); \
		PRINT("%s(%d) -> %s.%s: %s", __FUNCTION__, __LINE__, testsuit, testcase, condition); \
		return false; \
	} \
}

#define EXPECT_IF(condition) \
{ \
	if ((condition)) \
		PRINT(KGRN "[   PASS   ] " KNRM " [%d, %d]\n", expected, actual); \
	} else { \
		PRINT(KRED "[   FAIL   ] " KNRM name " [%d, %d]\n", expected, actual); \
	} \
}

#define EXPECT_COMP(expected, comp, actual, tcname) \
{ \
	if ((expected) comp (actual)) { \
		std::stringstream _stream; \
		_stream << "expected " << expected \
				<< ", but it was " << actual; \
		sensorctl::test::report(__FUNCTION__, __LINE__, _stream.str()); \
		PRINT(KGRN "[   PASS   ] " KNRM name " [%d, %d]\n", expected, actual); \
	} else { \
		PRINT(KRED "[   FAIL   ] " KNRM name " [%d, %d]\n", expected, actual); \
	} \
}


namespace test {

class tester_common {
public:
	void reset(void);
	void pass(const char *name);
	void fail(const char *name);
	void result(int &pass, int &fail);

	bool is_supported(sensor_type_t type);
	bool scenario_basic_p(sensor_type_t type, int interval, int latency, sensor_cb_t cb, void *user_data);
private:
	int handle;
	sensor_data_t data;

	int m_pass;
	int m_fail;
};

GMainLoop *tester_common::mainloop;

bool tester_common::is_running_loop(void)
{
	if (!mainloop)
		return false;

	return (bool)g_main_is_running(mainloop);
}

bool tester_common::run_loop(void)
{
	if (is_running_loop())
		return true;

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);
	return true;
}

bool tester_common::exit_loop(void)
{
	if (!is_running_loop())
		return true;

	g_main_loop_quit(mainloop);
	g_main_loop_unref(mainloop);
	mainloop = NULL;
	return true;
}

void tester_common::reset(void)
{
	m_pass = 0;
	m_fail = 0;
}

void tester_common::pass(const char *name)
{
	m_pass++;
}

void tester_common::fail(const char *name)
{
	m_fail++;
}

void tester_common::result(int &pass, int &total)
{
	pass = m_pass;
	total = m_pass + m_fail;
}

} /* namespace test */


