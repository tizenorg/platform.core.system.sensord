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

#ifndef _GYRO_SENSOR_H_
#define _GYRO_SENSOR_H_

#include <sensor_common.h>

#include <physical_sensor.h>
#include <sensor_hal.h>

class gyro_sensor : public physical_sensor {
public:
	gyro_sensor();
	virtual ~gyro_sensor();

	virtual bool init();
	virtual sensor_type_t get_type(void);

	static bool working(void *inst);

	virtual bool on_start(void);
	virtual bool on_stop(void);

	virtual bool set_interval(unsigned long interval);
	virtual bool get_properties(const unsigned int type, sensor_properties_t &properties);
	int get_sensor_data(const unsigned int type, sensor_data_t &data);
private:
	sensor_hal *m_sensor_hal;
	float m_resolution;

	void raw_to_base(sensor_data_t &data);
	bool process_event(void);
};

#endif
