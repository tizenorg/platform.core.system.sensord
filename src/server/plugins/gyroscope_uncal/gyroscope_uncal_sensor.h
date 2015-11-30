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

#ifndef _GYROSCOPE_UNCAL_SENSOR_H_
#define _GYROSCOPE_UNCAL_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <orientation_filter.h>

class gyroscope_uncal_sensor : public virtual_sensor {
public:
	gyroscope_uncal_sensor();
	virtual ~gyroscope_uncal_sensor();

	bool init(void);

	void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);
	virtual bool get_properties(sensor_type_t sensor_type, sensor_properties_s &properties);
	virtual void get_types(std::vector<sensor_type_t> &types);

	int get_sensor_data(const unsigned int event_type, sensor_data_t &data);

private:
	sensor_base *m_accel_sensor;
	sensor_base *m_magnetic_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_fusion_sensor;

	sensor_data<float> m_fusion;
	sensor_data<float> m_gyro;

	cmutex m_value_mutex;

	unsigned int m_enable_gyroscope_uncal;

	unsigned long long m_time;
	unsigned int m_interval;

	std::string m_vendor;
	std::string m_raw_data_unit;
	int m_default_sampling_time;
	float m_gyro_static_bias[3];
	int m_gyro_rotation_direction_compensation[3];
	float m_gyro_scale;

	bool on_start(void);
	bool on_stop(void);
};

#endif /*_GYROSCOPE_UNCAL_SENSOR_H_*/
