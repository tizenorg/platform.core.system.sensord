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

#ifndef __SENSOR_COMMON_H__
#define __SENSOR_COMMON_H__

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

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

typedef unsigned int sensor_id_t;

typedef void *sensor_t;

typedef enum {
	SENSOR_PRIVILEGE_PUBLIC,
	SENSOR_PRIVILEGE_INTERNAL,
} sensor_privilege_t;


#define SENSOR_DATA_VALUE_SIZE 16

/*
 *	When modifying it, check copy_sensor_data()
 */
typedef struct sensor_data_t {
/*
 * 	Use "accuracy" instead of "data_accuracy"
 * 	which is going to be removed soon
 */
	union {
		int accuracy;
		int data_accuracy; //deprecated
	};

	union {
		unsigned long long timestamp;
		unsigned long long time_stamp; //deprecated
	};

/*
 * 	Use "value_count" instead of "values_num"
 * 	which is going to be removed soon
 */
	union {
		int value_count;
		int values_num; //deprecated
	};

	float values[SENSOR_DATA_VALUE_SIZE];

/*
 * 	If extra_data_size > 0,
 * 	then use extra_data.
 */
	int extra_data_size;
	void *extra_data;
} sensor_data_t;

#define SENSOR_HUB_DATA_SIZE	4096

typedef struct sensorhub_data_t {
    int version;
    int sensorhub;
    int type;
    int hub_data_size;
    unsigned long long timestamp;
    char hub_data[SENSOR_HUB_DATA_SIZE];
    float data[16];
} sensorhub_data_t;

enum sensor_accuracy_t {
	SENSOR_ACCURACY_UNDEFINED = -1,
	SENSOR_ACCURACY_BAD = 0,
	SENSOR_ACCURACY_NORMAL =1,
	SENSOR_ACCURACY_GOOD = 2,
	SENSOR_ACCURACY_VERYGOOD = 3
};

/*
 *	To prevent naming confliction as using same enums as sensor CAPI use
 */
#ifndef __SENSOR_H__
enum sensor_option_t {
	SENSOR_OPTION_DEFAULT = 0,
	SENSOR_OPTION_ON_IN_SCREEN_OFF = 1,
	SENSOR_OPTION_ON_IN_POWERSAVE_MODE = 2,
	SENSOR_OPTION_ALWAYS_ON = SENSOR_OPTION_ON_IN_SCREEN_OFF | SENSOR_OPTION_ON_IN_POWERSAVE_MODE,
	SENSOR_OPTION_END
};

typedef enum sensor_option_t sensor_option_e;
#endif

/*
 *	To prevent naming confliction as using same enums as sensor CAPI use
 */
#ifndef __SENSOR_H__
enum sensor_wakeup_t {
	SENSOR_WAKEUP_UNKNOWN = -1,
	SENSOR_WAKEUP_OFF = 0,
	SENSOR_WAKEUP_ON = 1,
};

typedef enum sensor_wakeup_t sensor_wakeup_e;
#endif

enum sensor_interval_t {
	SENSOR_INTERVAL_FASTEST = 0,
	SENSOR_INTERVAL_NORMAL = 200,
};


typedef enum {
	CONDITION_NO_OP,
	CONDITION_EQUAL,
	CONDITION_GREAT_THAN,
	CONDITION_LESS_THAN,
} condition_op_t;

#ifdef __cplusplus
}
#endif


#endif
//! End of a file
