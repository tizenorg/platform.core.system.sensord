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

#ifndef _VECTOR_H
#define _VECTOR_H

#include <matrix.h>

template <typename TYPE>
class vector {
public:
	int m_cols;
	TYPE *m_vec;

	vector(void);
	vector(const int cols);
	vector(const int cols, TYPE *vec_data);
	vector(const vector& v);
	~vector();

	vector operator =(const vector& v);
	matrix transpose(void);

	friend vector operator +(const vector v1, const vector v2);
	friend vector operator +(const vector v, const TYPE val);
	friend vector operator -(const vector v1, const vector v2);
	friend vector operator -(const vector v, const TYPE val);
	friend matrix operator *(const matrix v1, const vector v2);
	friend TYPE operator *(const vector v, const matrix m);
	friend vector operator *(const vector v, const TYPE val);
	friend vector operator /(const vector v1, const vector v2);
	friend bool operator ==(const vector v1, const vector v2);
	friend bool operator !=(const vector v1, const vector v2);
};

#endif  //_VECTOR_H
