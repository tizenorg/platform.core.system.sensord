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

#include <sensor_hal.h>
#include <sensor_types.h>
#include <stdint.h>

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*
typedef union {
	struct {
		sensor_type_t type;
		int32_t id;
	} __attribute__((packed));
	int64_t id;
} sensor_id_t;
*/
typedef int64_t sensor_id_t;

typedef void *sensor_t;

typedef enum {
	SENSOR_PRIVILEGE_PUBLIC,
	SENSOR_PRIVILEGE_INTERNAL,
} sensor_privilege_t;

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


#endif /* __SENSOR_COMMON_H__ */
