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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "matrix.h"

#define T_VDEF template<typename TYPE, int SIZE>
#define T_VDEF1 template<typename T, int S>
#define T_VDEF2 template<typename T, int S, int R, int C>
#define T_VDEF3 template<typename T, int S1, int S2>
#define T_VDEF4 template<typename T, int R>

T_VDEF class vect {
public:
	TYPE m_vec[SIZE];
	vect(void);
	vect(TYPE vec_data[SIZE]);
	vect(const vect<TYPE, SIZE>& v);
	~vect();

	vect<TYPE,SIZE> operator =(const vect<TYPE, SIZE>& v);

	T_VDEF1 friend ostream& operator << (ostream& dout, vect<T, S>& v);
	T_VDEF1 friend vect<T, S> operator +(const vect<T, S> v1, const vect<T, S> v2);
	T_VDEF1 friend vect<T, S> operator +(const vect<T, S> v, const T val);
	T_VDEF1 friend vect<T, S> operator -(const vect<T,S> v1, const vect<T,S> v2);
	T_VDEF1 friend vect<T, S> operator -(const vect<T,S> v, const T val);
	T_VDEF2 friend matrix<T, R, S> operator *(const matrix<T, R, C> v1, const vect<T, S> v2);
	T_VDEF2 friend vect<T, S> operator *(const vect<T, S> v, const matrix<T, R, C> m);
	T_VDEF1 friend vect<T, S> operator *(const vect<T, S> v, const T val);
	T_VDEF1 friend vect<T, S> operator /(const vect<T,S> v1, const T val);
	T_VDEF3 friend bool operator ==(const vect<T, S1> v1, const vect<T, S2> v2);
	T_VDEF3 friend bool operator !=(const vect<T, S1> v1, const vect<T, S2> v2);

	T_VDEF1 friend void insert_end(vect<T, S>& v, T val);
	T_VDEF1 friend matrix<T, S, 1> transpose(const vect<T, S> v);
	T_VDEF4 friend vect<T, R> transpose(const matrix<T, R, 1> m);
	T_VDEF1 friend vect<T, S> cross(const vect<T, S> v1, const vect<T, S> v2);
	T_VDEF1 friend T dot(const vect<T, S> v1, const vect<T, S> v2);
	T_VDEF1 friend T var(const vect<T, S> v);
	T_VDEF1 friend bool is_initialized(const vect<T, S> v);
};

#include "vector.cpp"

#endif /* _VECTOR_H_ */

