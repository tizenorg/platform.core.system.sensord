/*
 * sensorctl
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "macro.h"
#include "sensor_manager.h"

struct sensor_info {
	sensor_type_t type;
	char name[NAME_MAX_TEST];
};

static struct sensor_info sensor_infos[] = {
	{ALL_SENSOR,				"all"},

	// General Sensors
	{ACCELEROMETER_SENSOR,			"accelerometer"},
	{GRAVITY_SENSOR,				"gravity"},
	{LINEAR_ACCEL_SENSOR,			"linear_accel"},
	{GEOMAGNETIC_SENSOR,			"magnetic"},
	{ROTATION_VECTOR_SENSOR,		"rotation_vector"},
	{ORIENTATION_SENSOR,			"orientation"},
	{GYROSCOPE_SENSOR,				"gyroscope"},
	{LIGHT_SENSOR,					"light"},
	{PROXIMITY_SENSOR,				"proximity"},
	{PRESSURE_SENSOR,				"pressure"},
	{ULTRAVIOLET_SENSOR,			"uv"},
	{TEMPERATURE_SENSOR,			"temperature"},
	{HUMIDITY_SENSOR,				"humidity"},
	{HRM_SENSOR,					"hrm"},
	{HRM_RAW_SENSOR,				"hrm_raw"},
	{HRM_LED_GREEN_SENSOR,			"hrm_led_green"},
	{HRM_LED_IR_SENSOR,				"hrm_led_ir"},
	{HRM_LED_RED_SENSOR,			"hrm_led_red"},
	{GYROSCOPE_UNCAL_SENSOR,		"gyro_uncal"},
	{GEOMAGNETIC_UNCAL_SENSOR,		"mag_uncal"},
	{GYROSCOPE_RV_SENSOR,			"gyro_rv"},
	{GEOMAGNETIC_RV_SENSOR,			"mag_rv"},

	{HUMAN_PEDOMETER_SENSOR			"pedo"},
	{HUMAN_SLEEP_MONITOR_SENSOR,	"sleep_monitor"},

	{AUTO_ROTATION_SENSOR,			"auto_rotation"},
	{AUTO_BRIGHTENESS_SENSOR,		"auto_brighteness"},
	{MOTION_SENSOR,					"motion"},
	{CONTEXT_SENSOR,				"context"},
	{EXERCISE_SENSOR,				"exercise"},

	{GESTURE_MOVEMENT_SENSOR,		"movement"},
	{GESTURE_WRIST_UP_SENSOR,		"wristup"},
	{GESTURE_WRIST_DOWN_SENSOR,		"wristdown"},
	{GESTURE_MOVEMENT_STATE_SENSOR,	"movement_state"},

	{WEAR_STATUS_SENSOR, 			"wear_status"},
	{WEAR_ON_MONITOR_SENSOR, 		"wear_on"},
	{GPS_BATCH_SENSOR, 				"gps"},
	{ACTIVITY_TRACKER_SENSOR, 		"activity"},
	{SLEEP_DETECTOR_SENSOR, 		"sleep_detector"},
};

mainloop& instance(void)
{
	static mainloop loop;
	return loop;
}

void mainloop::start_loop(void)
{
	if (is_loop_running())
		return;

	m_mainloop = g_main_loop_new(NULL, false);
	g_main_loop_run(mainloop);
	m_running = true;
}

void mainloop::stop_loop(void)
{
	if (!is_loop_running())
		return;

	g_main_loop_quit(mainloop);
	g_main_loop_unref(mainloop);
	m_mainloop = NULL;
	m_running = false;
}

bool mainloop::is_loop_running(void)
{
	return m_running;
}

bool sensor_manager::run(void)
{
	return true;
}

sensor_type_t sensor_manager::get_sensor_type(const char *name)
{
	int index;
	int count;

	if (is_hex(name))
		return (sensor_type_t) (strtol(name, NULL, 16));

	if (is_number(name))
		return (sensor_type_t) (atoi(name));

	count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < count; ++index) {
		if (!strcmp(sensor_infos[index].name, name))
			break;
	}

	if (index == count) {
		_E("Invaild sensor name\n");
		usage_sensors();
		return UNKNOWN_SENSOR;
	}
	return sensor_infos[index].type;
}

const char *sensor_manager::get_sensor_name(sensor_type_t type)
{
	int index;
	int count;

	count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < count; ++index) {
		if (sensor_infos[index].type == type)
			break;
	}

	if (index == count) {
		_E("Invaild sensor name\n");
		usage_sensors();
		return "UNKNOWN SENSOR";
	}
	return sensor_infos[index].name;
}

bool sensor_manager::is_number(const char *value)
{
	if (value == NULL || *value == 0)
		return false;

	while (*value) {
		if (*value < '0' || *value > '9')
			return false;
		value++;
	}

	return true;
}

bool sensor_manager::is_hex(const char *value)
{
	if (value == NULL || *value == 0)
		return false;

	if (value[0] != '0')
		return false;

	if (value[1] != 'x' || value[1] != 'X')
		return false;

	value += 2;

	while (*value) {
		if ((*value < '0' || *value > '9') &&
			(*value < 'a' || *value > 'f') &&
			(*value < 'A' || *value > 'F'))
			return false;
		value++;
	}

	return true;
}

void sensor_manager::usage_sensors(void)
{
	PRINT("The sensor types are:");
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (int i = 0; i < sensor_count; ++i)
		PRINT("%3d: %s(%#x)\n", i, sensor_infos[i].name, sensor_infos[i].type);
	PRINT("\n");
}

