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


#ifdef _ORIENTATION_FILTER_H_

#include "orientation_filter.h"

//Windowing is used for buffering of previous samples for statistical analysis
#define MOVING_AVERAGE_WINDOW_LENGTH	20
//Earth's Gravity
#define GRAVITY		9.80665
#define PI		3.141593
//Needed for non-zero initialization for statistical analysis
#define NON_ZERO_VAL	0.1
//microseconds to seconds
#define US2S	(1.0 / 1000000.0)
//Initialize sampling interval to 100000microseconds
#define SAMPLE_INTV		100000
#define ACCEL_THRESHOLD 0.2
#define GYRO_THRESHOLD (0.01 * PI)

// constants for computation of covariance and transition matrices
#define ZIGMA_W		(0.05 * DEG2RAD)
#define TAU_W		1000
#define QWB_CONST	((2 * (ZIGMA_W * ZIGMA_W)) / TAU_W)
#define F_CONST		(-1 / TAU_W)
#define SQUARE(T)	(T * T)

#define NEGLIGIBLE_VAL 0.0000001

#define ABS(val) (((val) < 0) ? -(val) : (val))


template <typename TYPE>
orientation_filter<TYPE>::orientation_filter()
{
	TYPE arr[MOVING_AVERAGE_WINDOW_LENGTH];

	std::fill_n(arr, MOVING_AVERAGE_WINDOW_LENGTH, NON_ZERO_VAL);

	vect<TYPE, MOVING_AVERAGE_WINDOW_LENGTH> vec(arr);

	m_var_gyr_x = vec;
	m_var_gyr_y = vec;
	m_var_gyr_z = vec;
	m_var_roll = vec;
	m_var_pitch = vec;
	m_var_azimuth = vec;

	m_magnetic_alignment_factor = 1;

	m_gyro.m_time_stamp = 0;
}

template <typename TYPE>
orientation_filter<TYPE>::~orientation_filter()
{
}

template <typename TYPE>
inline void orientation_filter<TYPE>::initialize_sensor_data(const sensor_data<TYPE> *accel,
		const sensor_data<TYPE> *gyro, const sensor_data<TYPE> *magnetic)
{
	m_accel.m_data = accel->m_data;
	m_accel.m_time_stamp = accel->m_time_stamp;
	normalize(m_accel);

	if (gyro != NULL) {
		unsigned long long sample_interval_gyro = SAMPLE_INTV;

		if (m_gyro.m_time_stamp != 0 && gyro->m_time_stamp != 0)
			sample_interval_gyro = gyro->m_time_stamp - m_gyro.m_time_stamp;

		m_gyro_dt = sample_interval_gyro * US2S;
		m_gyro.m_time_stamp = gyro->m_time_stamp;

		m_gyro.m_data = gyro->m_data * (TYPE) PI;

		m_gyro.m_data = m_gyro.m_data - m_bias_correction;
	}

	if (magnetic != NULL) {
		m_magnetic.m_data = magnetic->m_data;
		m_magnetic.m_time_stamp = magnetic->m_time_stamp;
	}
}

template <typename TYPE>
inline void orientation_filter<TYPE>::orientation_triad_algorithm()
{
	TYPE arr_acc_e[V1x3S] = {0.0, 0.0, 1.0};
	TYPE arr_mag_e[V1x3S] = {0.0, (TYPE) m_magnetic_alignment_factor, 0.0};

	vect<TYPE, V1x3S> acc_e(arr_acc_e);
	vect<TYPE, V1x3S> mag_e(arr_mag_e);

	vect<TYPE, SENSOR_DATA_SIZE> acc_b_x_mag_b = cross(m_accel.m_data, m_magnetic.m_data);
	vect<TYPE, V1x3S> acc_e_x_mag_e = cross(acc_e, mag_e);

	vect<TYPE, SENSOR_DATA_SIZE> cross1 = cross(acc_b_x_mag_b, m_accel.m_data);
	vect<TYPE, V1x3S> cross2 = cross(acc_e_x_mag_e, acc_e);

	matrix<TYPE, M3X3R, M3X3C> mat_b;
	matrix<TYPE, M3X3R, M3X3C> mat_e;

	for(int i = 0; i < M3X3R; i++)
	{
		mat_b.m_mat[i][0] = m_accel.m_data.m_vec[i];
		mat_b.m_mat[i][1] = acc_b_x_mag_b.m_vec[i];
		mat_b.m_mat[i][2] = cross1.m_vec[i];
		mat_e.m_mat[i][0] = acc_e.m_vec[i];
		mat_e.m_mat[i][1] = acc_e_x_mag_e.m_vec[i];
		mat_e.m_mat[i][2] = cross2.m_vec[i];
	}

	matrix<TYPE, M3X3R, M3X3C> mat_b_t = tran(mat_b);
	rotation_matrix<TYPE> rot_mat(mat_e * mat_b_t);

	m_quat_aid = rot_mat2quat(rot_mat);
}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_accel_orientation()
{
	TYPE arr_acc_e[V1x3S] = {0.0, 0.0, 1.0};

	vect<TYPE, V1x3S> acc_e(arr_acc_e);

	m_quat_aid = sensor_data2quat(m_accel, acc_e);
}

template <typename TYPE>
inline void orientation_filter<TYPE>::compute_covariance()
{
	TYPE var_gyr_x, var_gyr_y, var_gyr_z;
	TYPE var_roll, var_pitch, var_azimuth;
	quaternion<TYPE> quat_diff, quat_error;

	if(!is_initialized(m_quat_driv.m_quat))
		m_quat_driv = m_quat_aid;

	quaternion<TYPE> quat_rot_inc(0, m_gyro.m_data.m_vec[0], m_gyro.m_data.m_vec[1],
			m_gyro.m_data.m_vec[2]);

	quat_diff = (m_quat_driv * quat_rot_inc) * (TYPE) 0.5;

	m_quat_driv = m_quat_driv + (quat_diff * (TYPE) m_gyro_dt * (TYPE) PI);
	m_quat_driv.quat_normalize();

	m_quat_output = phase_correction(m_quat_driv, m_quat_aid);

	m_orientation = quat2euler(m_quat_output);

	quat_error = m_quat_aid * m_quat_driv;

	m_euler_error = (quat2euler(quat_error)).m_ang;

	m_gyro.m_data = m_gyro.m_data - m_euler_error.m_ang;

	m_euler_error.m_ang = m_euler_error.m_ang / (TYPE) PI;

	m_gyro_bias = m_euler_error.m_ang * (TYPE) PI;

	insert_end(m_var_gyr_x, m_gyro.m_data.m_vec[0]);
	insert_end(m_var_gyr_y, m_gyro.m_data.m_vec[1]);
	insert_end(m_var_gyr_z, m_gyro.m_data.m_vec[2]);
	insert_end(m_var_roll, m_orientation.m_ang.m_vec[0]);
	insert_end(m_var_pitch, m_orientation.m_ang.m_vec[1]);
	insert_end(m_var_azimuth, m_orientation.m_ang.m_vec[2]);

	var_gyr_x = var(m_var_gyr_x);
	var_gyr_y = var(m_var_gyr_y);
	var_gyr_z = var(m_var_gyr_z);
	var_roll = var(m_var_roll);
	var_pitch = var(m_var_pitch);
	var_azimuth = var(m_var_azimuth);

	m_driv_cov.m_mat[0][0] = var_gyr_x;
	m_driv_cov.m_mat[1][1] = var_gyr_y;
	m_driv_cov.m_mat[2][2] = var_gyr_z;
	m_driv_cov.m_mat[3][3] = (TYPE) QWB_CONST;
	m_driv_cov.m_mat[4][4] = (TYPE) QWB_CONST;
	m_driv_cov.m_mat[5][5] = (TYPE) QWB_CONST;

	m_aid_cov.m_mat[0][0] = var_roll;
	m_aid_cov.m_mat[1][1] = var_pitch;
	m_aid_cov.m_mat[2][2] = var_azimuth;
}

template <typename TYPE>
inline void orientation_filter<TYPE>::time_update()
{
	euler_angles<TYPE> orientation;

	m_tran_mat.m_mat[0][1] = m_gyro.m_data.m_vec[2];
	m_tran_mat.m_mat[0][2] = -m_gyro.m_data.m_vec[1];
	m_tran_mat.m_mat[1][0] = -m_gyro.m_data.m_vec[2];
	m_tran_mat.m_mat[1][2] = m_gyro.m_data.m_vec[0];
	m_tran_mat.m_mat[2][0] = m_gyro.m_data.m_vec[1];
	m_tran_mat.m_mat[2][1] = -m_gyro.m_data.m_vec[0];
	m_tran_mat.m_mat[3][3] = (TYPE) F_CONST;
	m_tran_mat.m_mat[4][4] = (TYPE) F_CONST;
	m_tran_mat.m_mat[5][5] = (TYPE) F_CONST;

	m_measure_mat.m_mat[0][0] = 1;
	m_measure_mat.m_mat[1][1] = 1;
	m_measure_mat.m_mat[2][2] = 1;

	if (is_initialized(m_state_old))
		m_state_new = transpose(m_tran_mat * transpose(m_state_old));

	m_pred_cov = (m_tran_mat * m_pred_cov * tran(m_tran_mat)) + m_driv_cov;

	for (int j = 0; j < M6X6C; ++j) {
		for (int i = 0; i < M6X6R; ++i) {
			if (ABS(m_pred_cov.m_mat[i][j]) < NEGLIGIBLE_VAL)
				m_pred_cov.m_mat[i][j] = NEGLIGIBLE_VAL;
		}
		if (ABS(m_state_new.m_vec[j]) < NEGLIGIBLE_VAL)
			m_state_new.m_vec[j] = NEGLIGIBLE_VAL;
	}

	m_quat_9axis = m_quat_output;
	m_quat_gaming_rv = m_quat_9axis;

	m_rot_matrix = quat2rot_mat(m_quat_driv);

	quaternion<TYPE> quat_eu_er(1, m_euler_error.m_ang.m_vec[0], m_euler_error.m_ang.m_vec[1],
			m_euler_error.m_ang.m_vec[2]);

	m_quat_driv = (m_quat_driv * quat_eu_er) * (TYPE) PI;
	m_quat_driv.quat_normalize();

	if (is_initialized(m_state_new)) {
		m_state_error.m_vec[0] = m_euler_error.m_ang.m_vec[0];
		m_state_error.m_vec[1] = m_euler_error.m_ang.m_vec[1];
		m_state_error.m_vec[2] = m_euler_error.m_ang.m_vec[2];
		m_state_error.m_vec[3] = m_state_new.m_vec[3];
		m_state_error.m_vec[4] = m_state_new.m_vec[4];
		m_state_error.m_vec[5] = m_state_new.m_vec[5];
	}
}

template <typename TYPE>
inline void orientation_filter<TYPE>::time_update_gaming_rv()
{
	euler_angles<TYPE> orientation;
	euler_angles<TYPE> euler_aid;
	euler_angles<TYPE> euler_driv;

	m_tran_mat.m_mat[0][1] = m_gyro.m_data.m_vec[2];
	m_tran_mat.m_mat[0][2] = -m_gyro.m_data.m_vec[1];
	m_tran_mat.m_mat[1][0] = -m_gyro.m_data.m_vec[2];
	m_tran_mat.m_mat[1][2] = m_gyro.m_data.m_vec[0];
	m_tran_mat.m_mat[2][0] = m_gyro.m_data.m_vec[1];
	m_tran_mat.m_mat[2][1] = -m_gyro.m_data.m_vec[0];
	m_tran_mat.m_mat[3][3] = (TYPE) F_CONST;
	m_tran_mat.m_mat[4][4] = (TYPE) F_CONST;
	m_tran_mat.m_mat[5][5] = (TYPE) F_CONST;

	m_measure_mat.m_mat[0][0] = 1;
	m_measure_mat.m_mat[1][1] = 1;
	m_measure_mat.m_mat[2][2] = 1;

	if (is_initialized(m_state_old))
		m_state_new = transpose(m_tran_mat * transpose(m_state_old));

	m_pred_cov = (m_tran_mat * m_pred_cov * tran(m_tran_mat)) + m_driv_cov;

	euler_aid = quat2euler(m_quat_aid);
	euler_driv = quat2euler(m_quat_output);

	if ((SQUARE(m_accel.m_data.m_vec[1]) < ACCEL_THRESHOLD) && (SQUARE(m_gyro.m_data.m_vec[0]) < GYRO_THRESHOLD)) {
		if ((SQUARE(m_accel.m_data.m_vec[0]) < ACCEL_THRESHOLD) && (SQUARE(m_gyro.m_data.m_vec[1]) < GYRO_THRESHOLD)) {
			if (SQUARE(m_gyro.m_data.m_vec[2]) < GYRO_THRESHOLD) {
				euler_angles<TYPE> euler_gaming_rv(euler_aid.m_ang.m_vec[0], euler_aid.m_ang.m_vec[1],
						euler_driv.m_ang.m_vec[2]);
				m_quat_gaming_rv = euler2quat(euler_gaming_rv);
			}
		}
	}

	if (is_initialized(m_state_new)) {
		m_state_error.m_vec[0] = m_euler_error.m_ang.m_vec[0];
		m_state_error.m_vec[1] = m_euler_error.m_ang.m_vec[1];
		m_state_error.m_vec[2] = m_euler_error.m_ang.m_vec[2];
		m_state_error.m_vec[3] = m_state_new.m_vec[3];
		m_state_error.m_vec[4] = m_state_new.m_vec[4];
		m_state_error.m_vec[5] = m_state_new.m_vec[5];
	}
}

template <typename TYPE>
inline void orientation_filter<TYPE>::measurement_update()
{
	matrix<TYPE, M6X6R, M6X6C> gain;
	matrix<TYPE, M6X6R, M6X6C> iden;
	iden.m_mat[0][0] = iden.m_mat[1][1] = iden.m_mat[2][2] = 1;
	iden.m_mat[3][3] = iden.m_mat[4][4] = iden.m_mat[5][5] = 1;

	for (int j = 0; j < M6X6C; ++j) {
		for (int i = 0; i < M6X6R; ++i) {
			gain.m_mat[i][j] = m_pred_cov.m_mat[j][i] / (m_pred_cov.m_mat[j][j] + m_aid_cov.m_mat[j][j]);
			m_state_new.m_vec[i] = m_state_new.m_vec[i] + gain.m_mat[i][j] * m_state_error.m_vec[j];
		}

		matrix<TYPE, M6X6R, M6X6C> temp = iden;

		for (int i = 0; i < M6X6R; ++i)
			temp.m_mat[i][j] = iden.m_mat[i][j] - (gain.m_mat[i][j] * m_measure_mat.m_mat[j][i]);

		m_pred_cov = temp * m_pred_cov;
	}

	for (int j = 0; j < M6X6C; ++j) {
		for (int i = 0; i < M6X6R; ++i) {
			if (ABS(m_pred_cov.m_mat[i][j]) < NEGLIGIBLE_VAL)
				m_pred_cov.m_mat[i][j] = NEGLIGIBLE_VAL;
		}
	}

	m_state_old = m_state_new;

	TYPE arr_bias[V1x3S] = {m_state_new.m_vec[3], m_state_new.m_vec[4], m_state_new.m_vec[5]};
	vect<TYPE, V1x3S> vec(arr_bias);

	m_bias_correction = vec;

	m_gyro_bias = m_gyro_bias + vec;
}

template <typename TYPE>
void orientation_filter<TYPE>::get_device_orientation(const sensor_data<TYPE> *accel,
		const sensor_data<TYPE> *gyro, const sensor_data<TYPE> *magnetic)
{
	initialize_sensor_data(accel, gyro, magnetic);

	if (gyro != NULL && magnetic != NULL) {
		orientation_triad_algorithm();
		compute_covariance();
		time_update();
		measurement_update();
		m_quaternion = m_quat_9axis;
	} else if (!gyro && !magnetic) {
		compute_accel_orientation();
		m_quaternion = m_quat_aid;
	} else if (!gyro) {
		orientation_triad_algorithm();
		m_quaternion = m_quat_aid;
	} else if (!magnetic) {
		compute_accel_orientation();
		compute_covariance();
		time_update_gaming_rv();
		measurement_update();
		m_quaternion = m_quat_gaming_rv;
	}
}

#endif  //_ORIENTATION_FILTER_H_
