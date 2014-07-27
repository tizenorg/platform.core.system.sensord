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

#define MOVING_AVERAGE_WINDOW_LENGTH	20
#define RAD2DEG		57.2957795
#define DEG2RAD		0.0174532925
#define US2S		(1.0 / 1000000.0)
#define GRAVITY		9.80665
#define PI			3.141593

// Gyro Types
// Systron-donner "Horizon"
#define ZIGMA_W		(0.05 * DEG2RAD) //deg/s
#define TAU_W		1000 //secs
// Crossbow DMU-6X
//#define ZIGMA_W			0.05 * DEG2RAD //deg/s
//#define TAU_W			300 //secs
// FOGs (KVH Autogyro and Crossbow DMU-FOG)
//#define ZIGMA_W			0 	//deg/s

template <typename TYPE>
orientation_filter<TYPE>::orientation_filter()
{

}

template <typename TYPE>
orientation_filter<TYPE>::~orientation_filter()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::filter_sensor_data(sensor_data<TYPE> accel,
		sensor_data<TYPE> gyro, sensor_data<TYPE> magnetic)
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
inline euler_angles<TYPE> orientation_filter<TYPE>::get_corrected_orientation()
{
	euler_angles<TYPE> euler_ang;

	return euler_ang;
}

template <typename TYPE>
euler_angles<TYPE> orientation_filter<TYPE>::get_orientation(sensor_data<TYPE> accel,
		sensor_data<TYPE> gyro, sensor_data<TYPE> magnetic)
{
	euler_angles<TYPE> cor_euler_ang;

	filter_sensor_data(accel, gyro, magnetic);

	orientation_triad_algorithm();

	compute_aiding_var();

	compute_driving_var();

	compute_measurement_covar();

	compute_prediction_covar();

	compute_quat_diff();

	compute_quat_sum();

	update_quat_sum();

	time_update();

	measurement_update();

	cor_euler_ang = get_corrected_orientation();

	return cor_euler_ang;
}
