/*
 * libsensord-share
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

#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <sensor_common.h>
#include <sf_common.h>

#define SENSOR_HAL_VERSION(maj,min) \
			((((maj) & 0xffff) << 24) | ((min) & 0xffff))

/*
 * Sensor Types
 * These types are used to controll the sensors
 *
 * - base unit
 *   acceleration values : meter per second^2 (m/s^2)
 *   magnetic values     : micro-Tesla (uT)
 *   orientation values  : degrees
 *   gyroscope values    : degree/s
 *   temperature values  : degrees centigrade
 *   proximity valeus    : distance
 *   light values        : lux
 *   pressure values     : hectopascal (hPa)
 *   humidity            : relative humidity (%)
 */
typedef enum {
	SENSOR_HAL_TYPE_UNKNOWN = -2,
	SENSOR_HAL_TYPE_ALL = -1,
	SENSOR_HAL_TYPE_ACCELEROMETER,
	SENSOR_HAL_TYPE_GRAVITY,
	SENSOR_HAL_TYPE_LINEAR_ACCELERATION,
	SENSOR_HAL_TYPE_GEOMAGNETIC,
	SENSOR_HAL_TYPE_ROTATION_VECTOR,
	SENSOR_HAL_TYPE_ORIENTATION,
	SENSOR_HAL_TYPE_GYROSCOPE,
	SENSOR_HAL_TYPE_LIGHT,
	SENSOR_HAL_TYPE_PROXIMITY,
	SENSOR_HAL_TYPE_PRESSURE,
	SENSOR_HAL_TYPE_ULTRAVIOLET,
	SENSOR_HAL_TYPE_TEMPERATURE,
	SENSOR_HAL_TYPE_HUMIDITY,
	SENSOR_HAL_TYPE_HRM,
	SENSOR_HAL_TYPE_HRM_LED_GREEN,
	SENSOR_HAL_TYPE_HRM_LED_IR,
	SENSOR_HAL_TYPE_HRM_LED_RED,
	SENSOR_HAL_TYPE_GYROSCOPE_UNCAL,
	SENSOR_HAL_TYPE_GEOMAGNETIC_UNCAL,
	SENSOR_HAL_TYPE_GYROSCOPE_RV,
	SENSOR_HAL_TYPE_GEOMAGNETIC_RV,

	SENSOR_HAL_TYPE_ACTIVITY_STATIONARY = 0x100,
	SENSOR_HAL_TYPE_ACTIVITY_WALK,
	SENSOR_HAL_TYPE_ACTIVITY_RUN,
	SENSOR_HAL_TYPE_ACTIVITY_IN_VEHICLE,
	SENSOR_HAL_TYPE_ACTIVITY_ON_BICYCLE,

	SENSOR_HAL_TYPE_GESTURE_MOVEMENT = 0x200,
	SENSOR_HAL_TYPE_GESTURE_WRIST_UP,
	SENSOR_HAL_TYPE_GESTURE_WRIST_DOWN,

	SENSOR_HAL_TYPE_HUMAN_PEDOMETER = 0x300,
	SENSOR_HAL_TYPE_HUMAN_SLEEP_MONITOR,

	SENSOR_HAL_TYPE_FUSION = 0x900,
	SENSOR_HAL_TYPE_AUTO_ROTATION,

	SENSOR_HAL_TYPE_CONTEXT = 0x1000,
	SENSOR_HAL_TYPE_MOTION,
	SENSOR_HAL_TYPE_PIR,
	SENSOR_HAL_TYPE_PIR_LONG,
	SENSOR_HAL_TYPE_DUST,
	SENSOR_HAL_TYPE_THERMOMETER,
	SENSOR_HAL_TYPE_PEDOMETER,
	SENSOR_HAL_TYPE_FLAT,
	SENSOR_HAL_TYPE_HRM_RAW,
	SENSOR_HAL_TYPE_TILT,
	SENSOR_HAL_TYPE_ROTATION_VECTOR_RAW,
} sensor_hal_type;

/*
 * A platform sensor handler is generated based on this handle
 * ID can be assigned from HAL developer. so it has to be unique in HAL.
 */
typedef struct sensor_handle_t {
	uint32_t id;
	std::string name;
	sensor_hal_type type;
	unsigned int event_type; // for Internal API
	sensor_properties_s properties;
} sensor_handle_t;

/*
 * Sensor HAL interface
 * 1 HAL must be abstracted from 1 device event node
 */
class sensor_hal
{
public:
	uint32_t get_hal_version(void) {
		return SENSOR_HAL_VERSION(1, 0);
	}

	virtual int get_poll_fd(void) = 0;
	virtual bool get_sensors(std::vector<sensor_handle_t> &sensors) = 0;

	/* enable/disable sensor device */
	virtual bool enable(uint32_t id) = 0;
	virtual bool disable(uint32_t id) = 0;

	/* set_command or set_option? */
	virtual bool set_command(uint32_t id, std::string command, std::string value) = 0;

	/* the belows can be merged to one */
	virtual bool set_interval(uint32_t id, unsigned long val) = 0;
	virtual bool set_batch_latency(uint32_t id, unsigned long val) = 0;

	/* sensor fw read the data when is_data_ready() is true */
	virtual bool is_data_ready() = 0;
	virtual bool get_sensor_data(uint32_t id, sensor_data_t &data) = 0;
	virtual int get_sensor_event(uint32_t id, sensor_event_t **event) = 0;

	/* TODO: use get_sensors() instead of get_properties() */
	virtual bool get_properties(uint32_t id, sensor_properties_s &properties) = 0;
};
#endif /*_SENSOR_HAL_H_*/
