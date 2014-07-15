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

#include "../../orientation_filter.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define ORIENTATION_DATA_PATH "../../../../design/data/100ms/orientation/roll_pitch_yaw/"
#define ORIENTATION_DATA_SIZE 1095

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

		cout << "Accel Data\t" << accel_data.m_data << "\t Time Stamp\t" << accel_data.m_time_stamp << "\n\n";

		getline(gyro_in, line_gyro);
		sdata[0] = strtof(line_gyro.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull (token, NULL, 10);
		sensor_data<float> gyro_data(sdata[0], sdata[1], sdata[2], time_stamp);

		cout << "Gyro Data\t" << gyro_data.m_data << "\t Time Stamp\t" << gyro_data.m_time_stamp << "\n\n";

		getline(mag_in, line_magnetic);
		sdata[0] = strtof(line_magnetic.c_str(), &token);
		sdata[1] = strtof(token, &token);
		sdata[2] = strtof(token, &token);
		time_stamp = strtoull (token, NULL, 10);
		sensor_data<float> magnetic_data(sdata[0], sdata[1], sdata[2], time_stamp);

		cout << "Magnetic Data\t" << magnetic_data.m_data << "\t Time Stamp\t" << magnetic_data.m_time_stamp << "\n\n";

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
