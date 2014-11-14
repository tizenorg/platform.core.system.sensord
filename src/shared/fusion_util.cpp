/*
 * libsensord-share
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

#include <fusion_util.h>
#include <math.h>
#include <stdlib.h>

int quat_to_matrix(const float *quat, float *R)
{
	if(quat == NULL || R == NULL)
		return -1;

	float q0 = quat[3];
	float q1 = quat[0];
	float q2 = quat[1];
	float q3 = quat[2];

	float sq_q1 = 2 * q1 * q1;
	float sq_q2 = 2 * q2 * q2;
	float sq_q3 = 2 * q3 * q3;
	float q1_q2 = 2 * q1 * q2;
	float q3_q0 = 2 * q3 * q0;
	float q1_q3 = 2 * q1 * q3;
	float q2_q0 = 2 * q2 * q0;
	float q2_q3 = 2 * q2 * q3;
	float q1_q0 = 2 * q1 * q0;

	R[0] = 1 - sq_q2 - sq_q3;
	R[1] = q1_q2 - q3_q0;
	R[2] = q1_q3 + q2_q0;
	R[3] = q1_q2 + q3_q0;
	R[4] = 1 - sq_q1 - sq_q3;
	R[5] = q2_q3 - q1_q0;
	R[6] = q1_q3 - q2_q0;
	R[7] = q2_q3 + q1_q0;
	R[8] = 1 - sq_q1 - sq_q2;

	return 0;
}

