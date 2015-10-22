/*
 * pressure_sensor_hal
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

#ifndef _PRESSURE_SENSOR_HAL_H_
#define _PRESSURE_SENSOR_HAL_H_

#include <sensor_hal.h>

class pressure_sensor_hal : public sensor_hal
{
public:
	pressure_sensor_hal();
	virtual ~pressure_sensor_hal();
	std::string get_model_id(void);
	sensor_hal_type_t get_type(void);

	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	virtual bool get_properties(sensor_properties_s &properties);
private:
	std::string m_model_id;
	std::string m_vendor;
	std::string m_chip_name;

	float m_pressure;
	float m_sea_level_pressure;
	float m_temperature;

	int m_resolution;

	float m_min_range;
	float m_max_range;
	float m_raw_data_unit;
	float m_temp_resolution;
	float m_temp_offset;
	unsigned long m_polling_interval;

	unsigned long long m_fired_time;
	int m_node_handle;

	std::string m_enable_node;
	std::string m_data_node;
	std::string m_interval_node;

	bool m_sensorhub_controlled;

	cmutex m_value_mutex;

	bool update_value(bool wait);
};
#endif /*_PRESSURE_SENSOR_HAL_CLASS_H_*/
