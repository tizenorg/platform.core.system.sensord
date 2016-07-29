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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sensor_log.h>
#include <sensor_loader.h>
#include <sensor_base.h>
#include <cmath>
#include "fusion_sensor.h"
#include "orientation_filter.h"

using std::string;
using std::vector;

fusion_sensor::fusion_sensor(fusion_type FUSION_TYPE) {
	init(FUSION_TYPE);
	_I("fusion_sensor is created!");
}

fusion_sensor::fusion_sensor() {
	init(FUSION_TYPE_ACCEL_GYRO_MAG);
	_I("fusion_sensor is created!");
}

fusion_sensor::~fusion_sensor() {
	_I("fusion_sensor is destroyed!");
}

void fusion_sensor::init(fusion_type FUSION_TYPE) {
	m_accel_rotation_direction_compensation[0] = -1;
	m_accel_rotation_direction_compensation[1] = -1;
	m_accel_rotation_direction_compensation[2] = -1;
	m_gyro_rotation_direction_compensation[0] = 1;
	m_gyro_rotation_direction_compensation[1] = 1;
	m_gyro_rotation_direction_compensation[2] = 1;
	m_geomagnetic_rotation_direction_compensation[0] = -1;
	m_geomagnetic_rotation_direction_compensation[1] = -1;
	m_geomagnetic_rotation_direction_compensation[2] = -1;
	m_magnetic_alignment_factor = 1;
	m_accel_ptr = m_gyro_ptr = m_magnetic_ptr = NULL;
	m_enable_accel = false;
	m_enable_gyro = false;
	m_enable_magnetic = false;
	m_orientation_filter.m_magnetic_alignment_factor = m_magnetic_alignment_factor;
	M_FUSION_TYPE = FUSION_TYPE;
}

void fusion_sensor::clear(void) {
	m_enable_accel = false;
	m_enable_gyro = false;
	m_enable_magnetic = false;
	m_accel_ptr = m_gyro_ptr = m_magnetic_ptr = NULL;
}

void fusion_sensor::get_orientation(void) {
	//_I("[fusion_sensor] : enable values are %d %d %d", m_enable_accel, m_enable_gyro, m_enable_magnetic);
	if (M_FUSION_TYPE == FUSION_TYPE_ACCEL_GYRO_MAG) {
		if (!m_enable_accel || !m_enable_gyro || !m_enable_magnetic)
			return;
		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, &m_magnetic);
		m_timestamp = fmax(m_accel.m_time_stamp, m_gyro.m_time_stamp);
		m_timestamp = fmax(m_timestamp, m_magnetic.m_time_stamp);
	} else if (M_FUSION_TYPE == FUSION_TYPE_ACCEL_GYRO) {
		if (!m_enable_accel || !m_enable_gyro)
			return;
		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, NULL);
		m_timestamp = fmax(m_accel.m_time_stamp, m_gyro.m_time_stamp);
	} else if (M_FUSION_TYPE == FUSION_TYPE_ACCEL_MAG) {
		if (!m_enable_accel || !m_enable_magnetic)
			return;
		m_orientation_filter.get_device_orientation(&m_accel, NULL, &m_magnetic);
		m_timestamp = fmax(m_accel.m_time_stamp, m_magnetic.m_time_stamp);
	}
	m_x = m_orientation_filter.m_quaternion.m_quat.m_vec[0];
	m_y = m_orientation_filter.m_quaternion.m_quat.m_vec[1];
	m_z = m_orientation_filter.m_quaternion.m_quat.m_vec[2];
	m_w = m_orientation_filter.m_quaternion.m_quat.m_vec[3];
	//_I("[fusion_sensor] : values are [%10f] [%10f] [%10f] [%10f]", m_x, m_y, m_z, m_w);
	clear();
}

bool fusion_sensor::push_accel(sensor_data_t &data) {
	//_I("[fusion_sensor] : Pushing accel");
	pre_process_data(m_accel, data.values, m_accel_static_bias, m_accel_rotation_direction_compensation, ACCEL_SCALE);
	m_accel.m_time_stamp = data.timestamp;
	m_accel_ptr = &m_accel;
	m_enable_accel = true;
	get_orientation();
	return true;
}

bool fusion_sensor::push_gyro(sensor_data_t &data) {
	//_I("[fusion_sensor] : Pushing mag");
	pre_process_data(m_gyro, data.values, m_gyro_static_bias, m_gyro_rotation_direction_compensation, GYRO_SCALE);
	m_gyro.m_time_stamp = data.timestamp;
	m_gyro_ptr = &m_gyro;
	m_enable_gyro = true;
	get_orientation();
	return true;
}

bool fusion_sensor::push_mag(sensor_data_t &data) {
	//_I("[fusion_sensor] : Pushing gyro");
	pre_process_data(m_magnetic, data.values, m_geomagnetic_static_bias, m_geomagnetic_rotation_direction_compensation, GEOMAGNETIC_SCALE);
	m_magnetic.m_time_stamp = data.timestamp;
	m_magnetic_ptr = &m_magnetic;
	m_enable_magnetic = true;
	get_orientation();
	return true;

}

bool fusion_sensor::get_rv(unsigned long long timestamp, float &x, float &y, float &z, float &w) {
	if (m_timestamp == 0)
		return false;
	timestamp = m_timestamp;
	x = m_x;
	y = m_y;
	z = m_z;
	w = m_w;
	return true;
}
