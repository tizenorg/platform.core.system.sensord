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

#ifndef _FUSION_SENSOR_H_
#define _FUSION_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <orientation_filter.h>

class fusion_sensor : public virtual_sensor {
public:
	fusion_sensor();
	virtual ~fusion_sensor();

	bool init(void);

	void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);
	virtual bool get_properties(sensor_type_t sensor_type, sensor_properties_s &properties);
	virtual void get_types(std::vector<sensor_type_t> &types);

	int get_sensor_data(const unsigned int data_id, sensor_data_t &data);

private:
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_magnetic_sensor;

	sensor_data<float> m_accel;
	sensor_data<float> m_gyro;
	sensor_data<float> m_magnetic;

	sensor_data<float> *m_accel_ptr;
	sensor_data<float> *m_gyro_ptr;
	sensor_data<float> *m_magnetic_ptr;

	cmutex m_value_mutex;

	orientation_filter<float> m_orientation_filter;
	orientation_filter<float> m_orientation_filter_poll;

	unsigned int m_enable_fusion;

	unsigned long long m_time;
	unsigned int m_interval;

	std::string m_vendor;
	std::string m_raw_data_unit;
	int m_default_sampling_time;
	float m_accel_static_bias[3];
	float m_gyro_static_bias[3];
	float m_geomagnetic_static_bias[3];
	int m_accel_rotation_direction_compensation[3];
	int m_gyro_rotation_direction_compensation[3];
	int m_geomagnetic_rotation_direction_compensation[3];
	float m_accel_scale;
	float m_gyro_scale;
	float m_geomagnetic_scale;
	int m_magnetic_alignment_factor;

	bool on_start(void);
	bool on_stop(void);
};

#endif
