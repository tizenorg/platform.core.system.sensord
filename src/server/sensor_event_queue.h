/*
 * sensord
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

#ifndef _SENSOR_EVENT_QUEUE_H_
#define _SENSOR_EVENT_QUEUE_H_

#include <sensor_common.h>
#include <queue>
#include <mutex>
#include <condition_variable>

class sensor_event_queue {
public:
	static sensor_event_queue& get_instance();

	void push(sensor_event_t *event);
	void* pop(void);

private:
	static const unsigned int QUEUE_FULL_SIZE = 1000;

	std::queue<void *> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond_var;

	typedef std::lock_guard<std::mutex> lock;
	typedef std::unique_lock<std::mutex> ulock;

	sensor_event_queue() {}
	~sensor_event_queue() {}
	sensor_event_queue(const sensor_event_queue &) {}
	sensor_event_queue& operator=(const sensor_event_queue &);
	void push_internal(void *event);
};

#endif /* _SENSOR_EVENT_QUEUE_H_*/
