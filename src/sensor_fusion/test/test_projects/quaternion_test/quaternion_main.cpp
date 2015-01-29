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

#include "../../../quaternion.h"

int main()
{
	float arr0[4] = {2344.98, 345.24, 456.12, 98.33};
	float arr1[4] = {0.056, 0.34, -0.0076, 0.001};
	float axis1[3] = {6.5, 7.5, 8.3};
	float ang1 = 8.4;
	float axis2[3] = {16.5, 17.5, 18.3};
	float ang2 = 10.4;
	float axis3[3] = {4.5, 7.5, 9.3};
	float ang3 = 11.5;

	vect<float, 4> v0(arr0);
	vect<float, 4> v1(arr1);

	quaternion<float> q0(v0);
	quaternion<float> q1(v1);
	quaternion<float> q2((float)2344.98, (float)345.24, (float)456.12, (float)98.33);
	quaternion<float> q3(q1);
	quaternion<float> q4;

	cout << "Constructor tests\n";
	cout << "input\t" << v0 << "\n";
	cout << "output\t" << q0.m_quat << "\n\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << q1.m_quat << "\n\n";
	cout << "input\t" << v0 << "\n";
	cout << "output\t" << q2.m_quat << "\n\n";
	cout << "input\t" << v1 << "\n";
	cout << "output\t" << q3.m_quat << "\n\n";
	cout << "default constructor\n";
	cout << "output\t" << q4.m_quat << "\n\n";

	cout << "Multiplication\n";
	float val = 0.1;
	quaternion<float> q5 = q0 * val;
	cout << "input\t" << q0.m_quat << "\n" << 0.1 << "\n";
	cout << "output\t" << q5.m_quat << "\n\n";
	quaternion<float> q6 = q0 * q1;
	cout << "input\t" << q0.m_quat << "\n" << q1.m_quat << "\n";
	cout << "output\t" << q6.m_quat << "\n\n";

	cout << "Addition\n";
	quaternion<float> q7 = q0 + q1;
	cout << "input\t" << q0.m_quat << "\n" << q1.m_quat << "\n";
	cout << "output\t" << q7.m_quat << "\n\n";

	cout << "Quaternion Normalization\n";
	cout << "input\t" << q1.m_quat << "\n";
	q1.quat_normalize();
	cout << "output\t" << q1.m_quat << "\n\n";

	cout << "Axis2quat\n";
	cout << "input\t" << " " << axis1[0]  << " " << axis1[1] <<" " << axis1[2] << " " << ang1 << endl;
	cout << endl;
	quaternion<float> q11 = axis2quat(axis1,ang1);
	cout << "output\t" << q11.m_quat << "\n\n";
	cout << "input\t" << " " << axis2[0]  << " " << axis2[1] <<" " << axis2[2] << " " << ang2 << endl;
	cout << endl;
	quaternion<float> q21 = axis2quat(axis2,ang2);
	cout << "output\t" << q21.m_quat << "\n\n";
	cout << "input\t" << " " << axis3[0]  << " " << axis3[1] <<" " << axis3[2] << " " << ang3 << endl;
	cout << endl;
	quaternion<float> q31 = axis2quat(axis3,ang3);
	cout << "output\t" << q31.m_quat << "\n\n";
}

