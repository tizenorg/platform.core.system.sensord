/*
 * accel_sensor_hal
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

#ifndef _ACCEL_SENSOR_HAL_H_
#define _ACCEL_SENSOR_HAL_H_

#define MAX_FREQ_COUNT		16
#define MAX_SCALING_COUNT	16

#include <sensor_hal.h>
#include <string>
#include <iio_common.h>

using std::string;

class accel_sensor_hal : public sensor_hal
{
public:
	accel_sensor_hal();
	virtual ~accel_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);
//	bool check_hw_node(void);

private:
	int m_x;
	int m_y;
	int m_z;
	int m_node_handle;
	unsigned long m_polling_interval;
	unsigned long long m_fired_time;

	int m_scale_factor_count;
	int m_sample_freq_count;
	int m_sample_freq[MAX_FREQ_COUNT];
	double m_scale_factor[MAX_SCALING_COUNT];
	char *m_data;
	int m_scan_size;
	struct channel_parameters *m_channels;

	string m_trigger_name;
	string m_trigger_path;
	string m_buffer_enable_node_path;
	string m_buffer_length_node_path;
	string m_available_freq_node_path;
	string m_available_scale_node_path;
	string m_accel_dir;
	vector<string> m_generic_channel_names;

	string m_model_id;
	string m_vendor;
	string m_chip_name;

	int m_resolution;
	float m_raw_data_unit;

	string m_data_node;
	string m_interval_node;

	bool m_sensorhub_controlled;

	cmutex m_value_mutex;

	bool update_value(bool wait);
	bool calibration(int cmd);

	bool setup_trigger(const char* trig_name, bool verify);
	bool setup_buffer(int enable);
	bool enable_resource(bool enable);
	bool add_accel_channels_to_array(void);
	bool setup_channels(void);
	bool setup_trigger(char* trig_name, bool verify);
	void decode_data(void);
};
#endif /*_ACCEL_SENSOR_HAL_CLASS_H_*/
