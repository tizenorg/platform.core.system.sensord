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

#ifndef _LIB_SENSOR_FUSION_H_
#define _LIB_SENSOR_FUSION_H_

#include <sensor_fusion.h>

class lib_sensor_fusion : public sensor_fusion
{
public:
	lib_sensor_fusion();
	~lib_sensor_fusion();

	bool init(void);
	bool on_start(void);
	bool on_stop(void);

	bool add_interval(int client_id, unsigned int interval);
	bool delete_interval(int client_id);
	bool get_properties(sensor_properties_t &properties);

	void fuse(const sensor_event_t &event);
	bool get_rotation_matrix(arr33_t &rot);
	bool get_attitude(float &x, float &y, float &z, float &w);
	bool get_gyro_bias(float &x, float &y, float &z);
	bool get_rotation_vector(float &x, float &y, float &z, float &w, float &heading_accuracy);
	bool get_linear_acceleration(float &x, float &y, float &z);
	bool get_gravity(float &x, float &y, float &z);
	bool get_rotation_vector_6axis(float &x, float &y, float &z, float &w, float &heading_accuracy);
	bool get_geomagnetic_rotation_vector(float &x, float &y, float &z, float &w);
	bool get_orientation(float &azimuth, float &pitch, float &roll);
private:
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;
	sensor_base *m_magnetic_sensor;
};

#endif /*_LIB_SENSOR_FUSION_H_*/
