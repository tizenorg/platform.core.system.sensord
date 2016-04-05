/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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
#include <client_common.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <map>

typedef std::map<sensor_type_t, log_attr> sensor_type_map;
static sensor_type_map g_log_maps = {
	{UNKNOWN_SENSOR, {"UNKNOWN", "UNKNOWN_EVENT"}},
	{ACCELEROMETER_SENSOR, {"ACCELEROMETER", "ACCELEROMETER_RAW_DATA_EVENT"}},
	{GRAVITY_SENSOR, {"GRAVITY", "GRAVITY_RAW_DATA_EVENT"}},
	{LINEAR_ACCEL_SENSOR, {"LINEAR_ACCEL", "LINEAR_ACCEL_RAW_DATA_EVENT"}},
	{GEOMAGNETIC_SENSOR, {"GEOMAGNETIC SENSOR", "GEOMAGNETIC SENSOR_RAW_DATA_EVENT"}},
	{ROTATION_VECTOR_SENSOR, {"ROTATION VECTOR", "ROTATION VECTOR_RAW_DATA_EVENT"}},
	{ORIENTATION_SENSOR, {"ORIENTATION", "ORIENTATION_RAW_DATA_EVENT"}},
	{GYROSCOPE_SENSOR, {"GYROSCOPE", "GYROSCOPE_RAW_DATA_EVENT"}},
	{LIGHT_SENSOR, {"LIGHT", "LIGHT_RAW_DATA_EVENT"}},
	{PROXIMITY_SENSOR, {"PROXIMITY", "PROXIMITY_RAW_DATA_EVENT"}},
	{PRESSURE_SENSOR, {"PRESSURE", "PRESSURE_RAW_DATA_EVENT"}},
	{ULTRAVIOLET_SENSOR, {"ULTRAVIOLET", "ULTRAVIOLET_RAW_DATA_EVENT"}},
	{TEMPERATURE_SENSOR, {"TEMPERATURE", "TEMPERATURE_RAW_DATA_EVENT"}},
	{HUMIDITY_SENSOR, {"HUMIDITY", "HUMIDITY_RAW_DATA_EVENT"}},
	{BIO_HRM_SENSOR, {"BIO_HRM", "BIO_HRM_RAW_DATA_EVENT"}},
	{BIO_LED_GREEN_SENSOR, {"BIO_LED_GREEN", "BIO_LED_GREEN_RAW_DATA_EVENT"}},
	{BIO_LED_IR_SENSOR, {"BIO_LED_IR", "BIO_LED_IR_RAW_DATA_EVENT"}},
	{BIO_LED_RED_SENSOR, {"BIO_LED_RED", "BIO_LED_RED_RAW_DATA_EVENT"}},
	{GYROSCOPE_UNCAL_SENSOR, {"GYROSCOPE_UNCAL", "GYROSCOPE_UNCAL_RAW_DATA_EVENT"}},
	{GEOMAGNETIC_UNCAL_SENSOR, {"GEOMAGNETIC_UNCAL", "GEOMAGNETIC_UNCAL_RAW_DATA_EVENT"}},
	{GYROSCOPE_RV_SENSOR, {"GYROSCOPE_RV", "GYROSCOPE_RV_RAW_DATA_EVENT"}},
	{GEOMAGNETIC_RV_SENSOR, {"GEOMAGNETIC_RV", "GEOMAGNETIC_RV_RAW_DATA_EVENT"}},
	{CONTEXT_SENSOR, {"CONTEXT", "CONTEXT_RAW_DATA_EVENT"}},
	{EXERCISE_SENSOR, {"EXERCISE", "EXERCISE_RAW_DATA_EVENT"}},
	{HUMAN_PEDOMETER_SENSOR, {"HUMAN_PEDOMETER", "HUMAN_PEDOMETER_EVENT_CHANGE_STATE"}},
	{HUMAN_SLEEP_MONITOR_SENSOR, {"HUMAN_SLEEP_MONITOR", "HUMAN_SLEEP_MONITOR_EVENT"}},
	{AUTO_ROTATION_SENSOR, {"AUTO_ROTATION", "AUTO_ROTATION_EVENT_CHANGE_STATE"}},
	{GESTURE_MOVEMENT_SENSOR, {"GESTURE_MOVEMENT", "GESTURE_MOVEMENT_EVENT_CHANGE_STATE"}},
	{GESTURE_WRIST_UP_SENSOR, {"GESTURE_WRIST_UP", "GESTURE_WRIST_UP_EVENT_CHANGE_STATE"}},
	{GESTURE_WRIST_DOWN_SENSOR, {"GESTURE_WRIST_DOWN", "GESTURE_WRIST_DOWN_EVENT_CHANGE_STATE"}},
	{GESTURE_MOVEMENT_STATE_SENSOR, {"GESTURE_MOVEMENT_STATE", "GESTURE_MOVEMENT_STATE_EVENT"}},

	{WEAR_STATUS_SENSOR, {"WEAR_STATUS", "WEAR_STATUS_EVENT_CHANGE_STATE"}},
	{WEAR_ON_MONITOR_SENSOR, {"WEAR_ON_MONITOR", "WEAR_ON_EVENT_CHANGE_STATE"}},
	{GPS_BATCH_SENSOR, {"GPS_BATCH", "GPS_BATCH_EVENT_CHANGE_STATE"}},
	{ACTIVITY_TRACKER_SENSOR, {"ACTIVITY_TRACKER", "ACTIVITY_TRACKER_EVENT_CHANGE_STATE"}},
	{SLEEP_DETECTOR_SENSOR, {"SLEEP_DETECTOR", "SLEEP_DETECTOR_EVENT_CHANGE_STATE"}},
};

const char* get_sensor_name(sensor_id_t sensor_id)
{
	const char* p_unknown = "UNKNOWN";
	sensor_type_t sensor_type = (sensor_type_t) (sensor_id >> SENSOR_TYPE_SHIFT);

	auto iter = g_log_maps.find(sensor_type);

	if (iter == g_log_maps.end()) {
		_I("Unknown type value: %#x", sensor_type);
		return p_unknown;
	}

	return iter->second.sensor_name;
}

const char* get_event_name(unsigned int event_type)
{
	const char* p_unknown = "UNKNOWN";
	sensor_type_t sensor_type = (sensor_type_t) (event_type >> EVENT_TYPE_SHIFT);

	auto iter = g_log_maps.find(sensor_type);

	if (iter == g_log_maps.end()) {
		_I("Unknown type value: %#x", sensor_type);
		return p_unknown;
	}

	return iter->second.event_name;
}

bool is_one_shot_event(unsigned int event_type)
{
	return false;
}

bool is_ontime_event(unsigned int event_type)
{
	switch (event_type ) {
	case ACCELEROMETER_RAW_DATA_EVENT:
	case PROXIMITY_STATE_EVENT:
	case GYROSCOPE_RAW_DATA_EVENT:
	case LIGHT_LEVEL_DATA_EVENT:
	case GEOMAGNETIC_RAW_DATA_EVENT:
	case LIGHT_LUX_DATA_EVENT:
	case PROXIMITY_DISTANCE_DATA_EVENT:
	case GRAVITY_RAW_DATA_EVENT:
	case LINEAR_ACCEL_RAW_DATA_EVENT:
	case ORIENTATION_RAW_DATA_EVENT:
	case PRESSURE_RAW_DATA_EVENT:
		return true;
		break;
	}

	return false;
}

bool is_panning_event(unsigned int event_type)
{
	return false;
}

bool is_single_state_event(unsigned int event_type)
{
	switch (event_type) {
	case LIGHT_CHANGE_LEVEL_EVENT:
	case PROXIMITY_CHANGE_STATE_EVENT:
	case AUTO_ROTATION_CHANGE_STATE_EVENT:
		return true;
		break;
	}

	return false;
}

unsigned int get_calibration_event_type(unsigned int event_type)
{
	sensor_type_t sensor;

	sensor = (sensor_type_t)(event_type >> EVENT_TYPE_SHIFT);

	switch (sensor) {
	default:
		return 0;
	}
}

unsigned long long get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

void print_event_occurrence_log(sensor_handle_info &sensor_handle_info, const reg_event_info *event_info)
{
	_D("%s receives %s[%d]", get_client_name(),
			get_sensor_name(sensor_handle_info.m_sensor_id), sensor_handle_info.m_handle);
}

/*
 *	To prevent user mistakenly freeing sensor_info using sensor_t
 */
static const int SENSOR_TO_SENSOR_INFO = 4;
static const int SENSOR_INFO_TO_SENSOR = -SENSOR_TO_SENSOR_INFO;

sensor_info *sensor_to_sensor_info(sensor_t sensor)
{
	if (!sensor)
		return NULL;

	sensor_info* info = (sensor_info *)((char *)sensor + SENSOR_TO_SENSOR_INFO);

	return info;
}

sensor_t sensor_info_to_sensor(const sensor_info *info)
{
	if (!info)
		return NULL;

	sensor_t sensor = (sensor_t)((char *)info + SENSOR_INFO_TO_SENSOR);

	return sensor;
}
