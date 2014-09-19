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

#include "../../../orientation_filter.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define ORIENTATION_DATA_PATH "../../../design/data/100ms/orientation/roll_pitch_yaw/"
#define ORIENTATION_DATA_SIZE 1095

float bias_accel[] = {0.098586, 0.18385, (10.084 - GRAVITY)};
float bias_gyro[] = {-5.3539, 0.24325, 2.3391};
float bias_magnetic[] = {0, 0, 0};
int sign_accel[] = {+1, +1, +1};
int sign_gyro[] = {+1, +1, +1};
int sign_magnetic[] = {+1, +1, +1};
float scale_accel = 1;
float scale_gyro = 575;
float scale_magnetic = 1;

int pitch_phase_compensation = -1;
int roll_phase_compensation = -1;
int yaw_phase_compensation = -1;
int magnetic_alignment_factor = -1;

void pre_process_data(sensor_data<float> &data_out, sensor_data<float> &data_in, float *bias, int *sign, float scale)
{
	data_out.m_data.m_vec[0] = sign[0] * (data_in.m_data.m_vec[0] - bias[0]) / scale;
	data_out.m_data.m_vec[1] = sign[1] * (data_in.m_data.m_vec[1] - bias[1]) / scale;
	data_out.m_data.m_vec[2] = sign[2] * (data_in.m_data.m_vec[2] - bias[2]) / scale;

	data_out.m_time_stamp = data_in.m_time_stamp;
}

int main()
{
	int data_available = ORIENTATION_DATA_SIZE;
	ifstream accel_in, gyro_in, mag_in;
	ofstream orien_file;
	string line_accel, line_gyro, line_magnetic;
	float sdata[3];
	unsigned long long time_stamp;
	euler_angles<float> orientation;
	orientation_filter<float> orien_filter;

	accel_in.open(((string)ORIENTATION_DATA_PATH + (string)"accel.txt").c_str());
	gyro_in.open(((string)ORIENTATION_DATA_PATH + (string)"gyro.txt").c_str());
	mag_in.open(((string)ORIENTATION_DATA_PATH + (string)"magnetic.txt").c_str());

	orien_file.open(((string)"orientation.txt").c_str());

	char *token = NULL;

	while (data_available-- > 0)
	{
		getline(accel_in, line_accel);
		sdata[0] = strtof(line_accel.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull (token, NULL, 10);
		sensor_data<float> accel_data(sdata[0], sdata[1], sdata[2], time_stamp);

		getline(gyro_in, line_gyro);
		sdata[0] = strtof(line_gyro.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull (token, NULL, 10);
		sensor_data<float> gyro_data(sdata[0], sdata[1], sdata[2], time_stamp);

		getline(mag_in, line_magnetic);
		sdata[0] = strtof(line_magnetic.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull (token, NULL, 10);
		sensor_data<float> magnetic_data(sdata[0], sdata[1], sdata[2], time_stamp);

		pre_process_data(accel_data, accel_data, bias_accel, sign_accel, scale_accel);
		normalize(accel_data);
		pre_process_data(gyro_data, gyro_data, bias_gyro, sign_gyro, scale_gyro);
		pre_process_data(magnetic_data, magnetic_data, bias_magnetic, sign_magnetic, scale_magnetic);
		normalize(magnetic_data);

		cout << "Accel Data\t" << accel_data.m_data << "\t Time Stamp\t" << accel_data.m_time_stamp << "\n\n";
		cout << "Gyro Data\t" << gyro_data.m_data << "\t Time Stamp\t" << gyro_data.m_time_stamp << "\n\n";
		cout << "Magnetic Data\t" << magnetic_data.m_data << "\t Time Stamp\t" << magnetic_data.m_time_stamp << "\n\n";

		orien_filter.m_pitch_phase_compensation = pitch_phase_compensation;
		orien_filter.m_roll_phase_compensation = roll_phase_compensation;
		orien_filter.m_yaw_phase_compensation = yaw_phase_compensation;
		orien_filter.m_magnetic_alignment_factor = magnetic_alignment_factor;

		orientation = orien_filter.get_orientation(accel_data, gyro_data, magnetic_data);

		orien_file << orientation.m_ang;

		cout << "Orientation Data\t" << orientation.m_ang << "\n\n";
	}

	accel_in.close();
	gyro_in.close();
	mag_in.close();
	orien_file.close();

	return 0;
}
