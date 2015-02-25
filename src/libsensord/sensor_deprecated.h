/*
 * libsensord
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

#ifndef __SENSOR_DEPRECATED_H__
#define __SENSOR_DEPRECATED_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define ACCELEROMETER_EVENT_ROTATION_CHECK ((ACCELEROMETER_SENSOR << 16) | 0x0100)

#define ACCELEROMETER_ORIENTATION_DATA_SET (ACCELEROMETER_SENSOR << 16) | 0x0002
#define ACCELEROMETER_LINEAR_ACCELERATION_DATA_SET (ACCELEROMETER_SENSOR << 16) | 0x0004
#define ACCELEROMETER_GRAVITY_DATA_SET (ACCELEROMETER_SENSOR << 16) | 0x0008

#define ACCELEROMETER_EVENT_GRAVITY_DATA_REPORT_ON_TIME (ACCELEROMETER_SENSOR << 16) | 0x0080
#define ACCELEROMETER_EVENT_LINEAR_ACCELERATION_DATA_REPORT_ON_TIME (ACCELEROMETER_SENSOR << 16) | 0x0040
#define ACCELEROMETER_EVENT_ORIENTATION_DATA_REPORT_ON_TIME (ACCELEROMETER_SENSOR << 16) | 0x0020
#define GEOMAGNETIC_EVENT_ATTITUDE_DATA_REPORT_ON_TIME (GEOMAGNETIC_SENSOR << 16) | 0x0004
#define ACCELEROMETER_EVENT_CALIBRATION_NEEDED 0x01
#define ACCELEROMETER_EVENT_SET_WAKEUP 0x02

#define TEMPERATURE_BASE_DATA_SET TEMPERATURE_RAW_DATA_EVENT
#define TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME TEMPERATURE_RAW_DATA_EVENT

#define ACCELEROMETER_BASE_DATA_SET ACCELEROMETER_RAW_DATA_EVENT
#define ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME ACCELEROMETER_RAW_DATA_EVENT
#define ACCELEROMETER_EVENT_UNPROCESSED_DATA_REPORT_ON_TIME ACCELEROMETER_UNPROCESSED_DATA_EVENT

#define GYRO_BASE_DATA_SET GYROSCOPE_RAW_DATA_EVENT
#define GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME GYROSCOPE_RAW_DATA_EVENT
#define GYROSCOPE_EVENT_UNPROCESSED_DATA_REPORT_ON_TIME GYROSCOPE_UNPROCESSED_DATA_EVENT

#define PROXIMITY_BASE_DATA_SET PROXIMITY_CHANGE_STATE_EVENT
#define PROXIMITY_DISTANCE_BASE_DATA_SET PROXIMITY_STATE_EVENT
#define PROXIMITY_EVENT_CHANGE_STATE PROXIMITY_CHANGE_STATE_EVENT
#define PROXIMITY_EVENT_STATE_REPORT_ON_TIME PROXIMITY_STATE_EVENT
#define PROXIMITY_EVENT_DISTANCE_DATA_REPORT_ON_TIME PROXIMITY_DISTANCE_DATA_EVENT

#define PRESSURE_BASE_DATA_SET PRESSURE_RAW_DATA_EVENT
#define PRESSURE_EVENT_RAW_DATA_REPORT_ON_TIME PRESSURE_RAW_DATA_EVENT

#define GEOMAGNETIC_BASE_DATA_SET GEOMAGNETIC_RAW_DATA_EVENT
#define GEOMAGNETIC_RAW_DATA_SET GEOMAGNETIC_RAW_DATA_EVENT
#define GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME GEOMAGNETIC_RAW_DATA_EVENT
#define GEOMAGNETIC_EVENT_CALIBRATION_NEEDED GEOMAGNETIC_CALIBRATION_NEEDED_EVENT
#define GEOMAGNETIC_EVENT_UNPROCESSED_DATA_REPORT_ON_TIME GEOMAGNETIC_UNPROCESSED_DATA_EVENT

#define AUTO_ROTATION_BASE_DATA_SET AUTO_ROTATION_CHANGE_STATE_EVENT
#define AUTO_ROTATION_EVENT_CHANGE_STATE AUTO_ROTATION_CHANGE_STATE_EVENT

#define LIGHT_LUX_DATA_SET LIGHT_LUX_DATA_EVENT
#define LIGHT_BASE_DATA_SET LIGHT_LEVEL_DATA_EVENT
#define LIGHT_EVENT_LUX_DATA_REPORT_ON_TIME LIGHT_LUX_DATA_EVENT
#define LIGHT_EVENT_LEVEL_DATA_REPORT_ON_TIME LIGHT_LEVEL_DATA_EVENT
#define LIGHT_EVENT_CHANGE_LEVEL LIGHT_CHANGE_LEVEL_EVENT

#define GRAVITY_BASE_DATA_SET GRAVITY_RAW_DATA_EVENT
#define GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME GRAVITY_RAW_DATA_EVENT

enum accelerometer_rotate_state {
	ROTATION_UNKNOWN = 0,
	ROTATION_LANDSCAPE_LEFT = 1,
	ROTATION_PORTRAIT_TOP = 2,
	ROTATION_PORTRAIT_BTM = 3,
	ROTATION_LANDSCAPE_RIGHT = 4,
	ROTATION_EVENT_0 = 2,
	ROTATION_EVENT_90 = 1,
	ROTATION_EVENT_180 = 3,
	ROTATION_EVENT_270 = 4,
};

#ifdef __cplusplus
}
#endif

#endif //__SENSOR_DEPRECATED_H__

