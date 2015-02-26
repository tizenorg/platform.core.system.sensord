/*
 * sensord
 *
 * Copyright (c) 2014-15 Samsung Electronics Co., Ltd.
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

#define DEFAULT_EVENT_INTERVAL 100

static GMainLoop *mainloop;

void usage()
{
	printf("Usage : ./tc-common <Sensor_type> <event>(optional) <interval>(optional)\n\n");

	printf("Sensor_type: ");
	printf("[accelerometer] ");
	printf("[gyroscope] ");
	printf("[pressure] ");
	printf("[temperature] ");
	printf("[geomagnetic] ");
	printf("[orientation] ");
	printf("[gravity] ");
	printf("[linear_accel] ");
	printf("[rotation_vector] ");
	printf("[geomagnetic_rv] ");
	printf("[light]\n");
	printf("event:");
	printf("[RAW_DATA_REPORT_ON_TIME]\n");

	printf("Sensor_type: ");
	printf("[proximity]\n");
	printf("event:");
	printf("[EVENT_CHANGE_STATE]\n");

	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

unsigned int get_event_driven(sensor_type_t sensor_type, char str[])
{
	switch (sensor_type) {
	case ACCELEROMETER_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return ACCELEROMETER_RAW_DATA_EVENT;
		break;
	case GYROSCOPE_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return GYROSCOPE_RAW_DATA_EVENT;
		break;
	case PRESSURE_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return PRESSURE_RAW_DATA_EVENT;
		break;
	case GEOMAGNETIC_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return GEOMAGNETIC_RAW_DATA_EVENT;
		break;
	case LIGHT_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME;
		break;
	case TEMPERATURE_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return TEMPERATURE_RAW_DATA_EVENT;
		break;
	case PROXIMITY_SENSOR:
		if (strcmp(str, "EVENT_CHANGE_STATE") == 0)
			return PROXIMITY_CHANGE_STATE_EVENT;
		break;
	case ORIENTATION_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
	case GRAVITY_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
	case LINEAR_ACCEL_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
	case ROTATION_VECTOR_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
	case GEOMAGNETIC_RV_SENSOR:
		if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
			return GEOMAGNETIC_RV_RAW_DATA_EVENT;
		break;
	default:
		return -1;
	}
}

void callback(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	sensor_type_t sensor_type = event_type >> 16;

	switch (sensor_type) {
	case ACCELEROMETER_SENSOR:
		printf("Accelerometer [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case GYROSCOPE_SENSOR:
		printf("Gyroscope [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case PRESSURE_SENSOR:
		printf("Pressure [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case GEOMAGNETIC_SENSOR:
		printf("Geomagnetic [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case LIGHT_SENSOR:
		printf("Light [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
		break;
	case TEMPERATURE_SENSOR :
		printf("Temperature [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
		break;
	case PROXIMITY_SENSOR:
		printf("Proximity [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
		break;
	case ORIENTATION_SENSOR :
		printf("Orientation [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case GRAVITY_SENSOR:
		printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case LINEAR_ACCEL_SENSOR:
		printf("Linear acceleration [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
		break;
	case ROTATION_VECTOR_SENSOR:
		printf("Rotation vector [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3]);
		break;
	case GEOMAGNETIC_RV_SENSOR:
		printf("Geomagnetic RV [%lld] [%6.6f] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2], data->values[3]);
		break;
	default:
		return;
	}
}

int main(int argc, char **argv)
{
	int result, handle, start_handle, stop_handle, interval;
	char *end1, *end2;
	unsigned int event;
	bool EVENT_NOT_ENTERED = TRUE;
	sensor_type_t sensor_type;
	mainloop = g_main_loop_new(NULL, FALSE);

	if (argc < 2 || argc > 4) {
		printf("Wrong number of arguments\n");
		usage();
		return 0;
	}

	if (strcmp(argv[1], "accelerometer") == 0) {
		 sensor_type = ACCELEROMETER_SENSOR;
		 event = ACCELEROMETER_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gyroscope") == 0) {
		 sensor_type = GYROSCOPE_SENSOR;
		 event = GYROSCOPE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "pressure") == 0) {
		 sensor_type = PRESSURE_SENSOR;
		 event = PRESSURE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "temperature") == 0) {
		 sensor_type = TEMPERATURE_SENSOR;
		 event = TEMPERATURE_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "geomagnetic") == 0) {
		 sensor_type = GEOMAGNETIC_SENSOR;
		 event = GEOMAGNETIC_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "orientation") == 0) {
		 sensor_type = ORIENTATION_SENSOR;
		 event = ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "gravity") == 0) {
		 sensor_type = GRAVITY_SENSOR;
		 event = GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "linear_accel") == 0) {
		 sensor_type = LINEAR_ACCEL_SENSOR;
		 event = LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "rotation_vector") == 0) {
		 sensor_type = ROTATION_VECTOR_SENSOR;
		 event = ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "geomagnetic_rv") == 0) {
		 sensor_type = GEOMAGNETIC_RV_SENSOR;
		 event = GEOMAGNETIC_RV_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "light") == 0) {
		 sensor_type = LIGHT_SENSOR;
		 event = LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME;
	}
	else if (strcmp(argv[1], "proximity") == 0) {
		 sensor_type = PROXIMITY_SENSOR;
		 event = PROXIMITY_CHANGE_STATE_EVENT;
	}
	else {
		 usage();
	}

	interval = DEFAULT_EVENT_INTERVAL;

	if (argc > 2) {
		event = get_event_driven(sensor_type, argv[2]);

		if (event == -1) {
			usage();
			return -1;
		}

		EVENT_NOT_ENTERED = FALSE;
	}

	if (argc == 4) {
		interval = strtol(argv[3], &end1, 10);

		if (*end1) {
			printf("Conversion error, non-convertible part: %s\n", end1);
			return -1;
		}
	}

	if (argc == 3 && EVENT_NOT_ENTERED) {
		interval = strtol(argv[2], &end2, 10);

		if (*end2) {
			printf("Conversion error, non-convertible part: %s\n", end2);
			return -1;
		}
	}

	sensor_t sensor = sensord_get_sensor(sensor_type);
	handle = sensord_connect(sensor);

	result = sensord_register_event(handle, event, interval, 0, callback, NULL);

	if (result < 0) {
		printf("Can't register %s\n", argv[1]);
		return -1;
	}

	start_handle = sensord_start(handle, 0);

	if (start_handle < 0) {
		printf("Error\n\n\n\n");
		sensord_unregister_event(handle, event);
		sensord_disconnect(handle);
		return -1;
	}

	g_main_loop_run(mainloop);
	g_main_loop_unref(mainloop);

	result = sensord_unregister_event(handle, event);

	if (result < 0) {
		printf("Error\n\n");
		return -1;
	}

	stop_handle = sensord_stop(handle);

	if (stop_handle < 0) {
		printf("Error\n\n");
		return -1;
	}

	sensord_disconnect(handle);

	return 0;
}
