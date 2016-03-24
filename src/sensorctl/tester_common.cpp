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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "tester_common.h"

#define SENSOR_SHIFT_TYPE 16
#define SENSOR_EVENT(type) ((type) << SENSOR_SHIFT_TYPE | 0x1)

#define ASSERT_IF(cond, src, dst, name, tester) \
	if ((cond)) { \
		PRINT(KGRN "[   PASS   ] " KNRM name " [%d, %d]\n", src, dst); \
		tester->pass(name); \
	} else { \
		PRINT(KRED "[   FAIL   ] " KNRM name " [%d, %d]\n", src, dst); \
		tester->fail(name); \
		return false; \
	}

#define ASSERT_EQ(src, dst, name, tester) \
	ASSERT_IF((src) == (dst), src, dst, name, tester)

#define ASSERT_NE(src, dst, name, tester) \
	ASSERT_IF((src) != (dst), src, dst, name, tester)

#define ASSERT_GE(src, dst, name, tester) \
	ASSERT_IF((src) >= (dst), src, dst, name, tester)

#define EXPECT_IF(cond, src, dst, name, tester) \
	if ((cond)) { \
		PRINT(KGRN "[   PASS   ] " KNRM name " [%d, %d]\n", src, dst); \
		tester->pass(name); \
	} else { \
		PRINT(KRED "[   FAIL   ] " KNRM name " [%d, %d]\n", src, dst); \
		tester->fail(name); \
	}

#define EXPECT_EQ(src, dst, name, tester) \
	EXPECT_IF((src) == (dst), src, dst, name, tester)

#define EXPECT_NE(src, dst, name, tester) \
	EXPECT_IF((src) != (dst), src, dst, name, tester)

#define EXPECT_GE(src, dst, name, tester) \
	EXPECT_IF((src) >= (dst), src, dst, name, tester)

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

bool tester_common::is_supported(sensor_type_t type)
{
	sensor_t sensor;

	sensord_get_default_sensor(type, &sensor);
	if (!sensor)
		return false;

	return true;
}

bool tester_common::scenario_basic_p(sensor_type_t type, int interval, int latency, sensor_cb_t cb, void *user_data)
{
	int ret;
	sensor_t sensor;

	reset();

	ret = sensord_get_default_sensor(type, &sensor);
	ASSERT_EQ(ret, 0, "sensord_get_default_sensor", this);

	handle = sensord_connect(sensor);
	ASSERT_GE(handle, 0, "sensord_connect", this);

	ret = sensord_register_event(handle, SENSOR_EVENT(type), interval, latency, cb, user_data);
	ASSERT_EQ(ret, 1, "sensord_register_event", this);

	ret = sensord_start(handle, SENSOR_OPTION_ALWAYS_ON);
	ASSERT_EQ(ret, 1, "sensord_start", this);

	ret = sensord_get_data(handle, SENSOR_EVENT(type), &data);
	EXPECT_EQ(ret, 1, "sensord_get_data", this);

	/*
	ret = sensord_flush(handle);
	EXPECT_EQ(ret, 0, "sensord_flush", this);
	*/

	run_loop();
	exit_loop();

	ret = sensord_unregister_event(handle, SENSOR_EVENT(type));
	ASSERT_EQ(ret, 1, "sensord_unregister_event", this);

	ret = sensord_stop(handle);
	ASSERT_EQ(ret, 1, "sensord_stop", this);

	ret = sensord_disconnect(handle);
	ASSERT_EQ(ret, 1, "sensord_disconnect", this);

	return (m_fail == 0) ? true : false;
}
