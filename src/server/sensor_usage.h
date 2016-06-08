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

#ifndef _SENSOR_USAGE_H_
#define _SENSOR_USAGE_H_

#include <algorithm>
#include <vector>

typedef std::vector<unsigned int> reg_event_vector;

class sensor_usage {
public:
	unsigned int m_interval;
	unsigned int m_latency;
	int m_pause_policy;
	reg_event_vector m_reg_events;
	bool m_start;

	sensor_usage();
	~sensor_usage();

	bool register_event(unsigned int event_type);
	bool unregister_event(unsigned int event_type);
	bool is_event_registered(unsigned int event_type);
};

#endif /* _SENSOR_USAGE_H_ */
