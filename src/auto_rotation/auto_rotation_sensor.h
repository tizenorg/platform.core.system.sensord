/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#ifndef _AUTO_ROTATION_SENSOR_H_
#define _AUTO_ROTATION_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <orientation_filter.h>
#include <auto_rotation_alg.h>

class auto_rotation_sensor : public virtual_sensor {
public:
	auto_rotation_sensor();
	virtual ~auto_rotation_sensor();

	bool init();
	sensor_type_t get_type(void);

	void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);

	int get_sensor_data(const unsigned int event_type, sensor_data_t &data);
	bool get_properties(sensor_properties_s &properties);
private:
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_magnetic_sensor;
	sensor_base *m_fusion_sensor;

	sensor_data<float> m_accel;
	sensor_data<float> m_gyro;
	sensor_data<float> m_magnetic;

	cmutex m_value_mutex;

	int m_accuracy;
	int m_rotation;
	unsigned long long m_time;
	unsigned int m_interval;
	int m_prev_rotation_x;
	int m_prev_rotation_y;
	int m_prev_rotation_z;


	auto_rotation_alg *m_alg;

	string m_vendor;
	string m_raw_data_unit;
	int m_default_sampling_time;
	int m_azimuth_rotation_compensation;
	int m_pitch_rotation_compensation;
	int m_roll_rotation_compensation;

	auto_rotation_alg *get_alg();
	bool on_start(void);
	bool on_stop(void);
};

#endif /* _AUTO_ROTATION_SENSOR_H_ */
