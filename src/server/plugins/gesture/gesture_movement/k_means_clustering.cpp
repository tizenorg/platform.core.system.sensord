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

#include <math.h>
#include <k_means_clustering.h>

#ifndef _MYDEBUG_
#define _MYDEBUG_
#endif

#ifdef _MYDEBUG_
#define _MYE _E
#else
#define _MYE(...) do{} while(0)
#endif

k_means_clustering::k_means_clustering()
: m_completed(false)
{
	fill_data();

	decide_initial_means();

	do {
		clustering();

		decide_new_means();
	} while (!is_completed());

	print_data();

	compute_template_base();
}

k_means_clustering::~k_means_clustering()
{
}

bool k_means_clustering::get_template_base(int *template_base, float template_distance_table[][KCOUNT])
{
	if (m_completed) {
		for (int i = 0; i < KCOUNT; i++) {
			template_base[i] = m_template_base[i].ci;
		}

		for (int k1 = 0; k1 < KCOUNT; k1++) {
			for (int k2 = 0; k2 < KCOUNT; k2++) {
				if (k1 == k2) {
					template_distance_table[k1][k2] = 0.0f;
				} else {
					float diff_x_pow = pow((m_means[k1].accel.x - m_means[k2].accel.x), 2.0f);
					float diff_y_pow = pow((m_means[k1].accel.y - m_means[k2].accel.y), 2.0f);
					float diff_z_pow = pow((m_means[k1].accel.z - m_means[k2].accel.z), 2.0f);
//////////					float euclid_distance = sqrt(diff_x_pow + diff_y_pow + diff_z_pow);
					template_distance_table[k1][k2] = diff_x_pow + diff_y_pow + diff_z_pow;
				}
			}
		}
	}

	return m_completed;
}

bool k_means_clustering::get_template_compare(int *template_compare, accel_t *accel_array)
{
	int n = 0;
	mydata_t temp_data[NCOUNT];

	for (n = 0; n < NCOUNT; n++) {
		temp_data[n].accel.x = accel_array[n].x;
		temp_data[n].accel.y = accel_array[n].y;
		temp_data[n].accel.z = accel_array[n].z;

		temp_data[n].ci = -1;
		temp_data[n].ed = -1.0f;
	}

	for (int n = 0; n < NCOUNT; n++) {
		for (int k = 0; k < KCOUNT; k++) {
			float diff_x_pow = pow((temp_data[n].accel.x - m_means[k].accel.x), 2.0f);
			float diff_y_pow = pow((temp_data[n].accel.y - m_means[k].accel.y), 2.0f);
			float diff_z_pow = pow((temp_data[n].accel.z - m_means[k].accel.z), 2.0f);
//////////			float euclid_distance = sqrt(diff_x_pow + diff_y_pow + diff_z_pow);
			float euclid_distance = diff_x_pow + diff_y_pow + diff_z_pow;

			if (temp_data[n].ed < 0.0f) {
				temp_data[n].ci = k;
				temp_data[n].ed = euclid_distance;
			}

			if (temp_data[n].ed > euclid_distance) {
				temp_data[n].ci = k;
				temp_data[n].ed = euclid_distance;
			}
		}

		template_compare[n] = temp_data[n].ci;
	}

	return true;
}

void k_means_clustering::fill_data(void)
{
	int m = 0, n = 0, k = 0;

	for (m = 0; m < MCOUNT; m++) {
		for (n = 0; n < NCOUNT; n++) {
			m_data[m][n].accel.x = pre_accel[m][n].x;
			m_data[m][n].accel.y = pre_accel[m][n].y;
			m_data[m][n].accel.z = pre_accel[m][n].z;

			m_data[m][n].ci = -1;
			m_data[m][n].ed = -1.0f;
		}
	}

	for (k = 0; k < KCOUNT; k++) {
		for (n = 0; n < NCOUNT; n++) {
			m_histogram[k][n].accel.x = 0;
			m_histogram[k][n].accel.y = 0;
			m_histogram[k][n].accel.z = 0;

			m_histogram[k][n].ci = 0;
			m_histogram[k][n].ed = 0.0f;
		}
	}
}

void k_means_clustering::print_data(void)
{
	for (int m = 0; m < MCOUNT; m++) {
		for (int n = 0; n < NCOUNT; n++) {
			_MYE("@@@@@ m_data[%2d][%2d] {%6.6f, %6.6f, %6.6f, ci(%6d), ed(%6.6f)}",
				m, n, m_data[m][n].accel.x, m_data[m][n].accel.y, m_data[m][n].accel.z, m_data[m][n].ci, m_data[m][n].ed);
		}
	}
}

void k_means_clustering::decide_initial_means(void)
{
	int k = 0;

	for (int m = 0; m < MCOUNT; m++) {
		for (int n = 0; n < NCOUNT; n++) {
			if ((n % 10) == 5 ) { // @@@@@@@@@@ make m_means with [5] for test @@@@@@@@@@
				m_means[k].accel.x = m_data[m][n].accel.x;
				m_means[k].accel.y = m_data[m][n].accel.y;
				m_means[k].accel.z = m_data[m][n].accel.z;

				m_means[k].ci = m_data[m][n].ci;
				m_means[k].ed = m_data[m][n].ed;

				k++;

				break;
			}
		}

		if (k < KCOUNT)
			continue;
		else
			break;
	}

	for (k = 0; k < KCOUNT; k++) {
		_MYE("@@@@@ m_means[%2d] {%6.6f, %6.6f, %6.6f}",
				k, m_means[k].accel.x, m_means[k].accel.y, m_means[k].accel.z);
	}
}

void k_means_clustering::clustering(void)
{
	for (int m = 0; m < MCOUNT; m++) {
		for (int n = 0; n < NCOUNT; n++) {
			for (int k = 0; k < KCOUNT; k++) {
				float diff_x_pow = pow((m_data[m][n].accel.x - m_means[k].accel.x), 2.0f);
				float diff_y_pow = pow((m_data[m][n].accel.y - m_means[k].accel.y), 2.0f);
				float diff_z_pow = pow((m_data[m][n].accel.z - m_means[k].accel.z), 2.0f);
//////////				float euclid_distance = sqrt(diff_x_pow + diff_y_pow + diff_z_pow);
				float euclid_distance = diff_x_pow + diff_y_pow + diff_z_pow;

				if (m_data[m][n].ed < 0.0f) {
					m_data[m][n].ci = k;
					m_data[m][n].ed = euclid_distance;
				}

				if (m_data[m][n].ed > euclid_distance) {
					m_data[m][n].ci = k;
					m_data[m][n].ed = euclid_distance;
				}
			}
		}
	}
}

void k_means_clustering::decide_new_means(void)
{
	int k = 0;

	mydata_t prev_means[KCOUNT];

	for (k = 0; k < KCOUNT; k++) {
		prev_means[k].accel.x = m_means[k].accel.x;
		prev_means[k].accel.y = m_means[k].accel.y;
		prev_means[k].accel.z = m_means[k].accel.z;
		prev_means[k].ci = m_means[k].ci;
		prev_means[k].ed = m_means[k].ed;
	}

	for (k = 0; k < KCOUNT; k++) {
		float sum_x = 0;
		float sum_y = 0;
		float sum_z = 0;
		float cluster_count = 0.0f;

		for (int m = 0; m < MCOUNT; m++) {
			for (int n = 0; n < NCOUNT; n++) {
				if (m_data[m][n].ci == k) {
					sum_x += m_data[m][n].accel.x;
					sum_y += m_data[m][n].accel.y;
					sum_z += m_data[m][n].accel.z;
					cluster_count++;
				}
			}
		}

		if (cluster_count != 0.0f) { // @@@@@@@@@@ do not update means when there is no data around this means @@@@@@@@@@
			m_means[k].accel.x = sum_x / cluster_count;
			m_means[k].accel.y = sum_y / cluster_count;
			m_means[k].accel.z = sum_z / cluster_count;
		}

		_MYE("@@@@@ new m_means[%2d] is {%6.6f, %6.6f, %6.6f}\n",
			k, m_means[k].accel.x, m_means[k].accel.y, m_means[k].accel.z);
	}

	for (k = 0; k < KCOUNT; k++) {
		if ((prev_means[k].accel.x != m_means[k].accel.x) ||
			(prev_means[k].accel.y != m_means[k].accel.y) ||
			(prev_means[k].accel.z != m_means[k].accel.z)) {
			m_completed = false;

			return;
		}
	}

	m_completed = true;
}

bool k_means_clustering::is_completed(void)
{
	return m_completed;
}

void k_means_clustering::compute_template_base(void)
{
	int m = 0, n = 0, k = 0;
	int temp = 0;
	int max = 0;

	for (m = 0; m < MCOUNT; m++) {
		for (n = 0; n < NCOUNT; n++) {
			temp = m_data[m][n].ci;
			m_histogram[temp][n].ci++;
		}
	}

	for (n = 0; n < NCOUNT; n++) {
		for (k = 0; k < KCOUNT; k++) { // @@@@@@@@@@ compare this with previous one @@@@@@@@@@
			if (k == 0) {
				max = k;
			} else {
				if (m_histogram[k][n].ci > m_histogram[max][n].ci) { // @@@@@@@@@@ if max's ci and k's ci are same, then choose max's one @@@@@@@@@@
					max = k;
				}
			}
		}

		m_template_base[n].ci = max;

		_MYE("@@@@@ m_template_base[%2d] is {%2d}\n", n, m_template_base[n].ci);
	}
}
