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
#include <sensor_common.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>


static GMainLoop *mainloop;
FILE *fp;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	g_main_loop_quit(mainloop);
}

bool check_sensor_api(sensor_type_t sensor_type, unsigned int event_type, int cond_value)
{
	int result, handle, start_handle, stop_handle, result_unregister, result_disconnect;
	int result_change_event, result_change_option, result_get_data;

	mainloop = g_main_loop_new(NULL, FALSE);
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1 = cond_value;

	handle = sensord_connect(sensord_get_sensor(sensor_type));

	if (handle < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_connect\n", sensor_type, event_type);
		return false;
	}

	bool is_supported;
	bool result_boolean = sensord_is_supported_event_type(sensord_get_sensor(sensor_type), event_type, &is_supported);
	if (!result_boolean && !is_supported) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_is_supported_event\n", sensor_type, event_type);
		return false;
	}

	int output;
	result_boolean = sensord_get_min_interval(sensord_get_sensor(sensor_type), &output);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_min_interval\n", sensor_type, event_type);
		return false;
	}

	float output3;
	result_boolean = sensord_get_resolution(sensord_get_sensor(sensor_type), &output3);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_resolution\n", sensor_type, event_type);
		return false;
	}

	result_boolean = sensord_get_max_range(sensord_get_sensor(sensor_type), &output3);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_max_range\n", sensor_type, event_type);
		return false;
	}

	result_boolean = sensord_get_min_range(sensord_get_sensor(sensor_type), &output3);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_min_range\n", sensor_type, event_type);
		return false;
	}

	sensor_privilege_t output4;
	result_boolean = sensord_get_privilege(sensord_get_sensor(sensor_type), &output4);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_privilege\n", sensor_type, event_type);
		return false;
	}

	const char* result_char = sensord_get_vendor(sensord_get_sensor(sensor_type));
	if (!result_char) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_vendor\n", sensor_type, event_type);
		return false;
	}

	result_char = sensord_get_name(sensord_get_sensor(sensor_type));
	if (!result_char) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_name\n", sensor_type, event_type);
		return false;
	}

	sensor_type_t output_type;
	result_boolean = sensord_get_type(sensord_get_sensor(sensor_type), &output_type);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_type\n", sensor_type, event_type);
		return false;
	}

	unsigned int *output2;
	result_boolean = sensord_get_supported_event_types(sensord_get_sensor(sensor_type), &output2, &output);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_supported_event_types\n", sensor_type, event_type);
		return false;
	}

	sensor_t *output_list;
	result_boolean = sensord_get_sensor_list(sensor_type, &output_list, &output);
	if (!result_boolean) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_sensor_list\n", sensor_type, event_type);
		return false;
	}

	result = sensord_register_event(handle, event_type, cond_value, 0, callback, NULL);

	if (result < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_register_event\n", sensor_type, event_type);
		return false;
	}

	start_handle = sensord_start(handle, 1) ? 0 : -1;

	if (start_handle < 0) {
		sensord_unregister_event(handle, event_type);
		sensord_disconnect(handle);
		free(event_condition);
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_start\n", sensor_type, event_type);
		return false;
	}

	sensor_data_t data;
	result_get_data = sensord_get_data(handle, event_type, &data) ? 0 : -1;
	if (result_get_data < 0) {
		sensord_unregister_event(handle, event_type);
		sensord_disconnect(handle);
		free(event_condition);
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_get_data\n", sensor_type, event_type);
		return false;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	event_condition->cond_value1 = 101;
	result_change_event = sensord_change_event_interval(handle, event_type, 101) ? 0 : -1;
	if (result_change_event < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_change_event_interval\n", sensor_type, event_type);
		return false;
	}

	result_change_option = sensord_set_option(handle, SENSOR_OPTION_ON_IN_SCREEN_OFF) ? 0 : -1;
	if (result_change_option < 0){
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_change_sensor_option\n", sensor_type, event_type);
		return false;
	}

	result_unregister = sensord_unregister_event(handle, event_type) ? 0 : -1;

	if (result_unregister < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_unregister_event\n", sensor_type, event_type);
		return false;
	}

	stop_handle = sensord_stop(handle) ? 0 : -1;

	if (stop_handle < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_stop\n", sensor_type, event_type);
		return false;
	}

	result_disconnect = sensord_disconnect(handle) ? 0 : -1;

	if (result_disconnect < 0) {
		fprintf(fp, "Sensor - %d, event - %d, failed at sensord_disconnect\n", sensor_type, event_type);
		return false;
	}

	free(event_condition);
	return true;
}

int main(int argc, char **argv)
{
	bool result;

	int interval = 100;
	if (argc == 2)
		interval = atof(argv[1]);

	fp = fopen("auto_test.output", "w+");

	result = check_sensor_api(ACCELEROMETER_SENSOR, ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Accelerometer - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(GEOMAGNETIC_SENSOR, GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Geomagnetic - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(GRAVITY_SENSOR, GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Gravity - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(GYROSCOPE_SENSOR, GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Gyroscope - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(LIGHT_SENSOR, LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Light - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(LINEAR_ACCEL_SENSOR, LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Linear Accel - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(ORIENTATION_SENSOR, ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Orientation - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(PRESSURE_SENSOR, PRESSURE_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Pressure - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_CHANGE_STATE, interval);
	fprintf(fp, "Proxi - EVENT_CHANGE_STATE - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_STATE_REPORT_ON_TIME, interval);
	fprintf(fp, "Proxi - EVENT_STATE_REPORT_ON_TIME - %d\n", result);

	result = check_sensors_apis(PROXIMITY_SENSOR, PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Proxi - EVENT_DISTANCE_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(ROTATION_VECTOR_SENSOR, ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Rotation Vector - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	result = check_sensor_api(TEMPERATURE_SENSOR, TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME, interval);
	fprintf(fp, "Temperature - RAW_DATA_REPORT_ON_TIME - %d\n", result);

	printf("Logs printed in ./auto_test.output\n");
	fclose(fp);
	return 0;
}
