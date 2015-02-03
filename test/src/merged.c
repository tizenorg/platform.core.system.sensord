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
#include <string.h>

static GMainLoop *mainloop;

void callback_accel(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Accelerometer [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}


void printformat()
{
	printf("Usage : ./merged <Sensor_name> <event> <interval>(optional)\n\n");

	printf("Sensor_name:");
	printf("[accelerometer]\n");

	printf("event:");
	printf("[RAW_DATA_REPORT_ON_TIME]\n");

	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

unsigned int get_event_driven(sensor_type_t type,char str[])
{
	switch(type) {
		case(ACCELEROMETER_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME;
			break;
		default:
		return -1;
		break;
	}
}



int main(int argc,char **argv)
{
	int result, handle, start_handle, stop_handle;
	unsigned int event;
	sensor_type_t type;
	mainloop = g_main_loop_new(NULL, FALSE);
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;

	if (argc < 3 || argc > 4) {
		printf("Wrong number of arguments");
		printformat();
		return 0;
	}

	if (strcmp(argv[1], "accelerometer") == 0)
		 type = ACCELEROMETER_SENSOR;

	else
		 printformat();

	event_condition->cond_value1 = 100.00;

	event = get_event_driven(type,argv[2]);
	if (event == -1) {
		free(event_condition);
		return -1;
	}

	if (argc == 4)
		event_condition->cond_value1 = atof(argv[3]);

	handle = sf_connect(type);

	switch(type) {
		case(ACCELEROMETER_SENSOR):
		result = sf_register_event(handle, event, event_condition, callback_accel, NULL);
		break;
		default:
		printformat();
		break;
	}

	if (result < 0) {
		printf("Can't register %s\n",argv[1]);
		free(event_condition);
		return -1;
	}

	start_handle = sf_start(handle, 0);

	if (start_handle < 0) {
		printf("Error\n\n\n\n");
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		free(event_condition);
		return -1;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	sf_unregister_event(handle, event);
	stop_handle = sf_stop(handle);

	if (stop_handle < 0) {
		printf("Error\n\n");
		free(event_condition);
		return -1;			
	}

	sf_disconnect(handle);
	free(event_condition);
	
	return 0;
}



