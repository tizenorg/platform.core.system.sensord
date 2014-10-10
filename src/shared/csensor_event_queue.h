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

#ifndef _CSENSOR_EVENT_QUEUE_H_
#define _CSENSOR_EVENT_QUEUE_H_

#include <sf_common.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sensor_accel.h>
#include <sensor_gyro.h>
#include <sensor_geomag.h>
#include <sensor_orientation.h>
#include <sensor_linear_accel.h>
#include <sensor_gravity.h>
#include <set>
#include <algorithm>

using namespace std;
using std::queue;
using std::set;
using std::mutex;
using std::lock_guard;
using std::unique_lock;
using std::condition_variable;

static set<unsigned int> priority_list;

class csensor_event_queue
{
private:	
	static const unsigned int QUEUE_FULL_SIZE = 1000;

	struct compare {

	bool operator() (void* &v1,void *&v2) {

	sensor_event_t *e1 = (sensor_event_t *)v1;
	sensor_event_t *e2 = (sensor_event_t *)v2;
	bool prioritize_e1 = true; 
	bool prioritize_e2 = true;

	if (e2->event_type == GRAVITY_EVENT_RAW_DATA_REPORT_ON_TIME || e2->event_type == LINEAR_ACCEL_EVENT_RAW_DATA_REPORT_ON_TIME || e2->event_type == ORIENTATION_EVENT_RAW_DATA_REPORT_ON_TIME) {
		priority_list.insert(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
		priority_list.insert(GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME);
		priority_list.insert(GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME);		
	}
	
	if (priority_list.empty())
		return (e2->data.timestamp < e1->data.timestamp);

	set<unsigned int>::iterator iter_e1 = priority_list.find(e1->event_type);
	set<unsigned int>::iterator iter_e2 = priority_list.find(e2->event_type);
	
	if (iter_e1 == priority_list.end())
		prioritize_e1 = false;

	if (iter_e2 == priority_list.end())
		prioritize_e2 = false;
	
	if (prioritize_e2) {
		if (!prioritize_e1)
			return true;
		else  {
			if (e2->data.timestamp <= e1->data.timestamp)
				return true;
			else
				return false;	
			}
	
	}
	else {
		if (prioritize_e1)
			return false;
		else if (e2->data.timestamp <= e1->data.timestamp)
			return true;
		else
			return false;
	}
	
	}

	};

	std::priority_queue<void*, std::vector<void*>, compare> m_queue;			
	
	mutex m_mutex;
	condition_variable m_cond_var;

	typedef lock_guard<mutex> lock;
	typedef unique_lock<mutex> ulock;

	csensor_event_queue();
	csensor_event_queue(csensor_event_queue const &) {};
	csensor_event_queue &operator=(csensor_event_queue const &);
	void push_internal(void *event);

public:
	static csensor_event_queue &get_instance() {
		static csensor_event_queue inst;
		return inst;
	}
	void push(sensor_event_t const &event);
	void push(sensorhub_event_t const &event);
	void *pop(void);
	
};

#endif /*_CSENSOR_EVENT_QUEUE_H_*/
