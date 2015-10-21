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

#include <cwakeup_info_list.h>
#include <algorithm>


cwakeup_info::cwakeup_info(int client_id, int wakeup)
{
	this->client_id = client_id;
	this->wakeup = wakeup;
}

cwakeup_info_iterator cwakeup_info_list::find_if(int client_id)
{
	auto iter = m_list.begin();

	while (iter != m_list.end()) {
		if (iter->client_id == client_id)
			break;

		++iter;
	}

	return iter;
}


bool cwakeup_info_list::add_wakeup(int client_id, int wakeup)
{
	auto iter = find_if(client_id);

	if (iter != m_list.end())
		*iter = cwakeup_info(client_id, wakeup);
	else
		m_list.push_back(cwakeup_info(client_id, wakeup));

	return true;
}

bool cwakeup_info_list::delete_wakeup(int client_id)
{
	auto iter = find_if(client_id);

	if (iter == m_list.end())
		return false;

	m_list.erase(iter);

	return true;
}

int cwakeup_info_list::get_wakeup(int client_id)
{
	auto iter = find_if(client_id);

	if (iter == m_list.end())
		return -1;

	return iter->wakeup;
}

int cwakeup_info_list::is_wakeup_on(void)
{
	if (m_list.empty())
		return -1;

	auto iter = m_list.begin();

	while (iter != m_list.end()) {
		if (iter->wakeup == true)
			break;

		++iter;
	}

	return iter->wakeup;
}

