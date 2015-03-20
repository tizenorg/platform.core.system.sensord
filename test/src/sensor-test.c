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
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor_internal.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>
#include <string.h>

#include "check-sensor.h"

void usage()
{
	printf("Usage : ./sensor-test <Sensor_type> <event>(optional) <interval>(optional)\n\n");

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
	printf("[gaming_rv] ");
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

int main(int argc, char **argv)
{
	int interval;
	unsigned int event;
	sensor_type_t sensor_type;

	char *end1;

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
		 event = ORIENTATION_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gravity") == 0) {
		 sensor_type = GRAVITY_SENSOR;
		 event = GRAVITY_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "linear_accel") == 0) {
		 sensor_type = LINEAR_ACCEL_SENSOR;
		 event = LINEAR_ACCEL_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "rotation_vector") == 0) {
		 sensor_type = ROTATION_VECTOR_SENSOR;
		 event = ROTATION_VECTOR_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "geomagnetic_rv") == 0) {
		 sensor_type = GEOMAGNETIC_RV_SENSOR;
		 event = GEOMAGNETIC_RV_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "gaming_rv") == 0) {
		 sensor_type = GAMING_RV_SENSOR;
		 event = GAMING_RV_RAW_DATA_EVENT;
	}
	else if (strcmp(argv[1], "light") == 0) {
		 sensor_type = LIGHT_SENSOR;
		 event = LIGHT_LUX_DATA_EVENT;
	}
	else if (strcmp(argv[1], "proximity") == 0) {
		 sensor_type = PROXIMITY_SENSOR;
		 event = PROXIMITY_CHANGE_STATE_EVENT;
	}
	else {
		 usage();
	}

	interval = DEFAULT_EVENT_INTERVAL;

	if (argc == 3) {
		int temp_event = get_event_driven(sensor_type, argv[2]);

		if (temp_event == -1) {
			interval = atoi(argv[2]);
			if (interval == 0){
				usage();
				return -1;
			}
		}
		else {
			event = temp_event;
		}
	}
	else if (argc == 4) {
		event = get_event_driven(sensor_type, argv[2]);
		interval = strtol(argv[3], &end1, 10);

		if (*end1) {
			printf("Conversion error, non-convertible part: %s\n", end1);
			return -1;
		}
	}

	return check_sensor(sensor_type, event, interval);
}
