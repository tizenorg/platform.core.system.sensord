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

#ifndef _SENSOR_FUSION_H_
#define _SENSOR_FUSION_H_

#include <array>
#include <sensor_base.h>

typedef std::array<std::array<float, 3> , 3> arr33_t;

class sensor_fusion : public sensor_base
{
public:
	sensor_fusion();
	virtual ~sensor_fusion();

	virtual void fuse(const sensor_event_t &event) = 0;
	virtual bool is_data_ready(void);
	virtual bool add_interval(int client_id, unsigned int interval);
	virtual bool delete_interval(int client_id);
	virtual bool get_properties(sensor_properties_t &properties);

	virtual bool get_rotation_matrix(arr33_t &rot);
	virtual bool get_attitude(float &x, float &y, float &z, float &w);
	virtual bool get_gyro_bias(float &x, float &y, float &z);
	virtual bool get_rotation_vector(float &x, float &y, float &z, float &w, float &heading_accuracy);
	virtual bool get_linear_acceleration(float &x, float &y, float &z);
	virtual bool get_gravity(float &x, float &y, float &z);
	virtual bool get_rotation_vector_6axis(float &x, float &y, float &z, float &w, float &heading_accuracy);
	virtual bool get_geomagnetic_rotation_vector(float &x, float &y, float &z, float &w);
	virtual bool get_orientation(float &azimuth, float &pitch, float &roll);

	bool is_fusion(void);
};

#endif /*_SENSOR_FUSION_H_*/
