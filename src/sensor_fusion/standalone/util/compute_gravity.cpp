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


#ifdef _COMPUTE_GRAVITY_H

#include "compute_gravity.h"

#define GRAVITY		9.80665

template <typename TYPE>
compute_gravity<TYPE>::compute_gravity()
{

}

template <typename TYPE>
compute_gravity<TYPE>::~compute_gravity()
{

}

template <typename TYPE>
sensor_data<TYPE> compute_gravity<TYPE>::orientation2gravity(const sensor_data<TYPE> accel,
		const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic)
{
	orientation_filter<TYPE> orien_filter;
	euler_angles<TYPE> orientation;
	sensor_data<TYPE> gravity;

	orientation = orien_filter.get_orientation(accel, gyro, magnetic);

	gravity.m_data.m_vec[0] = GRAVITY * sin(orien_filter.m_orientation.m_ang.m_vec[0]);
	gravity.m_data.m_vec[1] = GRAVITY * sin(orien_filter.m_orientation.m_ang.m_vec[1]);
	gravity.m_data.m_vec[2] = GRAVITY * cos(orien_filter.m_orientation.m_ang.m_vec[0]) *
									cos(orien_filter.m_orientation.m_ang.m_vec[1]);

	return gravity;
}

#endif
