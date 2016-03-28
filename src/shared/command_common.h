/*
 * sensord
 *
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

#ifndef _COMMAND_COMMON_H_
#define _COMMAND_COMMON_H_

#include <cpacket.h>
#include <sensor_common.h>

#define COMMAND_CHANNEL_PATH		"/tmp/sensord_command_socket\0"
#define EVENT_CHANNEL_PATH			"/tmp/sensord_event_socket\0"

#define MAX_HANDLE			256
#define MAX_HANDLE_REACHED	-2

enum packet_type_t {
	CMD_DONE = -1,
	CMD_NONE = 0,
	CMD_GET_ID,
	CMD_GET_SENSOR_LIST,
	CMD_HELLO,
	CMD_BYEBYE,
	CMD_START,
	CMD_STOP,
	CMD_REG,
	CMD_UNREG,
	CMD_SET_OPTION,
	CMD_SET_BATCH,
	CMD_UNSET_BATCH,
	CMD_GET_DATA,
	CMD_SET_ATTRIBUTE_INT,
	CMD_SET_ATTRIBUTE_STR,
	CMD_FLUSH,
	CMD_CNT,
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
	int extra_len;
	char extra_data[];
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


typedef struct  {
	int attribute;
	int value;
} cmd_set_attribute_int_t;

typedef struct  {
	int attribute;
	int value_len;
	char value[0];
} cmd_set_attribute_str_t;

typedef struct {
} cmd_flush_t;

#define CHANNEL_MAGIC_NUM 0xCAFECAFE

typedef struct {
	unsigned int magic;
	int client_id;
} channel_ready_t;

typedef void *(*cmd_func_t)(void *data, void *cb_data);

#endif /* _COMMAND_COMMON_H_ */
