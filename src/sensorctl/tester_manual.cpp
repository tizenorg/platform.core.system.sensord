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
#include "tester.h"
#include "tester_manual.h"

#define DEFAULT_INTERVAL 100
#define DEFAULT_LATENCY 0
#define DEFAULT_TEST_COUNT 1
#define DEFAULT_EVENT_COUNT 9999

#define SENSOR_EVENT(type) ((type) << 16 | 0x1)

namespace sensorctl {

static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	sensor_type_t type;
	sensord_get_type(sensor, &type);

	PRINT("[%llu] :", data->timestamp);
	for (int i = 0; i < data->value_count; ++i)
		PRINT(" [%10f]", data->values[i]);
	PRINT("\n");
}

tester_manual::tester_manual(sensor_type_t sensor_type, const char *event_name)
: tester(sensor_type, event_name)
{
}

bool tester_manual::test_sensor(sensor_type_t type, int interval, int latency, int cnt_event, int cnt_test)
{
	sensor_t sensor;
	int handle;
	int error;
	bool result;
	int count = 1;

	PRINT("=======================================\n");
	PRINT("TEST(%d/%d)\n", count, cnt_test);
	PRINT("=======================================\n");

	while (count++ <= cnt_test) {
		error = sensord_get_default_sensor(type, &sensor);
		ASSERT_IF_TEXT((error == 0), "manual.sensord_get_default_sensor");

		handle = sensord_connect(sensor);
		ASSERT_IF_TEXT((handle >= 0), "manual.sensord_connect");

		result = sensord_register_event(handle, SENSOR_EVENT(type), interval, latency, test_cb, NULL);
		ASSERT_IF_TEXT((result == true), "manual.sensord_register_event");

		result = sensord_start(handle, SENSOR_OPTION_ALWAYS_ON);
		ASSERT_IF_TEXT((result == true), "manual.sensord_start");

		sensor_data_t data;
		result = sensord_get_data(handle, SENSOR_EVENT(type), &data);
		EXPECT_IF_TEXT((result == true), "manual.sensord_get_data");

		result = sensord_flush(handle);
		EXPECT_IF_TEXT((result == true), "manual.sensord_flush");

		mainloop::run();
		mainloop::stop();

		result = sensord_unregister_event(handle, SENSOR_EVENT(type));
		ASSERT_IF_TEXT((result == true), "manual.sensord_unregister_event");

		result = sensord_stop(handle);
		ASSERT_IF_TEXT((result == true), "manual.sensord_stop");

		result = sensord_disconnect(handle);
		ASSERT_IF_TEXT((result == true), "manual.sensord_disconnect");
	}

	return true;
}

bool tester_manual::test(sensor_type_t type, int option_count, char *options[])
{
	int interval = DEFAULT_INTERVAL;
	int latency = DEFAULT_LATENCY;
	int cnt_test = DEFAULT_TEST_COUNT;
	int cnt_event = DEFAULT_EVENT_COUNT;

	sensor_type_t sensor_type = type;

	if (option_count >= 1)
		interval = atoi(options[TESTER_ARGC]);
	if (option_count >= 2)
		latency = atoi(options[TESTER_ARGC+1]);
	if (option_count >= 3)
		cnt_event = atoi(options[TESTER_ARGC+2]);
	if (option_count >= 4)
		cnt_test = atoi(options[TESTER_ARGC+3]);

	return test_sensor(sensor_type, interval, latency, cnt_event, cnt_test);
}

} /* namespace sensorctl */
