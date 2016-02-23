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

#include <sensor_common.h>
#include <sensor_logs.h>
#include <sensor_types.h>
#include <gesture_movement_alg.h>
#include <stdlib.h>
#include <math.h>

#ifndef _MYDEBUG_
#define _MYDEBUG_
#endif

#ifdef _MYDEBUG_
#define _MYE _E
#else
#define _MYE(...) do{} while(0)
#endif

gesture_movement_alg::gesture_movement_alg()
: m_kmc(NULL)
, m_tmm_dtw(NULL)
, m_accel_array_length(0)
, m_is_template_matched(false)
{
	m_kmc = new(std::nothrow) k_means_clustering();

	m_kmc->get_template_base(m_template_base, m_template_distance_table);

	for (int i = 0; i < TEMPLATE_NCOUNT; i++) {
		_MYE("@@@@@ m_template_base[%2d] is {%2d}", i, m_template_base[i]);
	}

	for (int k1 = 0; k1 < TEMPLATE_KCOUNT; k1++) {
		for (int k2 = 0; k2 < TEMPLATE_KCOUNT; k2++) {
			_MYE("@@@@@ m_template_distance_table[%2d][%2d] is {%6.6f}",
				k1, k2, m_template_distance_table[k1][k2]);
		}
	}

	m_tmm_dtw = new(std::nothrow) tmm_dtw();
}

gesture_movement_alg::~gesture_movement_alg()
{
	delete m_kmc;
	delete m_tmm_dtw;
}

bool gesture_movement_alg::get_gesture_movement(float acc[3], unsigned long long timestamp, int prev_gesture_movement, int &current_gesture_movement)
{
	int similarity = 0;

	if (m_accel_array_length < TEMPLATE_NCOUNT) { // not full
		m_accel_array[m_accel_array_length].x = acc[0];
		m_accel_array[m_accel_array_length].y = acc[1];
		m_accel_array[m_accel_array_length].z = acc[2];

		m_accel_array_length++;

		current_gesture_movement = GESTURE_MOVEMENT_NONE;
	} else { // full
		int i = 0;

		for (i = 0; i < (m_accel_array_length - 1); i++) {
			m_accel_array[i].x = m_accel_array[i+1].x;
			m_accel_array[i].y = m_accel_array[i+1].y;
			m_accel_array[i].z = m_accel_array[i+1].z;
		}

		m_accel_array[i].x = acc[0];
		m_accel_array[i].y = acc[1];
		m_accel_array[i].z = acc[2];

		// 1. full accel array -> m_kmc, get template_compare ( k_means_clustering )
		m_kmc->get_template_compare(m_template_compare, m_accel_array);

		for (i = 0; i < TEMPLATE_NCOUNT; i++) {
			_MYE("@@@@@ m_template_compare[%2d] is {%2d}", i, m_template_compare[i]);
		}

		// 2. use some TMM algorithm between template_base and template_compare ( template matching )
		similarity = m_tmm_dtw->template_matching(m_template_base, m_template_compare, m_template_distance_table);
		_MYE("@@@@@ similarity(%d)", similarity);





		if (similarity < SIMILAR_THRESHOLD) // @@@@@@@@@@ TEMPORAL DECISION @@@@@@@@@@
			m_is_template_matched = true;
		else
			m_is_template_matched = false;




		if (m_is_template_matched) { // m_kmc algorithm is true, then change event state
			current_gesture_movement = GESTURE_MOVEMENT_DETECTION;
		} else {
			current_gesture_movement = GESTURE_MOVEMENT_NONE;
		}
	}

	if (prev_gesture_movement != current_gesture_movement)
		return true;

	return false; // prev == current, then return false
}

