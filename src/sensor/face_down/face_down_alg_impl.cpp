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
#include<cmath>
#include <sensor_log.h>
#include <face_down_alg_impl.h>
#include <euler_angles.h>
#define SENSOR_INV_FREQUENCY (20*1000)
#define TWENTY_DEGREES 0.349066
#define ONE_SIXTY_DEGREES 2.79253

face_down_alg_impl::face_down_alg_impl()
{
	m_sensor_fusion = fusion_sensor(FUSION_TYPE_ACCEL_GYRO_MAG);
	m_time = 0;
	m_state = false;
	m_latest_down_time = 0;
}

face_down_alg_impl::~face_down_alg_impl() {
}

void face_down_alg_impl::push_event(const sensor_event_t& event)
{
	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_sensor_fusion.push_accel(*(event.data));
	} else if (event.event_type == GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_sensor_fusion.push_gyro(*(event.data));
	} else if (event.event_type == GEOMAGNETIC_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_sensor_fusion.push_mag(*(event.data));
	}

	//_I("face_down_alg: %llu acc[0]: %f, acc[1]: %f, acc[2]: %f",event.data->timestamp,event.data->values[0],event.data->values[1],event.data->values[2]);
	if ((event.data->timestamp - m_time) > SENSOR_INV_FREQUENCY) {
		unsigned long long timestamp = 0;
		float x, y, z, w;
		m_sensor_fusion.get_rv(timestamp, x, y, z, w);
		m_time = event.data->timestamp;
		//_I("face_down_alg: %llu quat[0]: %f, quat[1]: %f, quat[2]: %f, quat[3]: %f",timestamp, x,y,z,w);
		quaternion<float> new_quat(x, y, z, w);
		euler_angles<float> my_ang = quat2euler(new_quat);
		//_I("face_down_alg: time: %llu ang[0]: %f, ang[1]: %f, ang[2]: %f", event.data->timestamp, my_ang.m_ang.m_vec[0], my_ang.m_ang.m_vec[1], my_ang.m_ang.m_vec[2]);

		remove_old_up_time();

		if (std::abs(my_ang.m_ang.m_vec[0]) < TWENTY_DEGREES && std::abs(my_ang.m_ang.m_vec[1]) < TWENTY_DEGREES)
			m_latest_down_time = event.data->timestamp;
		
		if (std::abs(my_ang.m_ang.m_vec[0]) > ONE_SIXTY_DEGREES && std::abs(my_ang.m_ang.m_vec[1]) < TWENTY_DEGREES)
			m_oldest_up_time.push(event.data->timestamp);
	}
}

void face_down_alg_impl::remove_old_up_time(void)
{
	while ( m_oldest_up_time.size() > 0 && ( m_time - m_oldest_up_time.front() > WINDOW_SIZE ) )
		m_oldest_up_time.pop();
}

bool face_down_alg_impl::get_face_down(void)
{
	unsigned long long down = is_facing_down();
	unsigned long long up = was_facing_up();
	//_I("face_down_alg: down: %llu, up: %llu", down, up);
	return (down > up);
}

unsigned long long face_down_alg_impl::is_facing_down(void)
{
	if ( m_time - m_latest_down_time < WINDOW_SIZE )
		return m_latest_down_time;
	return 0;
}

unsigned long long face_down_alg_impl::was_facing_up(void)
{
	remove_old_up_time();
	if (m_oldest_up_time.size() == 0)
		return m_time+1;
	return  m_oldest_up_time.front();
}
