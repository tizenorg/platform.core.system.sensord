/*
 * sensord
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

#ifndef _GRAVITY_SENSOR_H_
#define _GRAVITY_SENSOR_H_

#include <virtual_sensor.h>
#include <sensor_types.h>
#include <sensor_fusion.h>

class gravity_sensor : public virtual_sensor {
public:
	gravity_sensor();
	virtual ~gravity_sensor();

	/* initialize sensor */
	bool init(void);

	/* sensor info */
	virtual sensor_type_t get_type(void);
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);

	virtual bool get_sensor_info(sensor_info &info);

	/* synthesize event */
	virtual void synthesize(const sensor_event_t& event);

	bool add_interval(int client_id, unsigned int interval, bool is_processor);
	bool delete_interval(int client_id, bool is_processor);
	/* get data */
	virtual int get_data(sensor_data_t **data, int *length);
private:
	sensor_fusion *m_fusion;
	sensor_base *m_accel_sensor;
	sensor_base *m_gyro_sensor;

	int m_fusion_phase;
	float m_x;
	float m_y;
	float m_z;
	int m_accuracy;
	unsigned long long m_time;
	unsigned long m_interval;

	double m_angle[3];
	double m_angle_n[3];
	double m_accel_mag;
	double m_velocity[3];
	unsigned long long m_time_new;

	virtual bool set_interval(unsigned long interval);
	virtual bool set_batch_latency(unsigned long latency);
	virtual bool set_wakeup(int wakeup);

	virtual bool on_start(void);
	virtual bool on_stop(void);

	bool rotation_to_gravity(const float *rotation, float *gravity);
	bool check_sampling_time(unsigned long long timestamp);

	void synthesize_rv(const sensor_event_t& event);
	void synthesize_lowpass(const sensor_event_t& event);
	void synthesize_fusion(const sensor_event_t& event);

	void fusion_set_accel(const sensor_event_t& event);
	void fusion_set_gyro(const sensor_event_t& event);
	void fusion_update_angle(void);
	void fusion_get_gravity(void);
	double complementary(double angle, double angle_in, double vel, double delta_t, double alpha);
	void complementary(unsigned long long time_diff);
};

#endif /* _GRAVITY_SENSOR_H_ */
