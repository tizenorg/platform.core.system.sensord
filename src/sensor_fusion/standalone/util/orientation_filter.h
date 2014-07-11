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

#ifndef _ORIENTATION_FILTER_H
#define _ORIENTATION_FILTER_H

#include "matrix.h"
#include "vector.h"
#include "sensor_data.h"
#include "quaternion.h"
#include "euler_angles.h"
#include "rotation_matrix.h"

template <typename TYPE>
class orientation_filter {
public:
	sensor_data<TYPE> m_filt_accel[2];
	sensor_data<TYPE> m_filt_gyro[2];
	sensor_data<TYPE> m_filt_magnetic[2];
	vector<TYPE> m_var_gyr_x;
	vector<TYPE> m_var_gyr_y;
	vector<TYPE> m_var_gyr_z;
	vector<TYPE> m_var_roll;
	vector<TYPE> m_var_pitch;
	vector<TYPE> m_var_yaw;
	matrix<TYPE> m_driv_cov;
	matrix<TYPE> m_aid_cov;
	matrix<TYPE> m_tran_mat;
	matrix<TYPE> m_measure_mat;
	matrix<TYPE> m_pred_cov;
	vector<TYPE> m_state_new;
	vector<TYPE> m_state_old;
	vector<TYPE> m_state_error;
	vector<TYPE> m_bias_correction;
	quaternion<TYPE> m_quat_aid;
	quaternion<TYPE> m_quat_driv;
	rotation_matrix<TYPE> m_rot_matrix;
	euler_angles<TYPE> m_orientation;

	orientation_filter();
	~orientation_filter();

	inline void filter_sensor_data(const sensor_data<TYPE> accel,
			const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic);
	inline void orientation_triad_algorithm();
	inline void compute_covariance();
	inline void time_update();
	inline void measurement_update();

	euler_angles<TYPE> get_orientation(const sensor_data<TYPE> accel,
			const sensor_data<TYPE> gyro, const sensor_data<TYPE> magnetic);
};

#include "orientation_filter.cpp"

#endif /* _ORIENTATION_FILTER_H */
