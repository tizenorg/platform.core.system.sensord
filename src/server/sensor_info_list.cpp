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

#include <sensor_info_list.h>
#include <algorithm>

interval_info::interval_info(int client_id, bool is_processor, unsigned int interval)
{
	this->client_id = client_id;
	this->is_processor = is_processor;
	this->interval = interval;
}

batch_info::batch_info(int client_id, unsigned int latency)
{
	this->client_id = client_id;
	this->latency = latency;
}

bool sensor_info_list::comp_interval_info(interval_info a, interval_info b)
{
	return a.interval < b.interval;
}

bool sensor_info_list::comp_batch_info(batch_info a, batch_info b)
{
	return a.latency < b.latency;
}

interval_info_iterator sensor_info_list::find_if_interval_info(int client_id, bool is_processor)
{
	auto iter = m_interval_info_list.begin();

	while (iter != m_interval_info_list.end()) {
		if ((iter->client_id == client_id) && (iter->is_processor == is_processor))
			break;

		++iter;
	}

	return iter;
}

batch_info_iterator sensor_info_list::find_if_batch_info(int client_id)
{
	auto iter = m_batch_info_list.begin();

	while (iter != m_batch_info_list.end()) {
		if ((iter->client_id == client_id))
			break;

		++iter;
	}

	return iter;
}

bool sensor_info_list::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter != m_interval_info_list.end())
		*iter = interval_info(client_id, is_processor, interval);
	else
		m_interval_info_list.push_back(interval_info(client_id, is_processor, interval));

	return true;
}

bool sensor_info_list::delete_interval(int client_id, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter == m_interval_info_list.end())
		return false;

	m_interval_info_list.erase(iter);

	return true;
}

unsigned int sensor_info_list::get_interval(int client_id, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter == m_interval_info_list.end())
		return 0;

	return iter->interval;
}

unsigned int sensor_info_list::get_min_interval(void)
{
	if (m_interval_info_list.empty())
		return 0;

	auto iter = min_element(m_interval_info_list.begin(), m_interval_info_list.end(), comp_interval_info);

	return iter->interval;
}

bool sensor_info_list::add_batch(int client_id, unsigned int latency)
{
	auto iter = find_if_batch_info(client_id);

	if (iter != m_batch_info_list.end())
		*iter = batch_info(client_id, latency);
	else
		m_batch_info_list.push_back(batch_info(client_id, latency));

	return true;
}

bool sensor_info_list::delete_batch(int client_id)
{
	auto iter = find_if_batch_info(client_id);

	if (iter == m_batch_info_list.end())
		return false;

	m_batch_info_list.erase(iter);

	return true;
}

unsigned int sensor_info_list::get_batch(int client_id)
{
	auto iter = find_if_batch_info(client_id);

	if (iter == m_batch_info_list.end())
		return 0;

	return iter->latency;
}

unsigned int sensor_info_list::get_max_batch(void)
{
	if (m_batch_info_list.empty())
		return 0;

	auto iter = max_element(m_batch_info_list.begin(), m_batch_info_list.end(), comp_batch_info);

	return iter->latency;
}

