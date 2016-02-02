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

#ifdef __cplusplus
#include <string>
#include <vector>
#endif /* __cplusplus */

#define SENSOR_HAL_VERSION(maj,min) \
			((((maj) & 0xffff) << 24) | ((min) & 0xffff))

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

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
	SENSOR_DEVICE_UNKNOWN = -2,
	SENSOR_DEVICE_ALL = -1,
	SENSOR_DEVICE_ACCELEROMETER,
	SENSOR_DEVICE_GRAVITY,
	SENSOR_DEVICE_LINEAR_ACCELERATION,
	SENSOR_DEVICE_GEOMAGNETIC,
	SENSOR_DEVICE_ROTATION_VECTOR,
	SENSOR_DEVICE_ORIENTATION,
	SENSOR_DEVICE_GYROSCOPE,
	SENSOR_DEVICE_LIGHT,
	SENSOR_DEVICE_PROXIMITY,
	SENSOR_DEVICE_PRESSURE,
	SENSOR_DEVICE_ULTRAVIOLET,
	SENSOR_DEVICE_TEMPERATURE,
	SENSOR_DEVICE_HUMIDITY,
	SENSOR_DEVICE_HRM,
	SENSOR_DEVICE_HRM_LED_GREEN,
	SENSOR_DEVICE_HRM_LED_IR,
	SENSOR_DEVICE_HRM_LED_RED,
	SENSOR_DEVICE_GYROSCOPE_UNCAL,
	SENSOR_DEVICE_GEOMAGNETIC_UNCAL,
	SENSOR_DEVICE_GYROSCOPE_RV,
	SENSOR_DEVICE_GEOMAGNETIC_RV,

	SENSOR_DEVICE_ACTIVITY_STATIONARY = 0x100,
	SENSOR_DEVICE_ACTIVITY_WALK,
	SENSOR_DEVICE_ACTIVITY_RUN,
	SENSOR_DEVICE_ACTIVITY_IN_VEHICLE,
	SENSOR_DEVICE_ACTIVITY_ON_BICYCLE,

	SENSOR_DEVICE_GESTURE_MOVEMENT = 0x200,
	SENSOR_DEVICE_GESTURE_WRIST_UP,
	SENSOR_DEVICE_GESTURE_WRIST_DOWN,

	SENSOR_DEVICE_HUMAN_PEDOMETER = 0x300,
	SENSOR_DEVICE_HUMAN_SLEEP_MONITOR,

	SENSOR_DEVICE_FUSION = 0x900,
	SENSOR_DEVICE_AUTO_ROTATION,

	SENSOR_DEVICE_CONTEXT = 0x1000,
	SENSOR_DEVICE_MOTION,
	SENSOR_DEVICE_PIR,
	SENSOR_DEVICE_PIR_LONG,
	SENSOR_DEVICE_DUST,
	SENSOR_DEVICE_THERMOMETER,
	SENSOR_DEVICE_PEDOMETER,
	SENSOR_DEVICE_FLAT,
	SENSOR_DEVICE_HRM_RAW,
	SENSOR_DEVICE_TILT,
	SENSOR_DEVICE_ROTATION_VECTOR_RAW,
} sensor_device_type;

typedef struct sensor_info_t {
	const char *model_name;
	const char *vendor;
	float min_range;
	float max_range;
	float resolution;
	int min_interval;
	int max_batch_count;
	bool wakeup_supported;
} sensor_info_t;

/*
 * A platform sensor handler is generated based on this handle
 * ID can be assigned from HAL developer. so it has to be unique in HAL.
 */
typedef struct sensor_handle_t {
	uint16_t id;
	const char *name;
	sensor_device_type type;
	unsigned int event_type; // for Internal API
	sensor_info_t info;
} sensor_handle_t;

enum sensor_accuracy_t {
	SENSOR_ACCURACY_UNDEFINED = -1,
	SENSOR_ACCURACY_BAD = 0,
	SENSOR_ACCURACY_NORMAL =1,
	SENSOR_ACCURACY_GOOD = 2,
	SENSOR_ACCURACY_VERYGOOD = 3
};

#define SENSOR_DATA_VALUE_SIZE 16

/* sensor_data_t */
typedef struct sensor_data_t {
	int accuracy;
	unsigned long long timestamp;
	int value_count;
	float values[SENSOR_DATA_VALUE_SIZE];
} sensor_data_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
/*
 * Create devices
 */
typedef void *sensor_device_t;
typedef int (*create_t)(sensor_device_t **devices);

/*
 * Sensor device interface
 * 1 device must be abstracted from 1 device event node
 */
class sensor_device
{
public:
	virtual ~sensor_device() {}

	uint32_t get_hal_version(void)
	{
		return SENSOR_HAL_VERSION(1, 0);
	}

	virtual int get_poll_fd(void) = 0;
	virtual int get_sensors(const sensor_handle_t **sensors) = 0;

	virtual bool enable(uint16_t id) = 0;
	virtual bool disable(uint16_t id) = 0;

	virtual bool set_interval(uint16_t id, unsigned long val) = 0;
	virtual bool set_batch_latency(uint16_t id, unsigned long val) = 0;
	virtual bool set_attribute(uint16_t id, int32_t attribute, int32_t value) = 0;

	virtual int read_fd(uint16_t **ids) = 0;
	virtual int get_data(uint16_t id, sensor_data_t **data) = 0;

	virtual bool flush(uint16_t id) = 0;
};
#endif /* __cplusplus */

#endif /* _SENSOR_HAL_H_ */
