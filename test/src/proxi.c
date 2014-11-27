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
#include <sensor_internal.h>
#include <stdbool.h>

static GMainLoop *mainloop;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Proximity [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
}

void printformat()
{
	printf("Usage : ./proxi <event> <interval>(optional)\n\n");
	printf("event:\n");
	printf("EVENT_CHANGE_STATE\n");
	printf("EVENT_STATE_REPORT_ON_TIME\n");
	printf("EVENT_DISTANCE_DATA_REPORT_ON_TIME\n");
	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by proximity driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

int main(int argc,char **argv)
{
	int result, handle, start_handle, stop_handle;
	unsigned int event;

	mainloop = g_main_loop_new(NULL, FALSE);
	sensor_type_t type = PROXIMITY_SENSOR;
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = 100;

	if (argc != 2 && argc != 3) {
		printformat();
		free(event_condition);
		return 0;
	}

	if (strcmp(argv[1], "EVENT_CHANGE_STATE") == 0) {
		event = PROXIMITY_EVENT_CHANGE_STATE;
	}
	else if (strcmp(argv[1], "EVENT_STATE_REPORT_ON_TIME") == 0) {
	 	event = PROXIMITY_EVENT_STATE_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "EVENT_DISTANCE_DATA_REPORT_ON_TIME") == 0) {
		event = PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME;
	}
	else {
		printformat();
		free(event_condition);
		return 0;
	}

	if (argc == 3)
		event_condition->cond_value1 = atof(argv[2]);

	handle = sf_connect(type);
	result = sf_register_event(handle, event, event_condition, callback, NULL);

	if (result < 0)
		printf("Can't register proximity sensor\n");

	start_handle = sf_start(handle,0);

	if (start_handle < 0) {
		printf("Error\n\n\n\n");
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		return -1;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	sf_unregister_event(handle, event);

	stop_handle = sf_stop(handle);

	if (stop_handle < 0) {
		printf("Failed to stop proximity sensor\n\n");
		return -1;
	}

	sf_disconnect(handle);

	free(event_condition);

	return 0;
}

