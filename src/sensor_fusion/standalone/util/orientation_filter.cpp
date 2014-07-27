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

#define M3X3R	3
#define M3x3C	3

#define V1x3S	3

template <typename TYPE>
orientation_filter<TYPE>::orientation_filter()
{

}

template <typename TYPE>
orientation_filter<TYPE>::~orientation_filter()
{

}

template <typename TYPE>
inline void orientation_filter<TYPE>::filter_sensor_data(const sensor_data<TYPE> accel,
		const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic)
{
	const TYPE iir_b[] = {0.98, 0};
	const TYPE iir_a[] = {1.0000000, 0.02};

	filt_accel[0] = filt_accel[1];
	filt_gyro[0] = filt_gyro[1];
	filt_magnetic[0] = filt_magnetic[1];

	filt_accel[1].m_data = accel.m_data * iir_b[0] - filt_accel[0].m_data * iir_a[1];
	filt_gyro[1].m_data = gyro.m_data * iir_b[0]  - filt_gyro[0].m_data * iir_a[1];
	filt_magnetic[1].m_data = magnetic.m_data * iir_b[0] - filt_magnetic[0].m_data * iir_a[1];

	filt_accel[1].m_time_stamp = accel.m_time_stamp;
	filt_gyro[1].m_time_stamp = accel.m_time_stamp;
	filt_magnetic[1].m_time_stamp = accel.m_time_stamp;
}

template <typename TYPE>
inline void orientation_filter<TYPE>::orientation_triad_algorithm()
{
	TYPE arr_acc_e[V1x3S] = {0.0, 0.0, 1.0};
	TYPE arr_mag_e[V1x3S] = {0.0, 1.0, 0.0};

	// gravity vector in earth frame
	vector<TYPE> acc_e(V1x3S, arr_acc_e);
	// magnetic field vector in earth frame
	vector<TYPE> mag_e(V1x3S, arr_mag_e);

	vector<TYPE> acc_b_x_mag_b = cross(filt_accel[1].m_data, filt_magnetic[1].m_data);
	vector<TYPE> acc_e_x_mag_e = cross(acc_e, mag_e);

	vector<TYPE> cross1 = cross(acc_b_x_mag_b, filt_accel[1].m_data);
	vector<TYPE> cross2 = cross(acc_e_x_mag_e, acc_e);

	matrix<TYPE> mat_b(M3X3R, M3x3C);
	matrix<TYPE> mat_e(M3X3R, M3x3C);

	for(int i = 0; i < M3X3R; i++)
	{
		mat_b.m_mat[i][0] = filt_accel[1].m_data.m_vec[i];
		mat_b.m_mat[i][1] = acc_b_x_mag_b.m_vec[i];
		mat_b.m_mat[i][2] = cross1.m_vec[i];
		mat_e.m_mat[i][0] = acc_e.m_vec[i];
		mat_e.m_mat[i][1] = acc_e_x_mag_e.m_vec[i];
		mat_e.m_mat[i][2] = cross2.m_vec[i];
	}

	matrix<TYPE> mat_b_t = transpose(mat_b);
	rotation_matrix<TYPE> rot_mat(mat_e * mat_b_t);
	quaternion<TYPE> quat_aid = rot_mat2quat(rot_mat);
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
euler_angles<TYPE> orientation_filter<TYPE>::get_orientation(const sensor_data<TYPE> accel,
		const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic)
{
	euler_angles<TYPE> cor_euler_ang;

	filter_sensor_data(accel, gyro, magnetic);

	normalize(filt_accel[1]);
	filt_gyro[1].m_data = filt_gyro[1].m_data * (TYPE) PI;
	normalize(filt_magnetic[1]);

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
