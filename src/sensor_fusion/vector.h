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

template <typename TYPE,size_t SIZE>
class vect {
public:
	TYPE m_vec[SIZE];
	vect(void);
	vect(TYPE vec_data[SIZE]);
	vect(const vect<TYPE,SIZE>& v);
	~vect();

	vect<TYPE,SIZE> operator =(const vect<TYPE,SIZE>& v);

	template<typename T,size_t S> friend ostream& operator << (ostream& dout,
			vect<T,S>& v);
	template<typename T,size_t S> friend vect<T,S> operator +(const vect<T,S> v1,
			const vect<T,S> v2);
	template<typename T,size_t S> friend vect<T,S> operator +(const vect<T,S> v,
			const T val);
	template<typename T,size_t S> friend vect<T,S> operator -(const vect<T,S> v1,
			const vect<T,S> v2);
	template<typename T,size_t S> friend vect<T,S> operator -(const vect<T,S> v,
			const T val);
	template<typename T,size_t S,size_t R,size_t C> friend matrix<T, R, S> operator *(const matrix<T,R,C> v1,
			const vect<T,S> v2);
	template<typename T,size_t S,size_t R,size_t C> friend vect<T,S> operator *(const vect<T,S> v,
			const matrix<T, R, C> m);
	template<typename T,size_t S> friend vect<T,S> operator *(const vect<T,S> v,
			const T val);
	template<typename T,size_t S> friend vect<T,S> operator /(const vect<T,S> v1,
			const T val);
	template<typename T,size_t S1, size_t S2> friend bool operator ==(const vect<T,S1> v1,
			const vect<T,S2> v2);
	template<typename T,size_t S1, size_t S2> friend bool operator !=(const vect<T,S1> v1,
			const vect<T,S2> v2);

	template<typename T,size_t S,size_t R,size_t C> friend T mul(const vect<T,S> v, const matrix<T,R,C> m);
	template<typename T,size_t S> friend void insert_end(vect<T,S>& v, T val);
	template<typename T,size_t S> friend matrix<T,S,1> transpose(const vect<T,S> v);
	template <typename T,size_t R> friend vect<T,R> transpose(const matrix<T, R, 1> m);
	template<typename T,size_t S> friend vect<T,S> cross(const vect<T,S> v1,
			const vect<T,S> v2);
	template <typename T,size_t S> friend T var(const vect<T,S> v);
	template <typename T,size_t S> friend bool is_initialized(const vect<T,S> v);
};

#include "vector.cpp"

#endif /* _VECTOR_H_ */

