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

#if defined (_ROTATION_MATRIX_H) && defined (_MATRIX_H)

#define QUAT_LEN 4
#define ROT_MAT_ROWS 3
#define ROT_MAT_COLS 3

template <typename T> T get_sign(T val)
{
	return (val >= (T) 0) ? (T) 1 : (T) -1;
}

template <typename TYPE>
rotation_matrix<TYPE>::rotation_matrix() : m_rot_mat(ROT_MAT_ROWS, ROT_MAT_COLS)
{

}

template <typename TYPE>
rotation_matrix<TYPE>::rotation_matrix(const matrix<TYPE> m)
{
	m_rot_mat = m;
}

template <typename TYPE>
rotation_matrix<TYPE>::rotation_matrix(const int rows, const int cols, TYPE *mat_data)
{
	matrix<TYPE> m(rows, cols, mat_data);
	m_rot_mat = m;
}

template <typename TYPE>
rotation_matrix<TYPE>::rotation_matrix(const rotation_matrix<TYPE>& rm)
{
	m_rot_mat = rm.m_rot_mat;
}

template <typename TYPE>
rotation_matrix<TYPE>::~rotation_matrix()
{

}

template <typename TYPE>
rotation_matrix<TYPE> rotation_matrix<TYPE>::operator =(const rotation_matrix<TYPE>& rm)
{
	m_rot_mat = rm.m_rot_mat;

	return *this;
}

template <typename T>
rotation_matrix<T> quat2rot_mat(quaternion<T> q)
{
	T w, x, y, z;
	T R[ROT_MAT_ROWS][ROT_MAT_COLS];

	w = q.m_quat.m_vec[0];
	x = q.m_quat.m_vec[1];
	y = q.m_quat.m_vec[2];
	z = q.m_quat.m_vec[3];

	R[0][0] = (2 * w * w) - 1 + (2 * x * x);
	R[0][1] = 2 * ((x * y) + (w * z));
	R[0][2] = 2 * ((x * z) - (w * y));
	R[1][0] = 2 * ((x * y) - (w * z));
	R[1][1] = (2 * w * w) - 1 + (2 * y * y);
	R[1][2] = 2 * ((y * z) + (w * x));
	R[2][0] = 2 * ((x * z) + (w * y));
	R[2][1] = 2 * ((y * z) - (w * x));
	R[2][2] = (2 * w * w) - 1 + (2 * z * z);

	rotation_matrix<T> rm(ROT_MAT_ROWS, ROT_MAT_COLS, &R[0][0]);

	return rm;
}

template <typename T>
quaternion<T> rot_mat2quat(rotation_matrix<T> rm)
{
	T q0, q1, q2, q3;

	q0 = (rm.m_rot_mat.m_mat[0][0] + rm.m_rot_mat.m_mat[1][1] +
			rm.m_rot_mat.m_mat[2][2] + (T) 1) / (T) QUAT_LEN;
	q1 = (rm.m_rot_mat.m_mat[0][0] - rm.m_rot_mat.m_mat[1][1] -
			rm.m_rot_mat.m_mat[2][2] + (T) 1) / (T) QUAT_LEN;
	q2 = (-rm.m_rot_mat.m_mat[0][0] + rm.m_rot_mat.m_mat[1][1] -
			rm.m_rot_mat.m_mat[2][2] + (T) 1) / (T) QUAT_LEN;
	q3 = (-rm.m_rot_mat.m_mat[0][0] - rm.m_rot_mat.m_mat[1][1] +
			rm.m_rot_mat.m_mat[2][2] + (T) 1) / (T) QUAT_LEN;

	if(q0 < (T) 0)
		q0 = (T) 0;
	if(q1 < (T) 0)
		q1 = (T) 0;
	if(q2 < (T) 0)
		q2 = (T) 0;
	if(q3 < (T) 0)
		q3 = (T) 0;

	q0 = sqrt(q0);
	q1 = sqrt(q1);
	q2 = sqrt(q2);
	q3 = sqrt(q3);

	if (q0 >= q1 && q0 >= q2 && q0 >= q3)
	{
		q0 *= (T) 1;
		q1 *= get_sign(rm.m_rot_mat.m_mat[2][1] - rm.m_rot_mat.m_mat[1][2]);
		q2 *= get_sign(rm.m_rot_mat.m_mat[0][2] - rm.m_rot_mat.m_mat[2][0]);
		q3 *= get_sign(rm.m_rot_mat.m_mat[1][0] - rm.m_rot_mat.m_mat[0][1]);
	}
	else if (q1 >= q0 && q1 >= q2 && q1 >= q3)
	{
		q0 *= get_sign(rm.m_rot_mat.m_mat[2][1] - rm.m_rot_mat.m_mat[1][2]);
		q1 *= (T) 1;
		q2 *= get_sign(rm.m_rot_mat.m_mat[1][0] + rm.m_rot_mat.m_mat[0][1]);
		q3 *= get_sign(rm.m_rot_mat.m_mat[0][2] + rm.m_rot_mat.m_mat[2][0]);
	}
	else if (q2 >= q0 && q2 >= q1 && q2 >= q3)
	{
		q0 *= get_sign(rm.m_rot_mat.m_mat[0][2] - rm.m_rot_mat.m_mat[2][0]);
		q1 *= get_sign(rm.m_rot_mat.m_mat[1][0] + rm.m_rot_mat.m_mat[0][1]);
		q2 *= (T) 1;
		q3 *= get_sign(rm.m_rot_mat.m_mat[2][1] + rm.m_rot_mat.m_mat[1][2]);
	}
	else if(q3 >= q0 && q3 >= q1 && q3 >= q2)
	{
		q0 *= get_sign(rm.m_rot_mat.m_mat[1][0] - rm.m_rot_mat.m_mat[0][1]);
		q1 *= get_sign(rm.m_rot_mat.m_mat[2][0] + rm.m_rot_mat.m_mat[0][2]);
		q2 *= get_sign(rm.m_rot_mat.m_mat[2][1] + rm.m_rot_mat.m_mat[1][2]);
		q3 *= (T) 1;
	}

	quaternion<T> q(-q0, q1, q2, q3);

	return quat_normalize(q);
}

#endif /* _ROTATION_MATRIX_H */
