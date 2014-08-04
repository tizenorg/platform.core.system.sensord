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

#ifndef _PROXI_SENSOR_HAL_H_
#define _PROXI_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>

#define IIO_DIR			"/sys/bus/iio/devices/"
#define NAME_NODE		"/name"
#define EVENT_DIR		"/events"
#define EVENT_EN_NODE	"/in_proximity_thresh_either_en"
#define DEV_DIR			"/dev/"

#define IIO_DEV_BASE_NAME	"iio:device"
#define IIO_DEV_STR_LEN		10

#define PROXIMITY_NODE_STATE_NEAR	1
#define PROXIMITY_NODE_STATE_FAR	2
#define PROXIMITY_TYPE				8

using std::string;

class proxi_sensor_hal : public sensor_hal
{
public:
	proxi_sensor_hal();
	virtual ~proxi_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);
	bool check_hw_node(void);

private:
	unsigned int m_state;
	int m_node_handle;
	unsigned long long m_fired_time;
	bool m_sensorhub_supported;

	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	string m_proxi_dir;

	string m_enable_resource;
	string m_event_resource;

	cmutex m_value_mutex;

	int m_event_fd;

	bool enable_resource(int enable);
	bool update_value(bool wait);
	bool is_sensorhub_supported(void);
};
#endif /*_PROXI_SENSOR_HAL_H_*/
