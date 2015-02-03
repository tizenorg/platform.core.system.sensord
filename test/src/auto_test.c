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
FILE *fp;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data) {
	g_main_loop_quit(mainloop);
}

bool check_sensors_apis(sensor_type_t type, unsigned int event, int cond_value) {
	int result, handle, start_handle, stop_handle, result_unregister, result_disconnect;
	int result_change_event, result_change_option;

	mainloop = g_main_loop_new(NULL, FALSE);
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = cond_value;

	handle = sf_connect(type);

	if (handle < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_connect\n", type, event);
		return false;
	}

	result = sf_register_event(handle, event, event_condition, callback, NULL);

	if (result < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_register_event\n", type, event);
		return false;
	}

	start_handle = sf_start(handle,0);

	if (start_handle < 0) {
		sf_unregister_event(handle, event);
		sf_disconnect(handle);
		free(event_condition);
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_start\n", type, event);
		return false;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	event_condition->cond_value1 = 101;
	result_change_event = sf_change_event_condition(handle, event, event_condition);
	if (result_change_event < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_change_event_condition\n", type, event);
		return false;
	}

	result_change_option = sf_change_sensor_option(handle, SENSOR_OPTION_ON_IN_SCREEN_OFF);
	if (result_change_option < 0){
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_change_sensor_option\n", type, event);
		return false;
	}

	result_unregister = sf_unregister_event(handle, event);

	if (result_unregister < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_unregister_event\n", type, event);
		return false;
	}

	stop_handle = sf_stop(handle);

	if (stop_handle < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_stop\n", type, event);
		return false;
	}

	result_disconnect = sf_disconnect(handle);

	if (result_disconnect < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sf_disconnect\n", type, event);
		return false;
	}

	free(event_condition);
	return true;
}

int main(int argc,char **argv) {
	bool result;

	fp = fopen("auto_test.output", "w+");

	result = check_sensors_apis(ACCELEROMETER_SENSOR, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Accelerometer - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(GEOMAGNETIC_SENSOR, GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Geomagnetic - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(GRAVITY_SENSOR, GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Gravity - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(GYROSCOPE_SENSOR, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, 10.52);
	fprintf(fp, "Gyroscope - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(LIGHT_SENSOR, LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Light - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(LINEAR_ACCEL_SENSOR, LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Linear Accel - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(ORIENTATION_SENSOR, ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Orientation - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(PRESSURE_SENSOR, PRESSURE_EVENT_RAW_DATA_REPORT_ON_TIME, 10.52);
	fprintf(fp, "Pressure - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_CHANGE_STATE, 100);
	fprintf(fp, "Proxi - EVENT_CHANGE_STATE - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_STATE_REPORT_ON_TIME, 100);
	fprintf(fp, "Proxi - EVENT_STATE_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Proxi - EVENT_DISTANCE_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(ROTATION_VECTOR_SENSOR, ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Rotation Vector - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(TEMPERATURE_SENSOR, TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME, 100);
	fprintf(fp, "Temperature - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	fclose(fp);
	return 0;
}
