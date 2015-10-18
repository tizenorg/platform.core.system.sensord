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

#ifndef _LINEAR_ACCEL_SENSOR_H_
#define _LINEAR_ACCEL_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <orientation_filter.h>

class linear_accel_sensor : public virtual_sensor {
public:
	linear_accel_sensor();
	virtual ~linear_accel_sensor();

	bool init();
	virtual void get_types(std::vector<sensor_type_t> &types);

	void synthesize(const sensor_event_t& event, std::vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);

	int get_sensor_data(const unsigned int event_type, sensor_data_t &data);
	virtual bool get_properties(sensor_type_t sensor_type, sensor_properties_s &properties);
private:
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_magnetic_sensor;
	sensor_base *m_fusion_sensor;

	sensor_data<float> m_accel;
	sensor_data<float> m_gyro;
	sensor_data<float> m_magnetic;

	cmutex m_value_mutex;

	unsigned long long m_time;
	unsigned int m_interval;

	unsigned int m_enable_linear_accel;

	std::string m_vendor;
	std::string m_raw_data_unit;
	std::string m_orientation_data_unit;
	int m_default_sampling_time;
	float m_accel_static_bias[3];
	int m_accel_rotation_direction_compensation[3];
	float m_accel_scale;
	int m_linear_accel_sign_compensation[3];
	int m_gravity_sign_compensation[3];
	int m_azimuth_rotation_compensation;
	int m_pitch_rotation_compensation;
	int m_roll_rotation_compensation;

	bool on_start(void);
	bool on_stop(void);
	sensor_data_t calculate_gravity(sensor_data_t data);
};

#endif
