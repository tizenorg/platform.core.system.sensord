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
#include <utility>

using std::make_pair;
using std::vector;

#define UNKNOWN_NAME "UNKNOWN_SENSOR"

sensor_base::sensor_base()
: m_privilege(SENSOR_PRIVILEGE_PUBLIC)
, m_permission(SENSOR_PERMISSION_STANDARD)
, m_client(0)
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

void sensor_base::add_id(sensor_id_t id)
{
	m_ids.insert(std::make_pair(static_cast<sensor_type_t> (id & SENSOR_TYPE_MASK), id));
}

sensor_id_t sensor_base::get_id(void)
{
	auto it = m_ids.begin();

	if (it != m_ids.end())
		return it->second;

	return UNKNOWN_SENSOR;
}

sensor_id_t sensor_base::get_id(sensor_type_t sensor_type)
{

	auto it = m_ids.find(sensor_type);

	if (it != m_ids.end())
		return it->second;

	return UNKNOWN_SENSOR;
}

sensor_privilege_t sensor_base::get_privilege(void)
{
	return m_privilege;
}

int sensor_base::get_permission(void)
{
	return m_permission;
}


void sensor_base::set_privilege(sensor_privilege_t privilege)
{
	m_privilege = privilege;
}

void sensor_base::set_permission(int permission)
{
	m_permission = permission;
}

const char* sensor_base::get_name()
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

	auto iter = m_client_info.find(event_type);

	if (iter == m_client_info.end())
		return false;

	if (iter->second == 0)
		return false;

	--(iter->second);

	return true;
}

bool sensor_base::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	unsigned int prev_min, cur_min;

	AUTOLOCK(m_plugin_info_list_mutex);

	prev_min = m_plugin_info_list.get_min_interval();

	if (!m_plugin_info_list.add_interval(client_id, interval, is_processor))
		return false;

	cur_min = m_plugin_info_list.get_min_interval();

	if (cur_min != prev_min) {
		INFO("Min interval for sensor[0x%x] is changed from %dms to %dms"
			" by%sclient[%d] adding interval",
			get_id(), prev_min, cur_min,
			is_processor ? " processor " : " ", client_id);
		set_interval(cur_min);
	}

	return true;
}

bool sensor_base::delete_interval(int client_id, bool is_processor)
{
	unsigned int prev_min, cur_min;
	AUTOLOCK(m_plugin_info_list_mutex);

	prev_min = m_plugin_info_list.get_min_interval();

	if (!m_plugin_info_list.delete_interval(client_id, is_processor))
		return false;

	cur_min = m_plugin_info_list.get_min_interval();

	if (!cur_min) {
		INFO("No interval for sensor[0x%x] by%sclient[%d] deleting interval, "
			 "so set to default %dms",
			 get_id(), is_processor ? " processor " : " ",
			 client_id, POLL_1HZ_MS);

		set_interval(POLL_1HZ_MS);
	} else if (cur_min != prev_min) {
		INFO("Min interval for sensor[0x%x] is changed from %dms to %dms"
			" by%sclient[%d] deleting interval",
			get_id(), prev_min, cur_min,
			is_processor ? " processor " : " ", client_id);

		set_interval(cur_min);
	}

	return true;
}

unsigned int sensor_base::get_interval(int client_id, bool is_processor)
{
	AUTOLOCK(m_plugin_info_list_mutex);

	return m_plugin_info_list.get_interval(client_id, is_processor);
}

bool sensor_base::add_wakeup(int client_id, int wakeup)
{
	int prev_wakeup, cur_wakeup;

	AUTOLOCK(m_plugin_info_list_mutex);

	prev_wakeup = m_plugin_info_list.is_wakeup_on();

	if (!m_plugin_info_list.add_wakeup(client_id, wakeup))
		return false;

	cur_wakeup = m_plugin_info_list.is_wakeup_on();

	if ((cur_wakeup == SENSOR_WAKEUP_ON) && (prev_wakeup < SENSOR_WAKEUP_ON)) {
		INFO("Wakeup for sensor[0x%x] is changed from %d to %d by client[%d] adding wakeup",
			get_id(), prev_wakeup, cur_wakeup, client_id);
		set_wakeup(client_id, SENSOR_WAKEUP_ON);
	}

	return true;
}

bool sensor_base::delete_wakeup(int client_id)
{
	int prev_wakeup, cur_wakeup;
	AUTOLOCK(m_plugin_info_list_mutex);

	prev_wakeup = m_plugin_info_list.is_wakeup_on();

	if (!m_plugin_info_list.delete_wakeup(client_id))
		return false;

	cur_wakeup = m_plugin_info_list.is_wakeup_on();

	if ((cur_wakeup < SENSOR_WAKEUP_ON) && (prev_wakeup == SENSOR_WAKEUP_ON)) {
		INFO("Wakeup for sensor[0x%x] is changed from %d to %d by client[%d] deleting wakeup",
			get_id(), prev_wakeup, cur_wakeup, client_id);
		set_wakeup(client_id, SENSOR_WAKEUP_OFF);
	}

	return true;
}

int sensor_base::get_wakeup(int client_id)
{
	AUTOLOCK(m_plugin_info_list_mutex);

	return m_plugin_info_list.is_wakeup_on();
}

bool sensor_base::add_batch(int client_id, unsigned int latency)
{
	unsigned int prev_max, cur_max;

	AUTOLOCK(m_plugin_info_list_mutex);

	prev_max = m_plugin_info_list.get_max_batch();

	if (!m_plugin_info_list.add_batch(client_id, latency))
		return false;

	cur_max = m_plugin_info_list.get_max_batch();

	if (cur_max != prev_max) {
		INFO("Max latency for sensor[0x%x] is changed from %dms to %dms by client[%d] adding latency",
			get_id(), prev_max, cur_max, client_id);
		set_batch(client_id, cur_max);
	}

	return true;
}

bool sensor_base::delete_batch(int client_id)
{
	unsigned int prev_max, cur_max;
	AUTOLOCK(m_plugin_info_list_mutex);

	prev_max = m_plugin_info_list.get_max_batch();

	if (!m_plugin_info_list.delete_batch(client_id))
		return false;

	cur_max = m_plugin_info_list.get_max_batch();

	if (!cur_max) {
		INFO("No latency for sensor[0x%x] by client[%d] deleting latency, so set to default 0 ms",
			 get_id(), client_id);

		set_batch(client_id, 0);
	} else if (cur_max != prev_max) {
		INFO("Max latency for sensor[0x%x] is changed from %dms to %dms by client[%d] deleting latency",
			get_id(), prev_max, cur_max, client_id);

		set_batch(client_id, cur_max);
	}

	return true;
}

unsigned int sensor_base::get_batch(int client_id)
{
	AUTOLOCK(m_plugin_info_list_mutex);

	return m_plugin_info_list.get_batch(client_id);
}

void sensor_base::get_sensor_info(sensor_type_t sensor_type, sensor_info &info)
{
	sensor_properties_s properties;
	properties.wakeup_supported = false;

	get_properties(sensor_type, properties);

	info.set_type(sensor_type);
	info.set_id(get_id(sensor_type));
	info.set_privilege(m_privilege);
	info.set_name(properties.name.c_str());
	info.set_vendor(properties.vendor.c_str());
	info.set_min_range(properties.min_range);
	info.set_max_range(properties.max_range);
	info.set_resolution(properties.resolution);
	info.set_min_interval(properties.min_interval);
	info.set_fifo_count(properties.fifo_count);
	info.set_max_batch_count(properties.max_batch_count);

	vector<unsigned int> events;

	for (unsigned int i = 0; i < m_supported_event_info.size(); ++ i) {
		if (m_supported_event_info[i] & (sensor_type << 16))
			events.push_back(m_supported_event_info[i]);
	}

	info.set_supported_events(events);
	info.set_wakeup_supported(properties.wakeup_supported);

	return;
}

bool sensor_base::get_properties(sensor_type_t sensor_type, sensor_properties_s &properties)
{
	return false;
}

bool sensor_base::is_supported(unsigned int event_type)
{
	auto iter = find(m_supported_event_info.begin(), m_supported_event_info.end(), event_type);

	if (iter == m_supported_event_info.end())
		return false;

	return true;
}

bool sensor_base::is_wakeup_supported(void)
{
	return false;
}

long sensor_base::set_command(unsigned int cmd, long value)
{
	return -1;
}

bool sensor_base::set_wakeup(int client_id, int wakeup)
{
	return false;
}

bool sensor_base::set_batch(int client_id, unsigned int latency)
{
	return false;
}

int sensor_base::send_sensorhub_data(const char* data, int data_len)
{
	return -1;
}

int sensor_base::get_sensor_data(unsigned int type, sensor_data_t &data)
{
	return -1;
}

void sensor_base::register_supported_event(unsigned int event_type)
{
	m_supported_event_info.push_back(event_type);
}

void sensor_base::unregister_supported_event(unsigned int event_type)
{
	m_supported_event_info.erase(std::remove(m_supported_event_info.begin(),
			m_supported_event_info.end(), event_type), m_supported_event_info.end());
}
unsigned int sensor_base::get_client_cnt(unsigned int event_type)
{
	AUTOLOCK(m_client_info_mutex);

	auto iter = m_client_info.find(event_type);

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
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

unsigned long long sensor_base::get_timestamp(timeval *t)
{
	if (!t) {
		ERR("t is NULL");
		return 0;
	}

	return ((unsigned long long)(t->tv_sec)*1000000LL +t->tv_usec);
}
