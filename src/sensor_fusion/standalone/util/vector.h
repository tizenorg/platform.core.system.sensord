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

#include<stdlib.h>
#include<matrix.h>

class vector {
public:
	int cols;
	float **vec;
private:
	vector(const int cols);
	vector(const vector& v);
	~vector();

	vector operator =(const vector& v);
	vector operator +(const vector v);
	vector operator +(const float val);
	vector operator -(const vector v);
	vector operator -(const float val);
	matrix operator *(const vector v);
	matrix operator *(const matrix m);
	vector operator *(const float val);
	vector operator /(const vector v);
	bool operator ==(const vector v);
	bool operator !=(const vector v);

	matrix conjugate(void);
};

#endif  //_VECTOR_H
