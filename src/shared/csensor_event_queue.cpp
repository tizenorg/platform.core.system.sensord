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
#include "common.h"
#include <sensor_accel.h>

bool prioritize_events = false;

csensor_event_queue::csensor_event_queue()
{
}

void csensor_event_queue::push(sensor_event_t const &event)
{
	sensor_event_t *new_event = new sensor_event_t;
	*new_event = event;
	push_internal(new_event);
}

void csensor_event_queue::push(sensorhub_event_t const &event)
{
	sensorhub_event_t *new_event = new sensorhub_event_t;
	*new_event = event;
	push_internal(new_event);
}

void csensor_event_queue::push_internal(void *event)
{
	sensor_event_t *ev= (sensor_event_t *) event;
	lock l(m_mutex);
	bool wake = m_queue.empty();
	
	if (m_queue.size() >= QUEUE_FULL_SIZE) {
		ERR("Queue is full");
	} else {
		if(prioritize_events == false) {
			m_queue.push(event);
		}

		else {	
			if (ev->event_type != ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME)
				m_queue.push(event);
			else { 
				while(!m_queue.empty()) { 
				void *e = m_queue.front(); 
				aux_queue.push(e); 
				m_queue.pop(); 
				} 

				m_queue.push(ev); 
				while(!aux_queue.empty()) { 
				void *e = aux_queue.front(); 
				m_queue.push(e); 
				aux_queue.pop(); 
                 		} 
			}
			
		}
	}
		
	if (wake)
		m_cond_var.notify_one();

		
}

void *csensor_event_queue::pop(void)
{
	ulock u(m_mutex);

	while (m_queue.empty())
		m_cond_var.wait(u);

	void *event = m_queue.front();
	m_queue.pop();
	return event;
}

