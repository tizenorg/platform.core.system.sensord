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

#ifndef _SENSOR_BASE_H_
#define _SENSOR_BASE_H_

#include <vector>
#include <sensor_types.h>
#include <plugin_info_list.h>
#include <cmutex.h>

#include <sensor_logs.h>
#include <sensor_common.h>
#include <worker_thread.h>
#include <sensor_info.h>
#include <sensor_hal.h>

class sensor_base
{
public:
	sensor_base();
	virtual ~sensor_base();

	/* id */
	void set_id(sensor_id_t id);
	sensor_id_t get_id(void);

	/* sensor info */
	virtual sensor_type_t get_type();
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);

	virtual bool get_sensor_info(sensor_info &info);
	virtual bool is_virtual(void);

	/* set/get data */
	virtual int get_data(sensor_data_t **data, int *length);

	virtual bool flush(void);
	virtual int set_attribute(int32_t attribute, int32_t value);

	/* start/stop */
	bool start(void);
	bool stop(void);
	bool is_started(void);

	/* interval / batch / wakeup */
	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);
	unsigned int get_interval(int client_id, bool is_processor);

	bool add_batch(int client_id, unsigned int latency);
	bool delete_batch(int client_id);
	unsigned int get_batch(int client_id);

	bool add_wakeup(int client_id, int wakeup);
	bool delete_wakeup(int client_id);
	int get_wakeup(int client_id);
	bool is_wakeup_supported(void);

	bool push(sensor_event_t *event);

	/* permission(privilege) */
	int get_permission(void);

protected:
	void set_permission(int permission);

private:
	sensor_id_t m_unique_id;
	int m_permission;

	plugin_info_list m_plugin_info_list;
	cmutex m_plugin_info_list_mutex;

	bool m_started;
	unsigned int m_client;
	cmutex m_client_mutex;

	virtual bool set_interval(unsigned long interval);
	virtual bool set_batch_latency(unsigned long latency);
	virtual bool set_wakeup(int wakeup);

	virtual bool on_start(void);
	virtual bool on_stop(void);

	static unsigned long long get_timestamp(void);
	static unsigned long long get_timestamp(timeval *t);
};

#endif /* _SENSOR_BASE_H_ */
