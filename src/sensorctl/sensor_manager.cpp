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

#include <string.h>
#include <macro.h>
#include <sensorctl_log.h>
#include "sensor_manager.h"

#define NAME_MAX_TEST 32

struct sensor_info {
	sensor_type_t type;
	char name[NAME_MAX_TEST];
	/* if manual is true, the way of injecting fake event will be created */
	bool manual;
};

static struct sensor_info sensor_infos[] = {
	{ALL_SENSOR,				"all",				false},

	// General Sensors
	{ACCELEROMETER_SENSOR,		"accelerometer",	false},
	{GRAVITY_SENSOR,			"gravity",			false},
	{LINEAR_ACCEL_SENSOR,		"linear_accel",		false},
	{GEOMAGNETIC_SENSOR,		"magnetic",			false},
	{ROTATION_VECTOR_SENSOR,	"rotation_vector",	false},
	{ORIENTATION_SENSOR,		"orientation",		false},
	{GYROSCOPE_SENSOR,			"gyroscope",		false},
	{LIGHT_SENSOR,				"light",			false},
	{PROXIMITY_SENSOR,			"proximity",		true},
	{PRESSURE_SENSOR,			"pressure",			false},
	{ULTRAVIOLET_SENSOR,		"ultraviolet",		false},
	{TEMPERATURE_SENSOR,		"temperature",		false},
	{HUMIDITY_SENSOR,			"humidity",			false},
	{HRM_RAW_SENSOR,			"hrm_raw",			false},
	{HRM_SENSOR,				"hrm",				false},
	{HRM_LED_GREEN_SENSOR,		"hrm_led_green",	false},
	{HRM_LED_IR_SENSOR,			"hrm_led_ir",		false},
	{HRM_LED_RED_SENSOR,		"hrm_led_red",		false},
	{GYROSCOPE_UNCAL_SENSOR,	"gyro_uncal",		false},
	{GEOMAGNETIC_UNCAL_SENSOR,	"mag_uncal",		false},
	{GYROSCOPE_RV_SENSOR,		"gyro_rv",			false},
	{GEOMAGNETIC_RV_SENSOR,		"mag_rv",			false},

	{HUMAN_PEDOMETER_SENSOR,	"pedo",				true},
	{HUMAN_SLEEP_MONITOR_SENSOR,"sleep_monitor",	true},

	/*
	{AUTO_ROTATION_SENSOR,		"auto_rotation",	true},
	{AUTO_BRIGHTNESS_SENSOR,	"auto_brightness",	true},
	{MOTION_SENSOR,				"motion",			true},
	{PIR_SENSOR, 				"pir",				true},
	{PIR_LONG_SENSOR,			"pir_long",			true},
	{DUST_SENSOR,				"dust",				false},
	{THERMOMETER_SENSOR, 		"thermometer",		false},
	{FLAT_SENSOR,				"flat",				true},
	{TILT_SENSOR, 				"tilt",				false},
	{RV_RAW_SENSOR,				"rv_raw",			false},
	{EXERCISE_SENSOR,			"exercise",			false},
	{GSR_SENSOR, 				"gsr",				false},
	{SIMSENSE_SENSOR,			"simsense",			false},
	{PPG_SENSOR,				"ppg",				false},

	{GESTURE_MOVEMENT_SENSOR,	"movement",			true},
	{GESTURE_WRIST_UP_SENSOR,	"wristup",			true},
	{GESTURE_WRIST_DOWN_SENSOR,	"wristdown",		true},
	{GESTURE_MOVEMENT_STATE_SENSOR, "movement_state",true},

	{WEAR_STATUS_SENSOR,		"wear_status",		true},
	{WEAR_ON_MONITOR_SENSOR,	"wear_on",			true},
	{GPS_BATCH_SENSOR,			"gps_batch",		true},
	{ACTIVITY_TRACKER_SENSOR,	"activity_tracker",	true},
	{SLEEP_DETECTOR_SENSOR,		"sleep_detector",	true},
	{NO_MOVE_DETECTOR_SENSOR,	"no_move",			true},
	{HRM_CTRL_SENSOR,			"hrm_ctrl",			true},
	{EXERCISE_COACH_SENSOR,		"ex_coach",			true},
	{EXERCISE_HR_SENSOR,		"ex_hr",			true},
	{RESTING_HR_SENSOR,			"resting_hr",		true},
	{STEP_LEVEL_MONITOR_SENSOR,	"step_level",		true},
	{ACTIVITY_LEVEL_MONITOR_SENSOR,	"activity",		true},
	{CYCLE_MONITOR_SENSOR,		"cycle",			true},
	{STRESS_MONITOR_SENSOR,		"stress",			true},
	{AUTOSESSION_EXERCISE_SENSOR,"autosesstion",	true},
	{STAIR_TRACKER_SENSOR,		"stair_tracker",	true}
	*/

};

bool sensor_manager::process(int argc, char *argv[])
{
	return false;
}

void sensor_manager::usage_sensors(void)
{
	PRINT("The sensor types are:\n");
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (int i = 0; i < sensor_count; ++i)
		PRINT("  %d: %s(%d)\n", i, sensor_infos[i].name, sensor_infos[i].type);
	PRINT("\n");
}

sensor_type_t sensor_manager::get_sensor_type(char *name)
{
	int index;
	int sensor_count = ARRAY_SIZE(sensor_infos);

	if (is_number(name))
		return (sensor_type_t) (atoi(name));

	for (index = 0; index < sensor_count; ++index) {
		if (!strcmp(sensor_infos[index].name, name))
			break;
	}

	if (index == sensor_count) {
		_E("ERROR: sensor name is wrong\n");
		usage_sensors();
		return UNKNOWN_SENSOR;
	}
	return sensor_infos[index].type;
}

const char *sensor_manager::get_sensor_name(sensor_type_t type)
{
	int index;
	int sensor_count = ARRAY_SIZE(sensor_infos);

	for (index = 0; index < sensor_count; ++index) {
		if (sensor_infos[index].type == type)
			break;
	}

	if (index == sensor_count) {
		_E("ERROR: sensor name is wrong\n");
		usage_sensors();
		return "UNKNOWN SENSOR";
	}
	return sensor_infos[index].name;
}

bool sensor_manager::is_number(char *value)
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
