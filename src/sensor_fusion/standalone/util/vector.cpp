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

vector::vector(void)
{

}

vector::vector(const int cols)
{

}

vector::vector(const int cols, float **vec_data)
{

}

vector::vector(const vector& v)
{

}

vector::~vector()
{

}

vector vector::operator =(const vector& v)
{

}


matrix vector::transpose(void)
{

}

vector operator +(const vector v1, const vector v2)
{

}

vector operator +(const vector v, const float val)
{

}

vector operator -(const vector v1, const vector v2)
{

}

vector operator -(const vector v, const float val)
{

}

matrix operator *(const matrix m, const vector v)
{

}

float operator *(const vector v, const matrix m)
{

}

vector operator *(const vector v, const float val)
{

}

vector operator /(const vector v1, const vector v2)
{

}

bool operator ==(const vector v1, const vector v2)
{

}

bool operator !=(const vector v1, const vector v2)
{

}
