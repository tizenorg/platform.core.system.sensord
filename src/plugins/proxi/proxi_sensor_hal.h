/*
 * proxi_sensor_hal
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

#ifndef _PROXI_SENSOR_HAL_H_
#define _PROXI_SENSOR_HAL_H_

#include <sensor_hal.h>

class proxi_sensor_hal : public sensor_hal
{
public:
	enum proxi_node_state_event_t {	//changed as per Input Event Method definitions
		PROXIMITY_NODE_STATE_NEAR = 0,
		PROXIMITY_NODE_STATE_FAR = 1,
		PROXIMITY_NODE_STATE_UNKNOWN = -1,
	};

// In case of IIO input method, use the following definitions as the values returned by sensor are different.
//	enum proxi_node_state_event_t {	//changed as per IIO Method definitions
//		PROXIMITY_NODE_STATE_NEAR = 1,
//		PROXIMITY_NODE_STATE_FAR = 2,
//		PROXIMITY_NODE_STATE_UNKNOWN = 0,
//	};

	proxi_sensor_hal();
	virtual ~proxi_sensor_hal();
	std::string get_model_id(void);
	sensor_hal_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	virtual bool get_properties(sensor_properties_s &properties);
private:
	std::string m_model_id;
	std::string m_vendor;
	std::string m_chip_name;

	std::string m_enable_node;
	std::string m_data_node;

	unsigned int m_state;

	unsigned long long m_fired_time;

	int m_node_handle;
	bool m_sensorhub_controlled;
	cmutex m_value_mutex;

	bool update_value(bool wait);
};
#endif /*_PROXI_SENSOR_HAL_H_*/
