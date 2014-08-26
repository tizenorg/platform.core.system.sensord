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

#ifndef _SENSOR_ORIENTATION_H_
#define _SENSOR_ORIENTATION_H_

//! Pre-defined events for the orientation sensor
//! Sensor Plugin developer can add more event to their own headers

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

/**
 * @defgroup SENSOR_ORIENTATION Orientation Sensor
 * @ingroup SENSOR_FRAMEWORK
 *
 * These APIs are used to control the Orientation sensor.
 * @{
 */

enum orientation_event_type {
	ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME		= (ORIENTATION_SENSOR << 16) | 0x0001,
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_SENSOR_ORIENTATION_H_*/
