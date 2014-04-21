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

#ifndef __SENSOR_MOTION_H__
#define __SENSOR_MOTION_H__

//! Pre-defined events for the motion sensor
//! Sensor Plugin developer can add more event to their own headers

#ifdef __cplusplus
extern "C"
{
#endif

enum motion_event_type {
	MOTION_ENGINE_EVENT_SNAP				= (MOTION_SENSOR << 16) | 0x0001,
	MOTION_ENGINE_EVENT_SHAKE				= (MOTION_SENSOR << 16) | 0x0002,
	MOTION_ENGINE_EVENT_DOUBLETAP			= (MOTION_SENSOR << 16) | 0x0004,
	MOTION_ENGINE_EVENT_DIRECT_CALL			= (MOTION_SENSOR << 16) | 0x0020,
};

enum motion_snap_event {
	MOTION_ENGIEN_SNAP_NONE			= 0,
	MOTION_ENGIEN_NEGATIVE_SNAP_X	= 1,
	MOTION_ENGIEN_POSITIVE_SNAP_X	= 2,
	MOTION_ENGIEN_NEGATIVE_SNAP_Y	= 3,
	MOTION_ENGIEN_POSITIVE_SNAP_Y	= 4,
	MOTION_ENGIEN_NEGATIVE_SNAP_Z	= 5,
	MOTION_ENGIEN_POSITIVE_SNAP_Z	= 6,
	MOTION_ENGIEN_SNAP_LEFT			= MOTION_ENGIEN_NEGATIVE_SNAP_X,
	MOTION_ENGIEN_SNAP_RIGHT		= MOTION_ENGIEN_POSITIVE_SNAP_X,
	MOTION_ENGINE_SNAP_NONE			= 0,
	MOTION_ENGINE_NEGATIVE_SNAP_X	= 1,
	MOTION_ENGINE_POSITIVE_SNAP_X	= 2,
	MOTION_ENGINE_NEGATIVE_SNAP_Y	= 3,
	MOTION_ENGINE_POSITIVE_SNAP_Y	= 4,
	MOTION_ENGINE_NEGATIVE_SNAP_Z	= 5,
	MOTION_ENGINE_POSITIVE_SNAP_Z	= 6,
	MOTION_ENGINE_SNAP_LEFT			= MOTION_ENGINE_NEGATIVE_SNAP_X,
	MOTION_ENGINE_SNAP_RIGHT		= MOTION_ENGINE_POSITIVE_SNAP_X,
};

enum motion_shake_event {
	MOTION_ENGIEN_SHAKE_NONE		= 0,
	MOTION_ENGIEN_SHAKE_DETECTION	= 1,
	MOTION_ENGIEN_SHAKE_CONTINUING	= 2,
	MOTION_ENGIEN_SHAKE_FINISH		= 3,
	MOTION_ENGINE_SHAKE_BREAK		= 4,
	MOTION_ENGINE_SHAKE_NONE		= 0,
	MOTION_ENGINE_SHAKE_DETECTION	= 1,
	MOTION_ENGINE_SHAKE_CONTINUING	= 2,
	MOTION_ENGINE_SHAKE_FINISH		= 3,
};

enum motion_doubletap_event {
	MOTION_ENGIEN_DOUBLTAP_NONE			= 0,
	MOTION_ENGIEN_DOUBLTAP_DETECTION	= 1,
	MOTION_ENGINE_DOUBLTAP_NONE			= 0,
	MOTION_ENGINE_DOUBLTAP_DETECTION	= 1,
};

enum motion_direct_call_event_t {
	MOTION_ENGINE_DIRECT_CALL_NONE,
	MOTION_ENGINE_DIRECT_CALL_DETECTION,
};

enum motion_property_id {
	MOTION_PROPERTY_UNKNOWN = 0,
	MOTION_PROPERTY_CHECK_ACCEL_SENSOR,
	MOTION_PROPERTY_CHECK_GYRO_SENSOR,
	MOTION_PROPERTY_CHECK_GEO_SENSOR,
	MOTION_PROPERTY_CHECK_PRIXI_SENSOR,
	MOTION_PROPERTY_CHECK_LIGHT_SENSOR,
	MOTION_PROPERTY_CHECK_BARO_SENSOR,
	MOTION_PROPERTY_LCD_TOUCH_ON,
	MOTION_PROPERTY_LCD_TOUCH_OFF,
	MOTION_PROPERTY_CHECK_GYRO_CAL_STATUS,
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
//! End of a file
