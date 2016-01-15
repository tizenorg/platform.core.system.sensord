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

#include <csensor_event_queue.h>
#include <sensor_types.h>
#include "sensor_logs.h"

csensor_event_queue& csensor_event_queue::get_instance()
{
	static csensor_event_queue inst;
	return inst;
}
/*
template<typename T>
void csensor_event_queue::push(const T &event)
{
	void *new_event = malloc(sizeof(event));
	if (!new_event)
		return;
	memcpy(new_event, &event, sizeof(event));
	push_internal(new_event, sizeof(event));
}

template<typename T>
void csensor_event_queue::push(T *event)
{
	push_internal(event, sizeof(event));
}
*/
/*
template void csensor_event_queue::push<sensor_event_t>(const sensor_event_t&);
template void csensor_event_queue::push<sensor_event_t>(sensor_event_t*);

template void csensor_event_queue::push<pedo_event_t>(const pedo_event_t&);
template void csensor_event_queue::push<pedo_event_t>(pedo_event_t*);

template void csensor_event_queue::push<sensorhub_event_t>(const sensorhub_event_t&);
template void csensor_event_queue::push<sensorhub_event_t>(sensorhub_event_t*);
*/
void csensor_event_queue::push_internal(void *event, int length)
{
	lock l(m_mutex);
	bool wake = m_queue.empty();

	if (m_queue.size() >= QUEUE_FULL_SIZE) {
		ERR("Queue is full, drop it!");
		free(event);
	} else
		m_queue.push(std::pair<void*, int>(event, length));

	if (wake)
		m_cond_var.notify_one();
}

void* csensor_event_queue::pop(int *length)
{
	ulock u(m_mutex);
	while (m_queue.empty())
		m_cond_var.wait(u);

	std::pair<void*, int> event = m_queue.top();
	m_queue.pop();

	*length = event.second;
	return event.first;
}
