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

#include "../../sensor_data.h"

int main()
{
	float arr1[3] = {1.04, -4.678, -2.34};

	vector<float> v1(3, arr1);

	sensor_data<float> sd1(2.0, 3.0, 4.0, 140737488355328);
	sensor_data<float> sd2(1.04, -4.678, -2.34);
	sensor_data<float> sd3(0.054, 1.097, 4.456, 140737488355328);
	sensor_data<float> sd10(v1, 140737488355328);

	cout << "Constructor tests\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << sd10.m_data << "\t" << sd10.m_time_stamp << "\n\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << sd2.m_data << "\t" << sd2.m_time_stamp << "\n\n";

	cout<< "Addition:\n";
	sensor_data<float> sd4 = sd1 + sd2;
	cout<< "\n" << sd1.m_data << "\n" << sd2.m_data;
	cout<< "\nSum:\n" << sd4.m_data << endl;
	sensor_data<float> sd9 = sd1 + sd10;
	cout<< "\n" << sd1.m_data << "\n" << sd10.m_data;
	cout<< "\nSum:\n" << sd9.m_data << endl;

	cout<< "\n\n\nNormalization:\n";
	sensor_data<float> sd6 = normalize(sd3);
	cout<< "\n" << sd3.m_data;
	cout<< "\nResult:\n" << sd6.m_data << endl;
	sensor_data<float> sd7 = normalize(sd2);
	cout<< "\n" << sd2.m_data;
	cout<< "\nResult:\n" << sd7.m_data << endl;

	float xx = 2.5;
	cout<<"\n\n\nScale data:\n";
	sensor_data<float> sd8 = scale_data(sd2, xx);
	cout<< "\n" << sd2.m_data << "\n" << xx;
	cout<< "\nResult:\n" << sd8.m_data << endl;
}

