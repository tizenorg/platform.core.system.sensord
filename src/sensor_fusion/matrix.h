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

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <assert.h>
#include <iostream>
using namespace std;

#define T_DEF template<typename TYPE, size_t ROW, size_t COL>
#define T_DEF1 template<typename T, size_t R, size_t C>
#define T_DEF2 template<typename T, size_t R, size_t C, size_t C2>
#define T_DEF3 template<typename T, size_t R1, size_t C1, size_t R2, size_t C2>

T_DEF class matrix {
public:
	TYPE m_mat[ROW][COL];

	matrix(void);
	matrix(TYPE mat_data[ROW][COL]);
	matrix(const matrix<TYPE, ROW, COL>& m);
	~matrix();

	matrix<TYPE, ROW, COL> operator =(const matrix<TYPE, ROW, COL>& m);

	T_DEF1 friend ostream& operator << (ostream& dout, matrix<T, R, C>& m);
	T_DEF1 friend matrix<T, R, C> operator +(const matrix<T, R, C> m1, const matrix<T, R, C> m2);
	T_DEF1 friend matrix<T, R, C> operator +(const matrix<T, R, C> m, const T val);
	T_DEF1 friend matrix<T, R, C> operator -(const matrix<T, R, C> m1, const matrix<T, R, C> m2);
	T_DEF1 friend matrix<T, R, C> operator -(const matrix<T, R, C> m, const T val);
	T_DEF2 friend matrix<T, R, C2> operator *(const matrix<T, R, C> m1, const matrix<T, C, C2> m2);
	T_DEF1 friend matrix<T, R, C> operator *(const matrix<T, R, C> m, const T val);
	T_DEF1 friend matrix<T, R, C> operator /(const matrix<T, R, C> m1, const T val);
	T_DEF3 friend bool operator ==(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2);
	T_DEF3 friend bool operator !=(const matrix<T, R1, C1> m1, const matrix<T, R2, C2> m2);
	T_DEF1 friend matrix<T, R, C> tran(const matrix<T, R, C> m);
	T_DEF1 friend matrix<T, R, 1> mul(const matrix<T, R, C> m1, const matrix<T, R, C> m2);
};

#include "matrix.cpp"

#endif  //_MATRIX_H_
