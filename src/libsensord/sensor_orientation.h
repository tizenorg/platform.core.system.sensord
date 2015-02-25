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

#ifndef __SENSOR_ORIENTATION_H__
#define __SENSOR_ORIENTATION_H__

//! Pre-defined events for the orientation sensor
//! Sensor Plugin developer can add more event to their own headers

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup SENSOR_ORIENTATION Orientation Sensor
 * @ingroup SENSOR_FRAMEWORK
 *
 * These APIs are used to control the Orientation sensor.
 * @{
 */

enum orientation_event_type {
	ORIENTATION_RAW_DATA_EVENT		= (ORIENTATION_SENSOR << 16) | 0x0001,
	ORIENTATION_CALIBRATION_NEEDED_EVENT			= (ORIENTATION_SENSOR << 16) | 0x0002,
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
//! End of a file
