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

#ifndef _SENSOR_EVENT_QUEUE_H_
#define _SENSOR_EVENT_QUEUE_H_

#include <sf_common.h>
#include <cstring>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <set>

extern std::set<unsigned int> priority_list;

class sensor_event_queue
{
private:
	static const unsigned int QUEUE_FULL_SIZE = 1000;

	class compare {
	public:
		bool operator() (std::pair<void *, int> v1, std::pair<void *, int> v2) {
			sensor_event_t *e1 = (sensor_event_t *)v1.first;
			sensor_event_t *e2 = (sensor_event_t *)v2.first;
			bool prioritize_e1 = true;
			bool prioritize_e2 = true;

			if (priority_list.empty())
				return (e2->data->timestamp < e1->data->timestamp);

			std::set<unsigned int>::iterator iter_e1 = priority_list.find(e1->event_type);
			std::set<unsigned int>::iterator iter_e2 = priority_list.find(e2->event_type);

			if (iter_e1 == priority_list.end())
				prioritize_e1 = false;

			if (iter_e2 == priority_list.end())
				prioritize_e2 = false;

			if (prioritize_e2) {
				if (!prioritize_e1)
					return true;
				else {
					if (e2->data->timestamp <= e1->data->timestamp)
						return true;
					return false;
				}
			}
			else {
				if (prioritize_e1)
					return false;
				else if (e2->data->timestamp <= e1->data->timestamp)
					return true;
				else
					return false;
			}
		}
	};

	std::priority_queue<std::pair<void*, int>, std::vector<std::pair<void*, int>>, compare> m_queue;

	std::mutex m_mutex;
	std::condition_variable m_cond_var;

	typedef std::lock_guard<std::mutex> lock;
	typedef std::unique_lock<std::mutex> ulock;

	sensor_event_queue() {};
	~sensor_event_queue() {};
	sensor_event_queue(const sensor_event_queue &) {};
	sensor_event_queue& operator=(const sensor_event_queue &);
	void push_internal(void *event, int length);
public:
	static sensor_event_queue& get_instance();

	void push(sensor_event_t *event, int event_length);
	void* pop(int *length);
};

#endif /* _SENSOR_EVENT_QUEUE_H_*/
