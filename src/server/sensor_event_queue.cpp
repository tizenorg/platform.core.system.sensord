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

#include <sensor_event_queue.h>
#include "sensor_log.h"

sensor_event_queue& sensor_event_queue::get_instance()
{
	static sensor_event_queue inst;
	return inst;
}

void sensor_event_queue::push_internal(void *event)
{
	lock l(m_mutex);
	bool wake = m_queue.empty();

	if (m_queue.size() >= QUEUE_FULL_SIZE) {
		_E("Queue is full, drop it!");
		free(event);
	} else
		m_queue.push(event);

	if (wake)
		m_cond_var.notify_one();
}

void* sensor_event_queue::pop(void)
{
	ulock u(m_mutex);
	while (m_queue.empty())
		m_cond_var.wait(u);

	void *event = m_queue.top();
	m_queue.pop();

	return event;
}

void sensor_event_queue::push(sensor_event_t *event)
{
	push_internal(event);
}
