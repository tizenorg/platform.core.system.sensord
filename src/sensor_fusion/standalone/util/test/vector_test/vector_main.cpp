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

#include "../../vector.h"

int main()
{
	float arr2[3] = {0.056,2.34,-65.76};
	float arr15[3] = {1.04,-4.678,-2.34};
	float arr3[4] = {1.03,2.345,6.78,5.55};
	float arr4[4] = {-6.78,-45.213,-7.89,-3.456};
	float arr8[5] = {0.0123,5.678,2.345,0.345,8.956};
	float arr0[5] = {2344.98,345.24,456.12,98.33,144.67};
	float arr1[6] = {1.234,4.5,6.8987,3.33,5.66,77.695};
	float arr43[6] = {2.3454,-0.0384,-8.90,3.455,6.785,21.345};
	float arr5[5] = {0.2,-0.4,0.6,-0.8,1.0};

	vector<float> v1(5, arr0);
	vector<float> v2(5, arr8);
	vector<float> v10(4, arr3);
	vector<float> v12(4, arr4);
	vector<float> v15(6, arr1);
	vector<float> v20(6, arr43);
	vector<float> v21(3, arr2);
	vector<float> v22(3, arr15);
	vector<float> v3(4);
	vector<float> v6(3);
	vector<float> v13(5);
	vector<float> v95(6);
	vector<float> v35(5, arr5);

	float arr57[3][3] = {{2.24, 0.5, 0.023}, {3.675, 5.32, 0.556}, {1.023, 45.75, 621.6}};
	matrix<float> m12(3, 3, (float *) arr57);
	float arr67[3][1] = {{2.0}, {3.0}, {4.0}};
	matrix<float> m32(3, 1, (float *) arr67);

	cout<< "Constructor Test\n";
	cout<< "\n" << v3;

	cout<< "\n\nAddition\n";
	v3 = v21 + v22;
	v95 = v15 + v20;
	cout<< "\n" << v21 << "\n" << v22;
	cout<< "\nSum:\n" << v3 << endl;
	cout<< "\n" << v15 << "\n" << v20;
	cout<< "\nSum:\n" << v95 << endl;

	float num = 5.5650;
	float num1 = -2.32;
	cout<< "\n\n\nScalar addition\n";
	v3 = v2 + num;
	v6 = v10 + num1;
	cout<< "\nNumber added:" << num;
	cout<< "\n\n" << v2;
	cout<< "\nResult:\n\n" << v3;
	cout<< "\nNumber added:"<< num1;
	cout<< "\n\n" << v10;
	cout<< "\nResult:\n\n"<< v6;

	cout<< "\n\n\nSubtraction\n";
	v6 = v10 - v12;
	cout<< "\n" << v10 << "\n" << v12;
	cout<< "\nDifference:\n" << v6 << endl;

	float x = 4.0;
	float x1 = -2.5;
	cout<< "\n\n\nScalar subtraction\n";
	v13 = v1 - x;
	v6 = v10 - x1;
	cout<< "\nNumber Subtracted:" << x;
	cout<< "\n\n" << v1;
	cout<< "\nResult:\n\n" << v13;
	cout<< "\nNumber Subtracted:" << x1;
	cout<< "\n\n" << v10;
	cout<< "\nResult:\n\n" << v6;

	float xx = 7.2;
	cout<<"\n\n\nMultiplication\n";
	v13 = v2 * xx;
	cout<< "\n" << v2 <<"\n"<< xx;
	cout<< "\nProduct:\n" << v13 << endl;

	cout<< "\n\n\nMultiplication matrix x vector:\n";
	matrix<float> m102 = m32 * v22;
	cout<< "\n" << m32 <<"\n"<< v22;
	cout<< "\nProduct:\n"<< m102 << endl;

	cout<< "\n\n\nVector x Multiplication matrix:\n";
	vector<float> v102 = (v22 * m12);
	cout<< "\n" << v22 << "\n" << m12;
	cout<< "\nProduct:\n" << v102 << endl;
	float val = mul(v22, m32);
	cout<< "\n" << v22 << "\n" << m32;
	cout<< "\nProduct:\n" << val << endl;

	float z = 3.50;
	float z1 = -5.567;
	cout<< "\n\n\nScalar multiplication\n";
	v13 = v1 * z;
	v6 = v12 * z1;
	cout<< "\nNumber Multiplied:" << z;
	cout<< "\n\n" << v1;
	cout<< "\nResult:\n\n" << v13;
	cout<< "\nNumber Multiplied:" << z1;
	cout<< "\n\n" << v12;
	cout<< "\nResult:\n\n" << v6;

	float num2 = 5.5;
	cout<< "\n\n\nDivision\n";
	v3 = v1 / num2;
	cout<< "\n" << v1 << "\n" << num2;
	cout<< "\nResult:\n" << v3 << endl;

	cout<< "\n\n\nTranspose:";
	cout << "\n\n" << v20;
	cout << "\nResult:\n\n";
	matrix<float> m101 = (transpose(v20));
	cout << m101;
	cout << "\n\n" << m101;
	cout << "\nResult:\n\n";
	v20 = (tran(m101));
	cout << v20;

	cout << "\n\nv1:\n\n" << v1;
	cout << "\n\nv2:\n\n" << v2;
	cout << "\n\n\nv1 == v2 :";
	cout << (v1 == v2);

	cout << "\n\nv10:\n\n" << v10;
	cout << "\n\nv10:\n\n" << v10;
	cout << "\n\n\nv10 == v10 :";
	cout << (v10 == v10);

	cout << "\n\nv12:\n\n" << v12;
	cout << "\n\nv15:\n\n" << v15;
	cout << "\n\n\nv12 != v15 :";
	cout << (v12 != v15);

	cout << "\n\nv15:\n\n" << v15;
	cout << "\n\nv15:\n\n" << v15;
	cout << "\n\n\nv15 != v15 :";
	cout << (v15 != v15);

	cout<< "\n\nAssignment\n";
	v3 = v1;
	cout<< "Input \n" << v1;
	cout<< "\nOutput:\n" << v3 << endl;


	vector<float> v111 = cross(v21, v22);
	cout<< "\n\n\nCross Product:";
	cout << "\n\n" << v21 << "\n\n" << v22;
	cout << "\nResult:\n\n" << v111;

	cout <<  "\n\n\nQueue insert function:";
	cout << "\nInput:\n\n" << v111;
	insert_end(v111, (float) 0.9191919);
	cout << "\nResult:\n\n" << v111;

	cout <<  "\n\n\nVariance:";
	cout << "\nInput:\n\n" << v35;
	val = var(v35);
	cout << "\nResult:\n\n" << val;

	cout <<  "\n\n\nIs Initialized:";
	cout << "\nInput:\n\n" << v35;
	cout << "\nResult:\n\n" << is_initialized(v35);
}

