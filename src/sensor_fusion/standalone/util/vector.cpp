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

#include <vector.h>

template <typename TYPE>
vector<TYPE>::vector(void)
{

}

template <typename TYPE>
vector<TYPE>::vector(const int cols)
{

}

template <typename TYPE>
vector<TYPE>::vector(const int cols, TYPE *vec_data)
{

}

template <typename TYPE>
vector<TYPE>::vector(const vector<TYPE>& v)
{

}

template <typename TYPE>
vector<TYPE>::~vector()
{

}

template <typename TYPE>
vector<TYPE> vector<TYPE>::operator =(const vector<TYPE>& v)
{

}

template <typename TYPE>
vector<TYPE> operator +(const vector<TYPE> v1, const vector<TYPE> v2)
{

}

template <typename TYPE>
vector<TYPE> operator +(const vector<TYPE> v, const TYPE val)
{

}

template <typename TYPE>
vector<TYPE> operator -(const vector<TYPE> v1, const vector<TYPE> v2)
{

}

template <typename TYPE>
vector<TYPE> operator -(const vector<TYPE> v, const TYPE val)
{

}

template <typename TYPE>
matrix<TYPE> operator *(const matrix<TYPE> m, const vector<TYPE> v)
{

}

template <typename TYPE>
TYPE operator *(const vector<TYPE> v, const matrix<TYPE> m)
{

}

template <typename TYPE>
vector<TYPE> operator *(const vector<TYPE> v, const TYPE val)
{

}

template <typename TYPE>
vector<TYPE> operator /(const vector<TYPE> v1, const vector<TYPE> v2)
{

}

template <typename TYPE>
bool operator ==(const vector<TYPE> v1, const vector<TYPE> v2)
{

}

template <typename TYPE>
bool operator !=(const vector<TYPE> v1, const vector<TYPE> v2)
{

}

template <typename TYPE>
matrix<TYPE> vector<TYPE>::transpose(void)
{

}
