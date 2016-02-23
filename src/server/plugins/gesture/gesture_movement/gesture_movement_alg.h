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

#ifndef _GESTURE_MOVEMENT_ALG_H_
#define _GESTURE_MOVEMENT_ALG_H_

#include <k_means_clustering.h>
#include <tmm_dtw.h>

#define TEMPLATE_NCOUNT 10 // template length 10
#define TEMPLATE_KCOUNT 10 // template element range is 0 ~ 9 ( 0,1,2,3,4,5,6,7,8,9 )

#define SIMILAR_THRESHOLD 70 // TEMPORAL THRESHOLD to decide whether same gesture or not

class gesture_movement_alg {
public:
	gesture_movement_alg();
	virtual ~gesture_movement_alg();

	bool get_gesture_movement(float acc[3], unsigned long long timestamp, int prev_gesture_movement, int &gesture_movement);

private:
	k_means_clustering *m_kmc;
	tmm_dtw            *m_tmm_dtw;

	accel_t m_accel_array[TEMPLATE_NCOUNT];
	int     m_accel_array_length;

	int   m_template_base[TEMPLATE_NCOUNT];
	int   m_template_compare[TEMPLATE_NCOUNT];
	float m_template_distance_table[TEMPLATE_KCOUNT][TEMPLATE_KCOUNT];

	bool m_is_template_matched;
};
#endif /* _GESTURE_MOVEMENT_ALG_H_ */
