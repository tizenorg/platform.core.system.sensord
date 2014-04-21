/*
 * libsensord-share
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

#ifndef _SENSOR_COMMON_H_
#define _SENSOR_COMMON_H_

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

#define MAX_KEY_LENGTH 64
#define MAX_VALUE_SIZE 12
#define SENSORHUB_TYPE_CONTEXT	1
#define SENSORHUB_TYPE_GESTURE	2
#define MS_TO_SEC 1000
#define US_TO_SEC 1000000
#define NS_TO_SEC 1000000000

/**
 * @defgroup SENSOR_FRAMEWORK SensorFW
 * To support the unified API for the various sensors
 */

/**
 * @defgroup SENSOR_FRAMEWORK_COMMON Sensor Framework Common API
 * @ingroup SENSOR_FRAMEWORK
 *
 * These APIs are used to control the sensors.
 * @{
 */

typedef enum {
	UNKNOWN_SENSOR = 0,
	ACCELEROMETER_SENSOR,
	GEOMAGNETIC_SENSOR,
	LIGHT_SENSOR,
	PROXIMITY_SENSOR,
	THERMOMETER_SENSOR,
	GYROSCOPE_SENSOR,
	PRESSURE_SENSOR,
	MOTION_SENSOR,
	FUSION_SENSOR,
	PEDOMETER_SENSOR,
	CONTEXT_SENSOR,
	FLAT_SENSOR,
	BIO_SENSOR,
	BIO_HRM_SENSOR,
	AUTO_ROTATION_SENSOR,
	GRAVITY_SENSOR,
	LINEAR_ACCEL_SENSOR,
	ROTATION_VECTOR_SENSOR,
	ORIENTATION_SENSOR,
} sensor_type_t;

typedef struct sensor_data_t {
	int data_accuracy;
	int data_unit_idx;
/*
 *	Use "timestamp" instead of "time_stamp"
 *	which is going to be removed soon
 */
	union {
		unsigned long long time_stamp;
		unsigned long long timestamp;
	};
	int values_num;
	float values[MAX_VALUE_SIZE];
} sensor_data_t;

typedef struct sensor_event_t {
	unsigned int event_type;
	int situation;
	sensor_data_t data;
} sensor_event_t;

#define HUB_DATA_MAX_SIZE	4096

typedef struct {
    int version;
    int sensorhub;
    int type;
    int hub_data_size;
    long long timestamp;
    char hub_data[HUB_DATA_MAX_SIZE];
    float data[16];
} sensorhub_data_t;

typedef struct sensorhub_event_t {
	unsigned int event_type;
	int situation;
	sensorhub_data_t data;
} sensorhub_event_t;

typedef struct {
	int sensor_unit_idx;
	float sensor_min_range;
	float sensor_max_range;
	float sensor_resolution;
	char sensor_name[MAX_KEY_LENGTH];
	char sensor_vendor[MAX_KEY_LENGTH];
} sensor_properties_t;

enum sensor_data_unit_idx {
	SENSOR_UNDEFINED_UNIT,
	SENSOR_UNIT_METRE_PER_SECOND_SQUARED,
	SENSOR_UNIT_MICRO_TESLA,
	SENSOR_UNIT_DEGREE,
	SENSOR_UNIT_LUX,
	SENSOR_UNIT_CENTIMETER,
	SENSOR_UNIT_LEVEL,
	SENSOR_UNIT_STATE_ON_OFF,
	SENSOR_UNIT_DEGREE_PER_SECOND,
	SENSOR_UNIT_HECTOPASCAL,
	SENSOR_UNIT_CELSIUS,
	SENSOR_UNIT_METER,
	SENSOR_UNIT_STEP,
	SENSOR_UNIT_VENDOR_UNIT = 100,
	SENSOR_UNIT_FILTER_CONVERTED,
	SENSOR_UNIT_SENSOR_END
};

enum sensor_data_accuracy {
	SENSOR_ACCURACY_UNDEFINED = -1,
	SENSOR_ACCURACY_BAD = 0,
	SENSOR_ACCURACY_NORMAL =1,
	SENSOR_ACCURACY_GOOD = 2,
	SENSOR_ACCURACY_VERYGOOD = 3
};

enum sensor_start_option {
	SENSOR_OPTION_DEFAULT = 0,
	SENSOR_OPTION_ALWAYS_ON = 1,
	SENSOR_OPTION_END
};

enum _sensor_current_state {
	SENSOR_STATE_UNKNOWN = -1,
	SENSOR_STATE_STOPPED = 0,
	SENSOR_STATE_STARTED = 1,
	SENSOR_STATE_PAUSED = 2
};

enum _sensor_wakeup_state {
	SENSOR_WAKEUP_UNKNOWN = -1,
	SENSOR_WAKEUP_UNSETTED = 0,
	SENSOR_WAKEUP_SETTED = 1,
};

enum _sensor_poweroff_state {
	SENSOR_POWEROFF_UNKNOWN = -1,
	SENSOR_POWEROFF_AWAKEN  =  1,
};

enum event_situation {
	SITUATION_LCD_ON,
	SITUATION_LCD_OFF,
	SITUATION_SURVIVAL_MODE
};

enum poll_value_t {
	POLL_100HZ_MS	= 10,
	POLL_50HZ_MS	= 20,
	POLL_25HZ_MS	= 40,
	POLL_20HZ_MS	= 50,
	POLL_10HZ_MS	= 100,
	POLL_5HZ_MS		= 200,
	POLL_1HZ_MS		= 1000,
	POLL_MAX_HZ_MS  = POLL_1HZ_MS,
};

typedef enum {
	CONDITION_NO_OP,
	CONDITION_EQUAL,
	CONDITION_GREAT_THAN,
	CONDITION_LESS_THAN,
} condition_op_t;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_SENSOR_COMMON_H_*/
