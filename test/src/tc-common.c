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

#define default_event_interval 100.00

static GMainLoop *mainloop;

void printformat()
{
	printf("Usage : ./tc-common <Sensor_name> <event> <interval>(optional)\n\n");

	printf("Sensor_name: ");
	printf("[accelerometer] ");
	printf("[gyroscope] ");
	printf("[pressure] ");
	printf("[temperature] ");
	printf("[geomagnetic] ");
	printf("[orientation] ");
	printf("[gravity] ");
	printf("[linear_accel] ");
	printf("[rotation_vector] ");
	printf("[gaming_rotation_vector] ");
	printf("[light]\n");
	printf("event:");
	printf("[RAW_DATA_REPORT_ON_TIME]\n");

	printf("Sensor_name: ");
	printf("[proximity] ");
	printf("event:");
	printf("[EVENT_CHANGE_STATE] ");
	printf("[EVENT_STATE_REPORT_ON_TIME] ");
	printf("[EVENT_DISTANCE_DATA_REPORT_ON_TIME]\n");

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
		case(GYROSCOPE_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(PRESSURE_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return PRESSURE_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(GEOMAGNETIC_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(LIGHT_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME;
		break;
		case(TEMPERATURE_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(PROXIMITY_SENSOR):
			if (strcmp(str, "EVENT_CHANGE_STATE") == 0)
				return PROXIMITY_EVENT_CHANGE_STATE;
			else if (strcmp(str, "EVENT_STATE_REPORT_ON_TIME") == 0)
				return PROXIMITY_EVENT_STATE_REPORT_ON_TIME;
			else if (strcmp(str, "EVENT_DISTANCE_DATA_REPORT_ON_TIME") == 0)
				return PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME;
		break;
		case(ORIENTATION_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(GRAVITY_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(LINEAR_ACCEL_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(ROTATION_VECTOR_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		case(GEOMAGNETIC_RV_SENSOR):
			if (strcmp(str, "RAW_DATA_REPORT_ON_TIME") == 0)
				return GEOMAGNETIC_RV_EVENT_RAW_DATA_REPORT_ON_TIME;
		break;
		default:
			return -1;
		break;
	}
}

void callback_accel(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Accelerometer [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_gyro(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Gyroscope [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_pressure(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Pressure [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_geo(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Geomagnetic [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_light(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Light [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
}

void callback_temperature(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Temperature [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
}

void callback_proxi(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Proximity [%lld] [%6.6f]\n\n", data->timestamp, data->values[0]);
}

void callback_orientation(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Orientation [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_gravity(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Gravity [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_linear__accel(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Linear acceleration [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_rot_vec(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Rotation vector [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
}

void callback_geomag_rv(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Geomagnetic rotation vector [%lld] [%6.6f] [%6.6f] [%6.6f]\n\n", data->timestamp, data->values[0], data->values[1], data->values[2]);
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
	else if (strcmp(argv[1], "gyroscope") == 0)
		 type = GYROSCOPE_SENSOR;
	else if (strcmp(argv[1], "pressure") == 0)
		 type = PRESSURE_SENSOR;
	else if (strcmp(argv[1], "temperature") == 0)
		 type = TEMPERATURE_SENSOR;
	else if (strcmp(argv[1], "geomagnetic") == 0)
		 type = GEOMAGNETIC_SENSOR;
	else if (strcmp(argv[1], "orientation") == 0)
		 type = ORIENTATION_SENSOR;
	else if (strcmp(argv[1], "gravity") == 0)
		 type = GRAVITY_SENSOR;
	else if (strcmp(argv[1], "linear_accel") == 0)
		 type = LINEAR_ACCEL_SENSOR;
	else if (strcmp(argv[1], "rotation_vector") == 0)
		 type = ROTATION_VECTOR_SENSOR;
	else if (strcmp(argv[1], "gaming_rotation_vector") == 0)
		 type = GEOMAGNETIC_RV_SENSOR;
	else if (strcmp(argv[1], "light") == 0)
		 type = LIGHT_SENSOR;
	else if (strcmp(argv[1], "proximity") == 0)
		 type = PROXIMITY_SENSOR;
	else
		 printformat();

	event_condition->cond_value1 = default_event_interval;

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
		case(GYROSCOPE_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_gyro, NULL);
		break;
		case(PRESSURE_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_pressure, NULL);
		break;
		case(GEOMAGNETIC_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_geo, NULL);
		break;
		case(LIGHT_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_light, NULL);
		break;
		case(TEMPERATURE_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_temperature, NULL);
		break;
		case(PROXIMITY_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_proxi, NULL);
		break;
		case(ORIENTATION_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_orientation, NULL);
		break;
		case(GRAVITY_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_gravity, NULL);
		break;
		case(LINEAR_ACCEL_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_linear_accel, NULL);
		break;
		case(ROTATION_VECTOR_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_rot_vec, NULL);
		break;
		case(GEOMAGNETIC_RV_SENSOR):
			result = sf_register_event(handle, event, event_condition, callback_geomag_rv, NULL);
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
