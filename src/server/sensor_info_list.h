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

#ifndef _SENSOR_INFO_LIST_H_
#define _SENSOR_INFO_LIST_H_

#include <list>

class interval_info {
public:
	interval_info(int client_id, bool is_processor, unsigned int interval);
	int client_id;
	bool is_processor;
	unsigned int interval;
};

typedef std::list<interval_info>::iterator interval_info_iterator;

class batch_info {
public:
	batch_info(int client_id, unsigned int latency);
	int client_id;
	unsigned int latency;
};

typedef std::list<batch_info>::iterator batch_info_iterator;

class sensor_info_list {
public:
	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);
	unsigned int get_interval(int client_id, bool is_processor);
	unsigned int get_min_interval(void);

	bool add_batch(int client_id, unsigned int latency);
	bool delete_batch(int client_id);
	unsigned int get_batch(int client_id);
	unsigned int get_max_batch(void);

private:
	static bool comp_interval_info(interval_info a, interval_info b);
	interval_info_iterator find_if_interval_info(int client_id, bool is_processor);

	static bool comp_batch_info(batch_info a, batch_info b);
	batch_info_iterator find_if_batch_info(int client_id);

	std::list<interval_info> m_interval_info_list;
	std::list<batch_info> m_batch_info_list;
};
#endif /* _SENSOR_INFO_LIST_H_ */
