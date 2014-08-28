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
#include <sensor_common.h>
#include <sensor_gyro.h>
#include <sensor_accel.h>
/*class compare{
public:
bool operator()(sensor_event_t &t1,sensor_event_t &t2){
//t1=(sensor_event_t *) t1;
//t2=(sensor_event_t *) t2;
//if(t2->event_type==ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME || t2->event_type==GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME)
return true;
//return false;

   }
};*/
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
	} else{        

		m_queue.push(event);

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

