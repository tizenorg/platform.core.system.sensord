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

#include "matrix.h"

template <typename TYPE>
class vector {
public:
	int m_size;
	TYPE *m_vec;

	vector(void);
	vector(const int size);
	vector(const int size, TYPE *vec_data);
	vector(const vector<TYPE>& v);
	~vector();

	vector<TYPE> operator =(const vector<TYPE>& v);

	template<typename T> friend ostream& operator << (ostream& dout,
			vector<T>& v);
	template<typename T> friend vector<T> operator +(const vector<T> v1,
			const vector<T> v2);
	template<typename T> friend vector<T> operator +(const vector<T> v,
			const T val);
	template<typename T> friend vector<T> operator -(const vector<T> v1,
			const vector<T> v2);
	template<typename T> friend vector<T> operator -(const vector<T> v,
			const T val);
	template<typename T> friend matrix<T> operator *(const matrix<T> v1,
			const vector<T> v2);
	template<typename T> friend vector<T> operator *(const vector<T> v,
			const matrix<T> m);
	template<typename T> friend vector<T> operator *(const vector<T> v,
			const T val);
	template<typename T> friend vector<T> operator /(const vector<T> v1,
			const T val);
	template<typename T> friend bool operator ==(const vector<T> v1,
			const vector<T> v2);
	template<typename T> friend bool operator !=(const vector<T> v1,
			const vector<T> v2);

	template<typename T> friend T mul(const vector<T> v, const matrix<T> m);
	template<typename T> friend matrix<T> transpose(const vector<T> v);
};

#include "vector.cpp"

#endif /* _VECTOR_H */
