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
#include <face_down_alg.h>
#include <euler_angles.h>
#define SENSOR_INV_FREQUENCY (20*1000)
#define TWENTY_DEGREES 0.349066
#define ONE_SIXTY_DEGREES 2.79253

using namespace std;

face_down_alg::face_down_alg() {
	m_sensor_fusion = fusion_sensor (FUSION_TYPE_ACCEL_GYRO_MAG);
	m_time = 0;
	m_state = false;
	m_current_number_of_quat = -1;
}

face_down_alg::~face_down_alg() {
}

void face_down_alg::push_event(const sensor_event_t& event){

	if (event.event_type == ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_sensor_fusion.push_accel(*(event.data));
	} else if (event.event_type == GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME) {
		m_sensor_fusion.push_gyro(*(event.data));
	} else {
		m_sensor_fusion.push_mag(*(event.data));
	}

	//_I("face_down_alg: %llu acc[0]: %f, acc[1]: %f, acc[2]: %f",event.data->timestamp,event.data->values[0],event.data->values[1],event.data->values[2]);
	if ((event.data->timestamp-m_time)>SENSOR_INV_FREQUENCY){
		unsigned long long timestamp;
		float x, y, z, w;
		m_sensor_fusion.get_rv(timestamp, x, y, z, w);
		//_I("face_down_alg: %llu quat[0]: %f, quat[1]: %f, quat[2]: %f, quat[3]: %f",timestamp, x,y,z,w);
		quaternion<float> new_quat(x,y,z,w);
		euler_angles<float> my_ang = quat2euler(new_quat);
		_I("face_down_alg: ang[0]: %f, ang[1]: %f, ang[2]: %f",my_ang.m_ang.m_vec[0],my_ang.m_ang.m_vec[1],my_ang.m_ang.m_vec[2]);
		shift_quaternions();
		m_old_quat[m_current_number_of_quat] = new_quat;
		m_time = event.data->timestamp;
	}
}

void face_down_alg::shift_quaternions( void){
	if(m_current_number_of_quat<49){
		m_current_number_of_quat++;
	} else {
		for(int i = 0; i<49; i++){
			quaternion<float> temp_quat = m_old_quat[i+1];
			m_old_quat[i] = temp_quat;
		}
	}
}

bool face_down_alg::get_face_down(void){
	return (is_facing_down() > was_facing_up());
}

int face_down_alg::is_facing_down(void){
	for(int i = m_current_number_of_quat; i>=0;i--){
		euler_angles<float> my_ang = quat2euler(m_old_quat[i]);
		if(abs(my_ang.m_ang.m_vec[0])<TWENTY_DEGREES && abs(my_ang.m_ang.m_vec[1])<TWENTY_DEGREES)
			return i;
	}
	return -1;
}

int face_down_alg::was_facing_up(void){
	for(int i = 0; i<=m_current_number_of_quat;i++){
		euler_angles<float> my_ang = quat2euler(m_old_quat[i]);
		if(abs(my_ang.m_ang.m_vec[0])>ONE_SIXTY_DEGREES && abs(my_ang.m_ang.m_vec[1])<TWENTY_DEGREES)
			return i;
	}
	return m_current_number_of_quat+1;
}
