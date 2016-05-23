/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#ifndef _SENSOR_HAL_TYPES_H_
#define _SENSOR_HAL_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_HAL_VERSION(maj, min) \
		((((maj) & 0xFFFF) << 24) | ((min) & 0xFFFF))

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

	SENSOR_DEVICE_HUMAN_PEDOMETER = 0x300,
	SENSOR_DEVICE_HUMAN_SLEEP_MONITOR,

	SENSOR_DEVICE_FUSION = 0x900,
	SENSOR_DEVICE_AUTO_ROTATION,
	SENSOR_DEVICE_AUTO_BRIGHTNESS,

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
	SENSOR_DEVICE_EXERCISE,
	SENSOR_DEVICE_GSR,
	SENSOR_DEVICE_SIMSENSE,
	SENSOR_DEVICE_PPG,

	SENSOR_DEVICE_GESTURE_MOVEMENT = 0x1200,
	SENSOR_DEVICE_GESTURE_WRIST_UP,
	SENSOR_DEVICE_GESTURE_WRIST_DOWN,
	SENSOR_DEVICE_GESTURE_MOVEMENT_STATE,

	SENSOR_DEVICE_WEAR_STATUS = 0x1A00,
	SENSOR_DEVICE_WEAR_ON_MONITOR,
	SENSOR_DEVICE_GPS_BATCH,
	SENSOR_DEVICE_ACTIVITY_TRACKER,
	SENSOR_DEVICE_SLEEP_DETECTOR,
	SENSOR_DEVICE_NO_MOVE_DETECTOR = 0x1A80,
	SENSOR_DEVICE_HRM_CTRL,
	SENSOR_DEVICE_EXERCISE_COACH,
	SENSOR_DEVICE_EXERCISE_HR,
	SENSOR_DEVICE_RESTING_HR,
	SENSOR_DEVICE_STEP_LEVEL_MONITOR,
	SENSOR_DEVICE_ACTIVITY_LEVEL_MONITOR,
	SENSOR_DEVICE_CYCLE_MONITOR,
	SENSOR_DEVICE_STRESS_MONITOR,
	SENSOR_DEVICE_AUTOSESSION_EXERCISE,
	SENSOR_DEVICE_STAIR_TRACKER,
} sensor_device_type;

/*
 * A platform sensor handler is generated based on this handle
 * This id can be assigned from HAL developer. so it has to be unique in 1 sensor_device.
 */
typedef struct sensor_info_t {
	uint32_t id;
	const char *name;
	sensor_device_type type;
	unsigned int event_type; // for Internal API
	const char *model_name;
	const char *vendor;
	float min_range;
	float max_range;
	float resolution;
	int min_interval;
	int max_batch_count;
	bool wakeup_supported;
} sensor_info_t;

enum sensor_accuracy_t {
	SENSOR_ACCURACY_UNDEFINED = -1,
	SENSOR_ACCURACY_BAD = 0,
	SENSOR_ACCURACY_NORMAL = 1,
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

#define SENSORHUB_DATA_VALUE_SIZE 4096

/* sensorhub_data_t */
typedef struct sensorhub_data_t {
	int accuracy;
	unsigned long long timestamp;

	/*
	 *  Use "value_count" instead of "hub_data_size"
	 *  which is going to be removed soon
	 */
	union {
		int value_count;
		int hub_data_size; /* deprecated */
	};

	/*
	 *  Use "values" instead of "hub_data"
	 *  which is going to be removed soon
	 */
	union {
		char values[SENSORHUB_DATA_VALUE_SIZE];
		char hub_data[SENSORHUB_DATA_VALUE_SIZE]; /* deprecated */
	};
} sensorhub_data_t;

#define SENSOR_PEDOMETER_DATA_DIFFS_SIZE	20

typedef struct {
	int accuracy;
	unsigned long long timestamp;
	int value_count;	/* value_count == 8 */
	float values[SENSOR_DATA_VALUE_SIZE];
	/* values = {step count, walk step count, run step count,
	             moving distance, calorie burned, last speed,
	             last stepping frequency (steps per sec),
	             last step status (walking, running, ...)} */
	/* Additional data attributes (not in sensor_data_t)*/
	int diffs_count;
	struct differences {
		int timestamp;
		int steps;
		int walk_steps;
		int run_steps;
		int walk_up_steps;
		int walk_down_steps;
		int run_up_steps;
		int run_down_steps;
		float distance;
		float calories;
		float speed;
	} diffs[SENSOR_PEDOMETER_DATA_DIFFS_SIZE];
} sensor_pedometer_data_t;

enum sensor_attribute {
	SENSOR_ATTR_ACTIVITY = 0x100,
};

enum sensor_activity {
	SENSOR_ACTIVITY_UNKNOWN = 1,
	SENSOR_ACTIVITY_STILL = 2,
	SENSOR_ACTIVITY_WALKING = 4,
	SENSOR_ACTIVITY_RUNNING = 8,
	SENSOR_ACTIVITY_IN_VEHICLE = 16,
	SENSOR_ACTIVITY_ON_BICYCLE = 32,
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SENSOR_HAL_TYPES_H_ */
