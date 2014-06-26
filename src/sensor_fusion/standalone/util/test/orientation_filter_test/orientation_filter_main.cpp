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
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define ORIENTATION_DATA_PATH "../../../../design/data/100ms/orientation/roll_pitch_yaw/"
#define ORIENTATION_DATA_SIZE 100

int main()
{
	int data_available = ORIENTATION_DATA_SIZE;
	ifstream accel_file, gyro_file, magnetic_file;
	string line;

	accel_file.open(((string)ORIENTATION_DATA_PATH + (string)"accel.txt").c_str());
	gyro_file.open(((string)ORIENTATION_DATA_PATH + (string)"gyro.txt").c_str());
	magnetic_file.open(((string)ORIENTATION_DATA_PATH + (string)"magnetic.txt").c_str());

	while (data_available-- > 0)
	{
		getline(accel_file, line);
		cout << line << '\n';
		getline(gyro_file, line);
		cout << line << '\n';
		getline(magnetic_file, line);
		cout << line << '\n';
	}

	return 0;
}
