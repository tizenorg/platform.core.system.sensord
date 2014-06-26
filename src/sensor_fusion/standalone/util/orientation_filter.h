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
	sensor_data<TYPE> filt_accel[2];
	sensor_data<TYPE> filt_gyro[2];
	sensor_data<TYPE> filt_magnetic[2];
	matrix<TYPE> transition_matrix;
	matrix<TYPE> prediction_covariance;
	vector<TYPE> state_new;
	vector<TYPE> state_old;
	vector<TYPE> state_error;
	vector<TYPE> bias_correction;
	quaternion<TYPE> quat_diff;
	quaternion<TYPE> quat_sum;
	quaternion<TYPE> quat_aid;
	quaternion<TYPE> quat_driv;
	quaternion<TYPE> quat_error;
	euler_angles<TYPE> euler_error;
	rotation_matrix<TYPE> rot_matrix;
	euler_angles<TYPE> orientation;

	orientation_filter();
	~orientation_filter();

	inline void filter_sensor_data(sensor_data<TYPE> accel,
			sensor_data<TYPE> gyro, sensor_data<TYPE> magnetic);
	inline void orientation_triad_algorithm();
	inline void compute_aiding_var();
	inline void compute_driving_var();
	inline void compute_process_covar();
	inline void compute_measurement_covar();
	inline void compute_prediction_covar();
	inline void compute_quat_diff();
	inline void compute_quat_sum();
	inline void update_quat_sum();
	inline void time_update();
	inline void measurement_update();
	inline euler_angles<TYPE> get_corrected_orientation();

	euler_angles<TYPE> get_orientation(sensor_data<TYPE> accel,
			sensor_data<TYPE> gyro, sensor_data<TYPE> magnetic);
};

#include "orientation_filter.cpp"

#endif /* _ORIENTATION_FILTER_H */
