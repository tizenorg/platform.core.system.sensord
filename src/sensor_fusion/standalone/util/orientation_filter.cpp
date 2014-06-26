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

#include "orientation_filter.h"

template <typename TYPE>
orientation_filter<TYPE>::orientation_filter()
{

}

template <typename TYPE>
orientation_filter<TYPE>::~orientation_filter()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::filter_sensor_data()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::orientation_triad_algorithm()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_aiding_var()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_driving_var()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_process_covar()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_measurement_covar()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_prediction_covar()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_quat_diff()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_quat_sum()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::update_quat_sum()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::time_update()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::measurement_update()
{

}

template <typename TYPE>
euler_angles<TYPE> orientation_filter<TYPE>::get_orientation(sensor_data<TYPE> accel,
		sensor_data<TYPE> gyro, sensor_data<TYPE> magnetic)
{
	euler_angles<TYPE> e;

	return e;
}
