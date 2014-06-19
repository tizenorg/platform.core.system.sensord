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

#if defined (_QUATERNION_H) && defined (_VECTOR_H)

#include <math.h>

template <typename TYPE>
quaternion<TYPE>::quaternion() : m_quat(4)
{
}

template <typename TYPE>
quaternion<TYPE>::quaternion(const TYPE w, const TYPE x, const TYPE y, const TYPE z)
{
	TYPE vec_data[4] = {w, x, y, z};

	vector<TYPE> v(4, vec_data);
	m_quat = v;
}

template <typename TYPE>
quaternion<TYPE>::quaternion(const vector<TYPE> v)
{
	m_quat = v;
}

template <typename TYPE>
quaternion<TYPE>::quaternion(const quaternion<TYPE>& q)
{
	m_quat = q.m_quat;
}

template <typename TYPE>
quaternion<TYPE>::~quaternion()
{
}

template <typename TYPE>
quaternion<TYPE> quaternion<TYPE>::operator =(const quaternion<TYPE>& q)
{
	m_quat = q.m_quat;
}

template <typename T>
quaternion<T> operator *(quaternion<T> q, T val)
{
	return (q.m_quat * val);
}

template <typename T>
quaternion<T> operator *(quaternion<T> q1, quaternion<T> q2)
{
	T w, x, y, z;
	T w1, x1, y1, z1;
	T w2, x2, y2, z2;

	w1 = q1.m_quat.m_vec[0];
	x1 = q1.m_quat.m_vec[1];
	y1 = q1.m_quat.m_vec[2];
	z1 = q1.m_quat.m_vec[3];

	w2 = q2.m_quat.m_vec[0];
	x2 = q2.m_quat.m_vec[1];
	y2 = q2.m_quat.m_vec[2];
	z2 = q2.m_quat.m_vec[3];

	x = x1*w2 + y1*z2 - z1*y2 + w1*x2;
	y = -x1*z2 + y1*w2 + z1*x2 + w1*y2;
	z = x1*y2 - y1*x2 + z1*w2 + w1*z2;
	w = -x1*x2 - y1*y2 - z1*z2 + w1*w2;

	quaternion<T> q(w, x, y, z);

	return q;
}

template <typename T>
quaternion<T> operator +(quaternion<T> q1, quaternion<T> q2)
{
	return (q1.m_quat + q2.m_quat);
}

template <typename T>
quaternion<T> quat_normalize(quaternion<T> q)
{
	T w, x, y, z;
	T val;

	w = q.m_quat.m_vec[0] * q.m_quat.m_vec[0];
	x = q.m_quat.m_vec[1] * q.m_quat.m_vec[1];
	y = q.m_quat.m_vec[2] * q.m_quat.m_vec[2];
	z = q.m_quat.m_vec[3] * q.m_quat.m_vec[3];

	val = sqrt(w + x + y + z);

	quaternion<T> q1(q.m_quat / val);

	return (q1);
}

template <typename T>
quaternion<T> quat_conj(quaternion<T> q)
{
	T w, x, y, z;

	w = q.m_quat.m_vec[0];
	x = q.m_quat.m_vec[1];
	y = q.m_quat.m_vec[2];
	z = q.m_quat.m_vec[3];

	quaternion<T> q1(w, -x, -y, -z);

	return q1;
}

#endif  //_QUATERNION_H
