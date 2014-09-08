/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <time.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor.h>
#include <stdbool.h>
#include <string.h>

static GMainLoop *mainloop;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Gravity [%6.6f] [%6.6f] [%6.6f] [%lld]\n\n", data->values[0], data->values[1], data->values[2], data->timestamp);
}

void printformat()
{
	printf("Usage : ./gravity <event> <interval>(optional)\n\n");
	printf("event:\n");
	printf("RAW_DATA_REPORT_ON_TIME\n\n");
	printf("interval:\n");
	printf("The time interval should be entered based on the sampling "
			"frequencies supported by accelerometer, gyroscope and "
			"geomagnetic sensors driver on the device in ms. If no value "
			"for gravity sensor is entered, a default value will be used.\n");
}

int main(int argc,char **argv)
{
	int result, handle;
	bool error_state = FALSE;
	unsigned int event;

	mainloop = g_main_loop_new(NULL, FALSE);
	sensor_type_t type = GRAVITY_SENSOR;
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = 100;

	if (argc != 2 && argc != 3) {
		printformat();
		error_state = TRUE;
	}
	else {
		if (strcmp(argv[1], "RAW_DATA_REPORT_ON_TIME") == 0)
			event = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
		else {
			printformat();
			error_state = TRUE;
		}

		if (argc == 3)
			event_condition->cond_value1 = atof(argv[2]);
	}

	if (!error_state) {
		handle = sf_connect(type);
		result = sf_register_event(handle, event, event_condition, callback, NULL);

		if (result < 0)
			printf("Can't register gravity virtual sensor\n");

		if (!(sf_start(handle,0) < 0)) {
			printf("Success start \n");
		}
		else {
			printf("Error\n\n\n\n");
			sf_unregister_event(handle, event);
			sf_disconnect(handle);
			return -1;
		}

		g_main_loop_run(mainloop);
		g_main_loop_unref(mainloop);

		sf_unregister_event(handle, event);

		if (!(sf_stop(handle) < 0))
			printf("Success stop \n");

		sf_disconnect(handle);
	}

	free(event_condition);

	return 0;
}
