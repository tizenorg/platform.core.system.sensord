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

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data) {
	g_main_loop_quit(mainloop);
}

bool check_sensors_event_driven(sensor_type_t type, unsigned event, int cond_value) {
	int result, handle, start_handle, stop_handle, result_unregister, result_disconnect;
	mainloop = g_main_loop_new(NULL, FALSE);
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = cond_value;

	handle = sf_connect(type);

	if (handle < 0)
		return false;

	result = sf_register_event(handle, event, event_condition, callback, NULL);

	if (result < 0)
		return false;

	start_handle = sf_start(handle,0);

	if (start_handle < 0) {
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		free(event_condition);
		return false;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	result_unregister = sf_unregister_event(handle, event);

	if (result_unregister < 0)
		return false;

	stop_handle = sf_stop(handle);

	if (stop_handle < 0)
		return false;

	result_disconnect = sf_disconnect(handle);

	if (result_disconnect < 0)
		return false;

	free(event_condition);
	return true;
}

int main(int argc,char **argv) {
	bool result;

	result = check_sensors_event_driven(ACCELEROMETER_SENSOR, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Accelerometer - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(GEOMAGNETIC_SENSOR, GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Geomagnetic - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(GRAVITY_SENSOR, GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Gravity - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(GYROSCOPE_SENSOR, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, 10.52);
	printf("Gyroscope - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(LIGHT_SENSOR, LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME, 100);
	printf("Light - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(LINEAR_ACCEL_SENSOR, LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Linear Accel - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(ORIENTATION_SENSOR, ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Orientation - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(PRESSURE_SENSOR, PRESSURE_EVENT_RAW_DATA_REPORT_ON_TIME, 10.52);
	printf("Pressure - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(PROXIMITY_SENSOR, PROXIMITY_EVENT_CHANGE_STATE, 100);
	printf("Proxi - EVENT_CHANGE_STATE - %d\n", result);

	result = check_sensors_event_driven(PROXIMITY_SENSOR, PROXIMITY_EVENT_STATE_REPORT_ON_TIME, 100);
	printf("Proxi - EVENT_STATE_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(PROXIMITY_SENSOR, PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME, 100);
	printf("Proxi - EVENT_DISTANCE_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(ROTATION_VECTOR_SENSOR, ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Rotation Vector - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_event_driven(TEMPERATURE_SENSOR, TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	printf("Temperature - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	return 0;
}

