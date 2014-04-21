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

#include <sensor_base.h>
#include <algorithm>

#define UNKNOWN_NAME "UNKNOWN_SENSOR"

sensor_base::sensor_base()
: m_client(0)
, m_started(false)
{
}

sensor_base::~sensor_base()
{
}

bool sensor_base::init()
{
	return true;
}

bool sensor_base::is_virtual()
{
	return false;
}

bool sensor_base::is_fusion(void)
{
	return false;
}

sensor_type_t sensor_base::get_type()
{
	return UNKNOWN_SENSOR;
}

const char *sensor_base::get_name()
{
	if (m_name.empty())
		return UNKNOWN_NAME;

	return m_name.c_str();
}

bool sensor_base::on_start()
{
	return true;
}

bool sensor_base::on_stop()
{
	return true;
}

bool sensor_base::start()
{
	AUTOLOCK(m_mutex);
	AUTOLOCK(m_client_mutex);
	++m_client;

	if (m_client == 1) {
		if (!on_start()) {
			ERR("[%s] sensor failed to start", get_name());
			return false;
		}

		m_started = true;
	}

	INFO("[%s] sensor started, #client = %d", get_name(), m_client);
	return true;
}

bool sensor_base::stop(void)
{
	AUTOLOCK(m_mutex);
	AUTOLOCK(m_client_mutex);
	--m_client;

	if (m_client == 0) {
		if (!on_stop()) {
			ERR("[%s] sensor faild to stop", get_name());
			return false;
		}

		m_started = false;
	}

	INFO("[%s] sensor stopped, #client = %d", get_name(), m_client);
	return true;
}

bool sensor_base::is_started(void)
{
	AUTOLOCK(m_mutex);
	AUTOLOCK(m_client_mutex);
	return m_started;
}

bool sensor_base::add_client(unsigned int event_type)
{
	if (!is_supported(event_type)) {
		ERR("Invaild event type: 0x%x", event_type);
		return false;
	}

	AUTOLOCK(m_client_info_mutex);
	++(m_client_info[event_type]);
	return true;
}

bool sensor_base::delete_client(unsigned int event_type)
{
	if (!is_supported(event_type)) {
		ERR("Invaild event type: 0x%x", event_type);
		return false;
	}

	AUTOLOCK(m_client_info_mutex);
	client_info::iterator iter;
	iter = m_client_info.find(event_type);

	if (iter == m_client_info.end())
		return false;

	if (iter->second == 0)
		return false;

	--(iter->second);
	return true;
}

bool sensor_base::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	unsigned int cur_min;
	AUTOLOCK(m_interval_info_list_mutex);
	cur_min = m_interval_info_list.get_min();

	if (!m_interval_info_list.add_interval(client_id, interval, is_processor))
		return false;

	if ((interval < cur_min) || !cur_min) {
		INFO("Min interval for sensor[0x%x] is changed from %dms to %dms"
			" by%sclient[%d] adding interval",
			get_type(), cur_min, interval,
			is_processor ? " processor " : " ", client_id);
		set_interval(interval);
	}

	return true;
}

bool sensor_base::delete_interval(int client_id, bool is_processor)
{
	unsigned int prev_min, cur_min;
	AUTOLOCK(m_interval_info_list_mutex);
	prev_min = m_interval_info_list.get_min();

	if (!m_interval_info_list.delete_interval(client_id, is_processor))
		return false;

	cur_min = m_interval_info_list.get_min();

	if (!cur_min) {
		INFO("No interval for sensor[0x%x] by%sclient[%d] deleting interval, "
			 "so set to default %dms",
			 get_type(), is_processor ? " processor " : " ",
			 client_id, POLL_1HZ_MS);
		set_interval(POLL_1HZ_MS);
	} else if (cur_min != prev_min) {
		INFO("Min interval for sensor[0x%x] is changed from %dms to %dms"
			" by%sclient[%d] deleting interval",
			get_type(), prev_min, cur_min,
			is_processor ? " processor " : " ", client_id);
		set_interval(cur_min);
	}

	return true;
}

unsigned int sensor_base::get_interval(int client_id, bool is_processor)
{
	AUTOLOCK(m_interval_info_list_mutex);
	return m_interval_info_list.get_interval(client_id, is_processor);
}

bool sensor_base::get_properties(const unsigned int type, sensor_properties_t &properties)
{
	ERR("Invalid type: 0x%x", type);
	return false;
}

bool sensor_base::is_supported(unsigned int event_type)
{
	vector<int>::iterator iter;
	iter = find(m_supported_event_info.begin(), m_supported_event_info.end(), event_type);

	if (iter == m_supported_event_info.end())
		return false;

	return true;
}

long sensor_base::set_command(const unsigned int cmd, long value)
{
	return -1;
}

int sensor_base::send_sensorhub_data(const char *data, int data_len)
{
	return -1;
}

int sensor_base::get_sensor_data(const unsigned int type, sensor_data_t &data)
{
	return -1;
}

void sensor_base::register_supported_event(unsigned int event_type)
{
	m_supported_event_info.push_back(event_type);
}

unsigned int sensor_base::get_client_cnt(unsigned int event_type)
{
	AUTOLOCK(m_client_info_mutex);
	client_info::iterator iter;
	iter = m_client_info.find(event_type);

	if (iter == m_client_info.end())
		return 0;

	return iter->second;
}

bool sensor_base::set_interval(unsigned long val)
{
	return true;
}

unsigned long long sensor_base::get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec) * NS_TO_SEC + t.tv_nsec) / MS_TO_SEC;
}

unsigned long long sensor_base::get_timestamp(timeval *t)
{
	if (!t) {
		ERR("t is NULL");
		return 0;
	}

	return ((unsigned long long)(t->tv_sec) * US_TO_SEC + t->tv_usec);
}
