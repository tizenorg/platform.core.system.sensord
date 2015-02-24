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

#define TEMPERATURE_BASE_DATA_SET (TEMPERATURE_SENSOR << 16) | 0x0001
#define TEMPERATURE_EVENT_RAW_DATA_REPORT_ON_TIME (TEMPERATURE_SENSOR << 16) | 0x0001

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

