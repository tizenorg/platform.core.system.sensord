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

#ifndef _COMPUTE_GRAVITY_H
#define _COMPUTE_GRAVITY_H

#include "orientation_filter.h"

template <typename TYPE>
class compute_gravity {
public:
	orientation_filter<TYPE> estimate_orientation;

	compute_gravity();
	~compute_gravity();

	sensor_data<TYPE> orientation2gravity(const sensor_data<TYPE> accel,
			const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic);
};

#include "compute_gravity.cpp"

#endif /* _COMPUTE_GRAVITY_H */
