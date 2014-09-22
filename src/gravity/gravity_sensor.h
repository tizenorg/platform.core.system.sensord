/*
 * sensord
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

#ifndef _GRAVITY_SENSOR_H_
#define _GRAVITY_SENSOR_H_

#include <sensor.h>
#include <virtual_sensor.h>
#include <string>

using std::string;

class gravity_sensor : public virtual_sensor
{
public:
	gravity_sensor();
	virtual ~gravity_sensor();

	bool init();
	sensor_type_t get_type(void);

	static bool working(void *inst);

	bool on_start(void);
	bool on_stop(void);

	void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);

	int get_sensor_data(const unsigned int event_type, sensor_data_t &data);
	bool get_properties(const unsigned int type, sensor_properties_t &properties);
private:
	sensor_base *m_orientation_sensor;

	float m_x;
	float m_y;
	float m_z;
	unsigned long long m_timestamp;
};

#endif /*_GRAVITY_SENSOR_H_*/
