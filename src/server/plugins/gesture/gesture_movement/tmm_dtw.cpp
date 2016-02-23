/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <sensor_logs.h> // for dlog

#include <cmath>
#include <tmm_dtw.h>

#ifndef _MYDEBUG_
#define _MYDEBUG_
#endif

#ifdef _MYDEBUG_
#define _MYE _E
#else
#define _MYE(...) do{} while(0)
#endif

tmm_dtw::tmm_dtw()
{
}

tmm_dtw::~tmm_dtw()
{
}

int tmm_dtw::template_matching(int *S, int *T, float table[][K_COUNT])
{
	int s = 0, t = 0;

	for (int s = 1; s <= BASE_COUNT; s++)
		m_dtw[s][0] = SIMILAR_INFINITY;

	for (int t = 1; t <= COMPARE_COUNT; t++)
		m_dtw[0][t] = SIMILAR_INFINITY;

	m_dtw[0][0] = 0;

	for (int s = 1; s <= BASE_COUNT; s++) {
		for (int t = 1; t <= COMPARE_COUNT; t++) { // @@@@@@@@@@ distance float -> int @@@@@@@@@@
			m_dtw[s][t] = (int)(table[S[s-1]][T[t-1]]) + min3(m_dtw[s-1][t], m_dtw[s][t-1], m_dtw[s-1][t-1]);
		}
	}

	return m_dtw[BASE_COUNT][COMPARE_COUNT];
}

int tmm_dtw::min3(int a, int b, int c)
{
	if (a > b) {
		if (b > c)
			return c;
		else
			return b;
	} else {
		if (a > c)
			return c;
		else
			return a;
	}
}

