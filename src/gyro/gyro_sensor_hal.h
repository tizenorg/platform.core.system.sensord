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

#ifndef _GYRO_SENSOR_HAL_H_
#define _GYRO_SENSOR_HAL_H_

#include <sensor_hal.h>
#include <string>
#include <iio_common.h>

#define INPUT_DEV_NAME	"lsm330dlc-gyro"
#define INPUT_TRIG_NAME	"lsm330dlc-gyro-trigger"

#define IIO_DIR 			"/sys/bus/iio/devices/"
#define GYRO_FREQ 			"sampling_frequency"
#define GYRO_FREQ_AVLBL		"sampling_frequency_available"
#define GYRO_SCALE_AVLBL	"in_anglvel_scale_available"
#define GYRO_X_SCALE		"in_anglvel_x_scale"
#define GYRO_Y_SCALE		"in_anglvel_y_scale"
#define GYRO_Z_SCALE		"in_anglvel_z_scale"

#define NO_OF_CHANNELS		4
#define MAX_FREQ_COUNT		16
#define MAX_SCALING_COUNT	16

#define CHANNEL_NAME_X		"in_anglvel_x"
#define CHANNEL_NAME_Y		"in_anglvel_y"
#define CHANNEL_NAME_Z		"in_anglvel_z"
#define CHANNEL_NAME_TIME	"in_timestamp"
#define ENABLE_SUFFIX		"_en"
#define NAME_NODE			"/name"
#define BUFFER_EN			"buffer/enable"
#define BUFFER_LEN			"buffer/length"
#define SCAN_EL_DIR			"scan_elements/"

#define IIO_DEV_BASE_NAME	"iio:device"
#define IIO_TRIG_BASE_NAME	"trigger"
#define IIO_DEV_STR_LEN		10
#define IIO_TRIG_STR_LEN	7

#define GYRO_RINGBUF_LEN	32

using std::string;

class gyro_sensor_hal : public sensor_hal
{
public:
	gyro_sensor_hal();
	virtual ~gyro_sensor_hal();
	string get_model_id(void);
	sensor_type_t get_type(void);
	bool enable(void);
	bool disable(void);
	bool set_interval(unsigned long val);
	bool is_data_ready(bool wait);
	virtual int get_sensor_data(sensor_data_t &data);
	bool get_properties(sensor_properties_t &properties);
	bool check_hw_node(void);

private:
	int m_x;
	int m_y;
	int m_z;
	unsigned long m_polling_interval;
	unsigned long long m_fired_time;
	bool m_sensorhub_supported;

	string m_model_id;
	string m_name;
	string m_vendor;
	string m_chip_name;

	int m_resolution;
	float m_raw_data_unit;

	string m_polling_resource;

	string m_gyro_dir;
	string m_gyro_trig_dir;
	string m_buffer_access;
	string m_freq_resource;

	int m_scale_factor_count;
	int m_sample_freq_count;
	int m_sample_freq[MAX_FREQ_COUNT];
	double m_scale_factor[MAX_SCALING_COUNT];

	int m_fp_buffer;
	char *m_data;
	int m_scan_size;
	struct channel_parameters *m_channels;

	cmutex m_value_mutex;

	bool enable_resource(bool enable);
	bool update_value(bool wait);
	bool is_sensorhub_supported(void);

	bool add_gyro_channels_to_array(void);
	bool setup_channels(void);
	bool setup_buffer(int enable);
	bool setup_trigger(char* trig_name, bool verify);
	void decode_data(void);
};
#endif /*_GYRO_SENSOR_HAL_H_*/
