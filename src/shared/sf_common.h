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

#ifndef _SF_COMMON_H_
#define _SF_COMMON_H_

#include <unistd.h>
#include <sensor_common.h>
#include <string>
#include <vector>
#include <sensor_logs.h>

#define COMMAND_CHANNEL_PATH			"/tmp/sensord_command_socket"
#define EVENT_CHANNEL_PATH				"/tmp/sensord_event_socket"

#define MAX_HANDLE			256
#define MAX_HANDLE_REACHED	-2

#define CLIENT_ID_INVALID   -1

enum packet_type_t {
	CMD_NONE = 0,
	CMD_GET_ID,
	CMD_GET_SENSOR_LIST,
	CMD_HELLO,
	CMD_BYEBYE,
	CMD_DONE,
	CMD_START,
	CMD_STOP,
	CMD_REG,
	CMD_UNREG,
	CMD_SET_OPTION,
	CMD_SET_WAKEUP,
	CMD_SET_BATCH,
	CMD_UNSET_BATCH,
	CMD_SET_COMMAND,
	CMD_GET_DATA,
	CMD_SEND_SENSORHUB_DATA,
	CMD_CNT,
};

enum sensor_state_t {
	SENSOR_STATE_UNKNOWN = -1,
	SENSOR_STATE_STOPPED = 0,
	SENSOR_STATE_STARTED = 1,
	SENSOR_STATE_PAUSED = 2
};

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

typedef struct {
	char name[NAME_MAX];
} cmd_get_id_t;

typedef struct {
} cmd_get_sensor_list_t;

typedef struct {
	int client_id;
	sensor_id_t sensor;
} cmd_hello_t;

typedef struct {
} cmd_byebye_t;


typedef struct {
	unsigned int type;
} cmd_get_data_t;

typedef struct {
	long value;
} cmd_done_t;


typedef struct {
	int client_id;
} cmd_get_id_done_t;

typedef struct {
	int sensor_cnt;
	char data[0];
} cmd_get_sensor_list_done_t;

typedef struct {
	int state;
	sensor_data_t base_data;
} cmd_get_data_done_t;

typedef struct {
} cmd_start_t;

typedef struct {
} cmd_stop_t;

typedef struct {
	unsigned int event_type;
} cmd_reg_t;

typedef struct {
	unsigned int event_type;
} cmd_unreg_t;

typedef struct {
	unsigned int interval;
	unsigned int latency;
} cmd_set_batch_t;

typedef struct {
} cmd_unset_batch_t;

typedef struct {
	int option;
} cmd_set_option_t;

typedef struct {
	int wakeup;
} cmd_set_wakeup_t;

typedef struct  {
	unsigned int cmd;
	long value;
} cmd_set_command_t;

typedef struct  {
	int data_len;
	int data;
} cmd_send_sensorhub_data_t;

#define EVENT_CHANNEL_MAGIC 0xCAFECAFE

typedef struct {
	unsigned int magic;
	int client_id;
} event_channel_ready_t;

typedef struct sensor_event_t {
	unsigned int event_type;
	sensor_id_t sensor_id;
	unsigned int data_length;
	sensor_data_t *data;
} sensor_event_t;

typedef struct sensorhub_event_t {
	unsigned int event_type;
	sensor_id_t sensor_id;
	unsigned int data_length;
	sensorhub_data_t data;
} sensorhub_event_t;

typedef void *(*cmd_func_t)(void *data, void *cb_data);

typedef std::vector<unsigned int> event_type_vector;

enum sensor_permission_t {
	SENSOR_PERMISSION_NONE	= 0,
	SENSOR_PERMISSION_STANDARD = (1 << 0),
	SENSOR_PERMISSION_BIO	=  (1 << 1),
};

#define BIO_SENSOR_PRIVELEGE_NAME "sensord::bio"
#define BIO_SENSOR_ACCESS_RIGHT "rw"

#endif /* _SF_COMMON_H_ */
