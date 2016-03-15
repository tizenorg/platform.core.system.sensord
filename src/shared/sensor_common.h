/*
 * sensord
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

#include <stddef.h>
#include <stdint.h>
#include <sensor_hal.h>
#include <sensor_types.h>

#define OP_ERROR -1
#define OP_SUCCESS 0

#define CLIENT_ID_INVALID -1
#define SENSOR_ID_INVALID -1

#define SENSOR_TYPE_SHIFT 32
#define SENSOR_INDEX_MASK 0xFFFFFFFF

#ifndef NAME_MAX
#define NAME_MAX 256
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

enum poll_interval_t {
	POLL_100HZ_MS	= 10,
	POLL_50HZ_MS	= 20,
	POLL_25HZ_MS	= 40,
	POLL_20HZ_MS	= 50,
	POLL_10HZ_MS	= 100,
	POLL_5HZ_MS		= 200,
	POLL_1HZ_MS		= 1000,
	POLL_MAX_HZ_MS  = POLL_1HZ_MS,
};

enum sensor_interval_t {
	SENSOR_INTERVAL_FASTEST = POLL_100HZ_MS,
	SENSOR_INTERVAL_NORMAL = POLL_5HZ_MS,
};

typedef enum {
	CONDITION_NO_OP,
	CONDITION_EQUAL,
	CONDITION_GREAT_THAN,
	CONDITION_LESS_THAN,
} condition_op_t;

enum sensor_state_t {
	SENSOR_STATE_UNKNOWN = -1,
	SENSOR_STATE_STOPPED = 0,
	SENSOR_STATE_STARTED = 1,
	SENSOR_STATE_PAUSED = 2
};

typedef enum {
	SENSOR_PRIVILEGE_PUBLIC,
	SENSOR_PRIVILEGE_INTERNAL,
} sensor_privilege_t;

enum sensor_permission_t {
	SENSOR_PERMISSION_NONE = 0,
	SENSOR_PERMISSION_STANDARD = (1 << 0),
	SENSOR_PERMISSION_BIO = (1 << 1)
};

typedef struct sensor_event_t {
	unsigned int event_type;
	sensor_id_t sensor_id;
	unsigned int data_length;
	sensor_data_t *data;
} sensor_event_t;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <vector>

typedef std::vector<unsigned int> event_type_vector;
#endif /* __cplusplus */

#endif /* __SENSOR_COMMON_H__ */
