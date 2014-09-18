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

#define NUM_SENSORS 2

static GMainLoop *mainloop;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;

	if(event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)
		printf("Accelerometer %lld\n\n", data->timestamp);

	if(event_type == GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME)
		printf("Gyroscope %lld\n\n", data->timestamp);

}

int main(int argc,char **argv)
{
	int result, handle, i = 1, handle_registered[NUM_SENSORS];
	unsigned int event, event_registered[NUM_SENSORS];

	sensor_type_t type;
	event_condition_t *event_condition;

	mainloop = g_main_loop_new(NULL, FALSE);

	if (i == 1) {
	event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = 100;
	type = ACCELEROMETER_SENSOR;
	event = ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME;
	event_registered[i] = event;
	handle = sf_connect(type);
	handle_registered[i] = handle;
	result = sf_register_event(handle, event, event_condition, callback, NULL);

	if (result < 0)
		printf("Can't register accelerometer\n");

	if (sf_start(handle, 0) < 0) {
		printf("Error accelerometer\n\n\n");
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		return -1;
	}
	free(event_condition);
        }

	i = i+1;

	if(i == 2){
	event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = 10.52;
	type = GYROSCOPE_SENSOR;
	event = GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME;
	event_registered[i] = event;
	handle = sf_connect(type);
	handle_registered[i] = handle;
	result = sf_register_event(handle, event, event_condition, callback, NULL);

	if (result < 0)
		printf("Can't register gyroscope\n");

	if (sf_start(handle, 0) < 0) {
		printf("Error gyroscope\n\n\n");
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		return -1;
	}
	free(event_condition);
        }

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	for (i = 1; i <= NUM_SENSORS; i++) {
	handle = handle_registered[i];
	event = event_registered[i];

	sf_unregister_event(handle, event);

	if (!(sf_stop(handle) < 0))
		printf("Success stop \n");

		sf_disconnect(handle);
	}

	return 0;
}

