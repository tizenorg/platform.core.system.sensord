/*
 * libsensord-share
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <plugin_info_list.h>
#include <algorithm>


cinterval_info::cinterval_info(int client_id, bool is_processor, unsigned int interval)
{
	this->client_id = client_id;
	this->is_processor = is_processor;
	this->interval = interval;
}

cbatch_info::cbatch_info(int client_id, unsigned int latency)
{
	this->client_id = client_id;
	this->latency = latency;
}

cwakeup_info::cwakeup_info(int client_id, int wakeup)
{
	this->client_id = client_id;
	this->wakeup = wakeup;
}

bool plugin_info_list::comp_interval_info(cinterval_info a, cinterval_info b)
{
	return a.interval < b.interval;
}

bool plugin_info_list::comp_batch_info(cbatch_info a, cbatch_info b)
{
	return a.latency < b.latency;
}

cinterval_info_iterator plugin_info_list::find_if_interval_info(int client_id, bool is_processor)
{
	auto iter = m_interval_info_list.begin();

	while (iter != m_interval_info_list.end()) {
		if ((iter->client_id == client_id) && (iter->is_processor == is_processor))
			break;

		++iter;
	}

	return iter;
}

cbatch_info_iterator plugin_info_list::find_if_batch_info(int client_id)
{
	auto iter = m_batch_info_list.begin();

	while (iter != m_batch_info_list.end()) {
		if ((iter->client_id == client_id))
			break;

		++iter;
	}

	return iter;
}

cwakeup_info_iterator plugin_info_list::find_if_wakeup_info(int client_id)
{
	auto iter = m_wakeup_info_list.begin();

	while (iter != m_wakeup_info_list.end()) {
		if (iter->client_id == client_id)
			break;

		++iter;
	}

	return iter;
}

bool plugin_info_list::add_interval(int client_id, unsigned int interval, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter != m_interval_info_list.end())
		*iter = cinterval_info(client_id, is_processor, interval);
	else
		m_interval_info_list.push_back(cinterval_info(client_id, is_processor, interval));

	return true;
}

bool plugin_info_list::delete_interval(int client_id, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter == m_interval_info_list.end())
		return false;

	m_interval_info_list.erase(iter);

	return true;
}

unsigned int plugin_info_list::get_interval(int client_id, bool is_processor)
{
	auto iter = find_if_interval_info(client_id, is_processor);

	if (iter == m_interval_info_list.end())
		return 0;

	return iter->interval;
}

unsigned int plugin_info_list::get_min_interval(void)
{
	if (m_interval_info_list.empty())
		return 0;

	auto iter = min_element(m_interval_info_list.begin(), m_interval_info_list.end(), comp_interval_info);

	return iter->interval;
}

bool plugin_info_list::add_batch(int client_id, unsigned int latency)
{
	auto iter = find_if_batch_info(client_id);

	if (iter != m_batch_info_list.end())
		*iter = cbatch_info(client_id, latency);
	else
		m_batch_info_list.push_back(cbatch_info(client_id, latency));

	return true;
}

bool plugin_info_list::delete_batch(int client_id)
{
	auto iter = find_if_batch_info(client_id);

	if (iter == m_batch_info_list.end())
		return false;

	m_batch_info_list.erase(iter);

	return true;
}

unsigned int plugin_info_list::get_batch(int client_id)
{
	auto iter = find_if_batch_info(client_id);

	if (iter == m_batch_info_list.end())
		return 0;

	return iter->latency;
}

unsigned int plugin_info_list::get_max_batch(void)
{
	if (m_batch_info_list.empty())
		return 0;

	auto iter = max_element(m_batch_info_list.begin(), m_batch_info_list.end(), comp_batch_info);

	return iter->latency;
}

bool plugin_info_list::add_wakeup(int client_id, int wakeup)
{
	auto iter = find_if_wakeup_info(client_id);

	if (iter != m_wakeup_info_list.end())
		*iter = cwakeup_info(client_id, wakeup);
	else
		m_wakeup_info_list.push_back(cwakeup_info(client_id, wakeup));

	return true;
}

bool plugin_info_list::delete_wakeup(int client_id)
{
	auto iter = find_if_wakeup_info(client_id);

	if (iter == m_wakeup_info_list.end())
		return false;

	m_wakeup_info_list.erase(iter);

	return true;
}

int plugin_info_list::get_wakeup(int client_id)
{
	auto iter = find_if_wakeup_info(client_id);

	if (iter == m_wakeup_info_list.end())
		return -1;

	return iter->wakeup;
}

int plugin_info_list::is_wakeup_on(void)
{
	if (m_wakeup_info_list.empty())
		return -1;

	auto iter = m_wakeup_info_list.begin();

	while (iter != m_wakeup_info_list.end()) {
		if (iter->wakeup == true)
			break;

		++iter;
	}

	return iter->wakeup;
}

