/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   sensor_fusion1.h
 * Author: akhil
 *
 * Created on July 20, 2016, 4:52 PM
 */

#ifndef SENSOR_FUSION1_H
#define SENSOR_FUSION1_H

#include <virtual_sensor.h>
#include "orientation_filter.h"

typedef enum fusion_type {
    FUSION_TYPE_ACELL_GYRO_MAG,
    FUSION_TYPE_ACELL_GYRO,
    FUSION_TYPE_ACELL_MAG,
} fusion_type;

class sensor_fusion1 {
public:
	sensor_fusion1();
	sensor_fusion1(fusion_type FUSION_TYPE);
	virtual ~sensor_fusion1();

	virtual bool push_accel(sensor_data_t &data);
	virtual bool push_gyro(sensor_data_t &data);
	virtual bool push_mag(sensor_data_t &data);
	virtual bool get_rv(unsigned long long timestamp, float &x, float &y, float &z, float &w);

	//virtual bool get_rv(unsigned long long timestamp, float &x, float &y, float &z, float &w);
	//virtual bool get_gyro_rv(unsigned long long timestamp, float &x, float &y, float &z, float &w);
	//virtual bool get_mag_rv(unsigned long long timestamp, float &x, float &y, float &z, float &w);

private:

	sensor_data<float> m_accel;
	sensor_data<float> m_gyro;
	sensor_data<float> m_magnetic;

	sensor_data<float> *m_accel_ptr;
	sensor_data<float> *m_gyro_ptr;
	sensor_data<float> *m_magnetic_ptr;

	orientation_filter<float> m_orientation_filter;

	bool m_enable_accel;
	bool m_enable_gyro;
	bool m_enable_magnetic;

	fusion_type M_FUSION_TYPE;
	float m_x;
	float m_y;
	float m_z;
	float m_w;
	float m_timestamp;

	float m_accel_static_bias[3];
	float m_gyro_static_bias[3];
	float m_geomagnetic_static_bias[3];
	int m_accel_rotation_direction_compensation[3];
	int m_gyro_rotation_direction_compensation[3];
	int m_geomagnetic_rotation_direction_compensation[3];
	int m_magnetic_alignment_factor;

	virtual void clear_values();
	virtual void get_orientation();
};



#endif /* SENSOR_FUSION1_H */
