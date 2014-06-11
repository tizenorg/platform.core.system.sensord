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

#ifndef _MATRIX_H
#define _MATRIX_H

template <typename TYPE>
class matrix {
public:
	int m_rows;
	int m_cols;
	TYPE **m_mat;

	matrix(void);
	matrix(const int rows, const int cols);
	matrix(const int rows, const int cols, TYPE **mat_data);
	matrix(const matrix& m);
	~matrix();

	matrix operator =(const matrix& m);

	friend matrix operator +(const matrix m1, const matrix m2);
	friend matrix operator +(const matrix m, const TYPE val);
	friend matrix operator -(const matrix m1, const matrix m2);
	friend matrix operator -(const matrix m, const TYPE val);
	friend matrix operator *(const matrix m1, const matrix m2);
	friend matrix operator *(const matrix m, const TYPE val);
	friend matrix operator /(const matrix m1, const matrix m2);
	friend bool operator ==(const matrix m1, const matrix m2);
	friend bool operator !=(const matrix m1, const matrix m2);
};

#endif  //_MATRIX_H
