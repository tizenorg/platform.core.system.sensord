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

#ifndef _FACE_DOWN_ALG_H
#define _FACE_DOWN_ALG_H

#include <sensor_common.h>
#include <fusion_sensor.h>
#include <quaternion.h>

class face_down_alg {
public:
	face_down_alg();
	virtual ~face_down_alg();
	virtual void push_event(const sensor_event_t& event);
	virtual bool get_face_down(void);
private:
	fusion_sensor m_sensor_fusion;
	unsigned long long m_time;
	int m_current_number_of_quat;
	bool m_state;
	quaternion<float> m_old_quat[50];
	void shift_quaternions(void);
	int is_facing_down();
	int was_facing_up();

};

#endif /* _FACE_DOWN_ALG_H */
