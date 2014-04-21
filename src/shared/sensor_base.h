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

#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cinterval_info_list.h>
#include <cmutex.h>
#include <common.h>
#include <sensor_common.h>
#include <worker_thread.h>

using std::string;
using std::mutex;
using std::recursive_mutex;
using std::lock_guard;
using std::list;
using std::map;
using std::vector;
using std::unique_lock;
using std::condition_variable;

class sensor_base
{
private:
	typedef map<unsigned int, unsigned int> client_info;

public:
	sensor_base();
	virtual ~sensor_base();

	virtual bool init(void);
	virtual sensor_type_t get_type(void);
	virtual const char *get_name(void);
	virtual bool is_virtual(void);
	virtual bool is_fusion(void);

	virtual bool on_start(void);
	virtual bool on_stop(void);

	bool start(void);
	bool stop(void);
	bool is_started(void);

	virtual bool add_client(unsigned int event_type);
	virtual bool delete_client(unsigned int event_type);

	virtual bool add_interval(int client_id, unsigned int interval, bool is_processor = false);
	virtual bool delete_interval(int client_id, bool is_processor = false);
	unsigned int get_interval(int client_id, bool is_processor = false);

	virtual bool get_properties(const unsigned int type, sensor_properties_t &properties);
	bool is_supported(unsigned int event_type);

	virtual long set_command(const unsigned int cmd, long value);
	virtual int send_sensorhub_data(const char *data, int data_len);

	virtual int get_sensor_data(const unsigned int type, sensor_data_t &data);

	void register_supported_event(unsigned int event_type);
protected:
	typedef lock_guard<mutex> lock;
	typedef lock_guard<recursive_mutex> rlock;
	typedef unique_lock<mutex> ulock;

	cinterval_info_list m_interval_info_list;
	cmutex m_interval_info_list_mutex;

	cmutex m_mutex;

	unsigned int m_client;
	cmutex m_client_mutex;

	client_info m_client_info;
	cmutex m_client_info_mutex;

	vector<int> m_supported_event_info;

	bool m_started;

	string m_name;

	unsigned int get_client_cnt(unsigned int event_type);
	virtual bool set_interval(unsigned long val);

	unsigned long long get_timestamp(void);
	unsigned long long get_timestamp(timeval *t);
};

#endif /*_SENSOR_BASE_H_*/
