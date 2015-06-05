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

#ifdef _ORIENTATION_SENSOR_H_

float bias_accel[] = {0.098586, 0.18385, (10.084 - GRAVITY)};
float bias_gyro[] = {-5.3539, 0.24325, 2.3391};
float bias_magnetic[] = {0, 0, 0};
int sign_accel[] = {+1, +1, +1};
int sign_gyro[] = {+1, +1, +1};
int sign_magnetic[] = {+1, +1, +1};
float scale_accel = 1;
float scale_gyro = 1150;
float scale_magnetic = 1;
int magnetic_alignment_factor = -1;

void pre_process_data(sensor_data<float> *data_out, sensor_data<float> *data_in, float *bias, int *sign, float scale)
{
	data_out->m_data.m_vec[0] = sign[0] * (data_in->m_data.m_vec[0] - bias[0]) / scale;
	data_out->m_data.m_vec[1] = sign[1] * (data_in->m_data.m_vec[1] - bias[1]) / scale;
	data_out->m_data.m_vec[2] = sign[2] * (data_in->m_data.m_vec[2] - bias[2]) / scale;

	data_out->m_time_stamp = data_in->m_time_stamp;
}

void orientation_sensor::get_device_orientation(sensor_data<float> *accel_data,
		sensor_data<float> *gyro_data, sensor_data<float> *magnetic_data)
{
	vect<float, 3> vec_bias_gyro(bias_gyro);

	pre_process_data(accel_data, accel_data, bias_accel, sign_accel, scale_accel);
	normalize(*accel_data);
	pre_process_data(gyro_data, gyro_data, bias_gyro, sign_gyro, scale_gyro);
	pre_process_data(magnetic_data, magnetic_data, bias_magnetic, sign_magnetic, scale_magnetic);
	normalize(*magnetic_data);

	orien_filter.m_magnetic_alignment_factor = magnetic_alignment_factor;

	orien_filter.get_device_orientation(accel_data, gyro_data, magnetic_data);

	orien_filter.m_gyro_bias = orien_filter.m_gyro_bias + vec_bias_gyro;
}

#endif
