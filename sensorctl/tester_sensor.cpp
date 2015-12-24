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
#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>
#include "sensor_log.h"
#include "tester_sensor.h"

#define DEFAULT_INTERVAL 100
#define DEFAULT_LATENCY 0
#define DEFAULT_TEST_COUNT 1
#define DEFAULT_EVENT_COUNT 9999

#define SENSOR_SHIFT_TYPE 16

static GMainLoop *mainloop;
static int check_loop;

static const char *result_str(int result) {
	if (result)		return KGRN"[PASS]"RESET;
	else 			return KRED"[FAIL]"RESET;
}

bool tester_sensor::init(sensor_type_t type, int argc, char *argv[])
{
	interval = DEFAULT_INTERVAL;
	latency = DEFAULT_LATENCY;
	cnt_test = DEFAULT_TEST_COUNT;
	cnt_event = DEFAULT_EVENT_COUNT;

	sensor_type = type;

	if (argc >= 4)
		interval = atoi(argv[3]);
	if (argc >= 5)
		cnt_event = atoi(argv[4]);
	if (argc >= 6)
		cnt_test = atoi(argv[5]);

	return true;
}

void tester_sensor::test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	sensor_type_t type;
	int *cnt_event;

	sensord_get_type(sensor, &type);

	cnt_event = (int *)user_data;

	if (check_loop++ >= *cnt_event) {
		if (mainloop) {
			g_main_loop_quit(mainloop);
			g_main_loop_unref(mainloop);
		}
		mainloop = NULL;
		return;
	}

	PRINT("[%llu] ", data->timestamp);
	PRINT("%s:", sensord_get_name(sensor));

	for (int i=0; i<data->value_count; ++i)
		PRINT(" [%f]", data->values[i]);
	PRINT("\n");
}

void tester_sensor::test_sensor(sensor_type_t type, int interval, int latency, int cnt_test, int cnt_event)
{
	sensor_t sensor;
	unsigned int event_id;
	sensor_data_t data;
	int handle;
	int ret;
	int count = 0;

	event_id = type << SENSOR_SHIFT_TYPE | 0x1;

	while (count++ < cnt_test) {
		mainloop = g_main_loop_new(NULL, FALSE);
		check_loop = 0;

		PRINT("=======================================\n");
		PRINT("TEST(%d/%d)\n", count, cnt_test);
		PRINT("=======================================\n");

		sensor = sensord_get_sensor(type);
		PRINT("%s sensord_get_sensor: sensor(%p)\n", result_str(sensor==NULL?0:1), sensor);

		handle = sensord_connect(sensor);
		PRINT("%s sensord_connect: handle(%d)\n", result_str((handle >= 0)), handle);

		ret = sensord_register_event(handle, event_id, interval, latency, test_cb, (void *)&cnt_event);
		PRINT("%s sensord_register_event\n", result_str(ret));

		ret = sensord_start(handle, 3);
		PRINT("%s sensord_start\n", result_str(ret));

		ret = sensord_get_data(handle, event_id, &data);
		PRINT("%s sensord_get_data: ", result_str(ret));

		if (ret) {
			for (int i=0; i<data.value_count; ++i)
				PRINT("[%f] ", data.values[i]);
			PRINT("\n");
		}

		g_main_loop_run(mainloop);

		ret = sensord_unregister_event(handle, event_id);
		PRINT("%s sensord_unregister_event: handle(%d)\n", result_str(ret), handle);
		ret = sensord_stop(handle);
		PRINT("%s sensord_stop: handle(%d)\n", result_str(ret), handle);
		ret = sensord_disconnect(handle);
		PRINT("%s sensord_disconnect: handle(%d)\n", result_str(ret), handle);
	}
}

bool tester_sensor::test()
{
	test_sensor(sensor_type, interval, latency, cnt_test, cnt_event);
	return true;
}
