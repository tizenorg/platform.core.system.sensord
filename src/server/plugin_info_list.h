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

#ifndef _CPLUGIN_INFO_LIST_CLASS_H_
#define _CPLUGIN_INFO_LIST_CLASS_H_

#include <list>

class cinterval_info
{
public:
	cinterval_info(int client_id, bool is_processor, unsigned int interval);
	int client_id;
	bool is_processor;
	unsigned int interval;
};

typedef std::list<cinterval_info>::iterator cinterval_info_iterator;

class cbatch_info
{
public:
	cbatch_info(int client_id, unsigned int latency);
	int client_id;
	unsigned int latency;
};

typedef std::list<cbatch_info>::iterator cbatch_info_iterator;

class cwakeup_info
{
public:
	cwakeup_info(int client_id, int wakeup);
	int client_id;
	int wakeup;
};

typedef std::list<cwakeup_info>::iterator cwakeup_info_iterator;

class plugin_info_list
{
private:
	static bool comp_interval_info(cinterval_info a, cinterval_info b);
	cinterval_info_iterator find_if_interval_info(int client_id, bool is_processor);

	static bool comp_batch_info(cbatch_info a, cbatch_info b);
	cbatch_info_iterator find_if_batch_info(int client_id);

	cwakeup_info_iterator find_if_wakeup_info(int client_id);

	std::list<cinterval_info> m_interval_info_list;
	std::list<cbatch_info> m_batch_info_list;
	std::list<cwakeup_info> m_wakeup_info_list;

public:
	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);
	unsigned int get_interval(int client_id, bool is_processor);
	unsigned int get_min_interval(void);

	bool add_batch(int client_id, unsigned int latency);
	bool delete_batch(int client_id);
	unsigned int get_batch(int client_id);
	unsigned int get_max_batch(void);

	bool add_wakeup(int client_id, int wakeup);
	bool delete_wakeup(int client_id);
	int get_wakeup(int client_id);
	int is_wakeup_on(void);
};
#endif
