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
#include <sensor_common.h>
#include <unistd.h>

static GMainLoop *mainloop;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void printformat()
{
	printf("Usage : ./gravity <mode>(optional) <event> <interval>(optional)\n\n");

	printf("mode:");
	printf("[-p]\n");
	printf("p is for polling based,default mode is event driven\n");

	printf("event:");
	printf("[RAW_DATA_REPORT_ON_TIME]\n");

	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer,gyroscope and geomagnetic driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

int main(int argc,char **argv)
{
	int result, handle, start_handle, stop_handle;
	unsigned int event;
	mainloop = g_main_loop_new(NULL, FALSE);
	event = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;

	sensor_type_t type = GRAVITY_SENSOR;

	if (argc != 2 && argc != 3 && argc!=4) {
		printformat();
		free(event_condition);
		return 0;
	}

	else if (argc>=3 && strcmp(argv[1], "-p") == 0 && strcmp(argv[2], "RAW_DATA_REPORT_ON_TIME") == 0) {
		printf("Polling based\n");
		handle = sf_connect(type);
		result = sf_start(handle, 1);

		if (result < 0) {
			printf("Can't start gravity SENSOR\n");
			printf("Error\n\n\n\n");
			return -1;
		}

		sensor_data_t data;

		while(1) {
			result = sf_get_data(handle, GRAVITY_BASE_DATA_SET , &data);
			printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data.timestamp, data.values[0], data.values[1], data.values[2]);			
			usleep(100000);
		}

		result = sf_disconnect(handle);

		if (result < 0) {
			printf("Can't disconnect gravity sensor\n");
			printf("Error\n\n\n\n");
			return -1;
		}
	}

	else if (strcmp(argv[1], "RAW_DATA_REPORT_ON_TIME") == 0) {
		printf("Event based\n");

		event_condition->cond_value1 = 100;
		if (argc == 3)
			event_condition->cond_value1 = atof(argv[2]);

		handle = sf_connect(type);
		result = sf_register_event(handle, event, event_condition, callback, NULL);

		if (result < 0)
			printf("Can't register gravity\n");

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
			printf("Error\n\n");
			return -1;
		}

		sf_disconnect(handle);
		free(event_condition);
	}

	else {
		printformat();
	}

	return 0;
}

