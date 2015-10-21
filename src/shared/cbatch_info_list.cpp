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

#include <cbatch_info_list.h>
#include <algorithm>

cbatch_info::cbatch_info(int client_id, unsigned int latency)
{
	this->client_id = client_id;
	this->latency = latency;
}

bool cbatch_info_list::comp_batch_info(cbatch_info a, cbatch_info b)
{
	return a.latency < b.latency;
}

cbatch_info_iterator cbatch_info_list::find_if(int client_id)
{
	auto iter = m_list.begin();

	while (iter != m_list.end()) {
		if ((iter->client_id == client_id))
			break;

		++iter;
	}

	return iter;
}

bool cbatch_info_list::add_batch(int client_id, unsigned int latency)
{
	auto iter = find_if(client_id);

	if (iter != m_list.end())
		*iter = cbatch_info(client_id, latency);
	else
		m_list.push_back(cbatch_info(client_id, latency));

	return true;
}

bool cbatch_info_list::delete_batch(int client_id)
{
	auto iter = find_if(client_id);

	if (iter == m_list.end())
		return false;

	m_list.erase(iter);

	return true;
}

unsigned int cbatch_info_list::get_batch(int client_id)
{
	auto iter = find_if(client_id);

	if (iter == m_list.end())
		return 0;

	return iter->latency;
}

unsigned int cbatch_info_list::get_max(void)
{
	if (m_list.empty())
		return 0;

	auto iter = max_element(m_list.begin(), m_list.end(), comp_batch_info);

	return iter->latency;
}

