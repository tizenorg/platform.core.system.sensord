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

#ifndef _ORIENTATION_SENSOR_H_
#define _ORIENTATION_SENSOR_H_

#include <sensor_fusion.h>
#include <orientation_filter.h>

class orientation_sensor : public sensor_fusion
{
public:
	orientation_sensor();
	~orientation_sensor();

	bool init(void);
	bool on_start(void);
	bool on_stop(void);

	void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);
	bool get_properties(sensor_properties_t &properties);

	int get_sensor_data(const unsigned int data_id, sensor_data_t &data);

private:
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_magnetic_sensor;

	orientation_filter<float> orientation;

	float m_roll;
	float m_pitch;
	float m_yaw;
	unsigned long long m_timestamp;
};

#endif /* _ORIENTATION_SENSOR_H_ */
