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

#ifndef _SENSOR_DATA_H_
#define _SENSOR_DATA_H_

#include "vector.h"
#include "quaternion.h"

#define SENSOR_DATA_SIZE 3

template <typename TYPE>
class sensor_data {
public:
	vect<TYPE, SENSOR_DATA_SIZE> m_data;
	unsigned long long m_time_stamp;

	sensor_data();
	sensor_data(const TYPE x, const TYPE y, const TYPE z,
			const unsigned long long time_stamp);
	sensor_data(const vect<TYPE, SENSOR_DATA_SIZE> v,
			const unsigned long long time_stamp);
	sensor_data(const sensor_data<TYPE>& s);
	~sensor_data();

	sensor_data<TYPE> operator =(const sensor_data<TYPE>& s);

	template<typename T> friend sensor_data<T> operator +(sensor_data<T> data1,
			sensor_data<T> data2);

	template<typename T> friend void normalize(sensor_data<T>& data);
	template<typename T> friend sensor_data<T> scale_data(sensor_data<T> data,
			T scaling_factor);
	template<typename T> friend quaternion<T> sensor_data2quat(const sensor_data<T> data,
			const vect<T, REF_VEC_SIZE> ref_vec);
};

#include "sensor_data.cpp"

#endif /* _SENSOR_DATA_H_ */
