/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#ifndef _FUSION_SENSOR_H
#define _FUSION_SENSOR_H

#include <fusion.h>
#include <orientation_filter.h>

class fusion_sensor : public virtual fusion {
public:
	fusion_sensor();
	fusion_sensor(fusion_type FUSION_TYPE);
	virtual ~fusion_sensor();

	virtual bool push_accel(sensor_data_t &data);
	virtual bool push_gyro(sensor_data_t &data);
	virtual bool push_mag(sensor_data_t &data);
	virtual bool get_rv(unsigned long long timestamp, float &w, float &x, float &y, float &z);

private:

	sensor_data<float> m_accel;
	sensor_data<float> m_gyro;
	sensor_data<float> m_magnetic;

	orientation_filter<float> m_orientation_filter;

	bool m_enable_accel;
	bool m_enable_gyro;
	bool m_enable_magnetic;

	fusion_type M_FUSION_TYPE;
	float m_x;
	float m_y;
	float m_z;
	float m_w;
	float m_timestamp;

	float m_accel_static_bias[3];
	float m_gyro_static_bias[3];
	float m_geomagnetic_static_bias[3];
	int m_accel_rotation_direction_compensation[3];
	int m_gyro_rotation_direction_compensation[3];
	int m_geomagnetic_rotation_direction_compensation[3];
	int m_magnetic_alignment_factor;

	void init(fusion_type FUSION_TYPE);
	void clear();
	void get_orientation();
};



#endif /* _FUSION_SENSOR_H */
