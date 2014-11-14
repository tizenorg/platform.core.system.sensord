/*
 * gyro_sensor_hal
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
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include <linux/input.h>
#include <cconfig.h>

#include <gyro_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cconfig.h>
#include <sys/poll.h>

using std::ifstream;
using config::CConfig;

#define DPS_TO_MDPS 1000
#define MIN_RANGE(RES) (-((1 << (RES))/2))
#define MAX_RANGE(RES) (((1 << (RES))/2)-1)
#define RAW_DATA_TO_DPS_UNIT(X) ((float)(X)/((float)DPS_TO_MDPS))

#define SENSOR_TYPE_GYRO		"GYRO"
#define ELEMENT_NAME 			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION 		"RESOLUTION"

#define ATTR_VALUE 				"value"

#define SCALE_AVAILABLE_NODE	"in_anglvel_scale_available"
#define SCAN_EL_DIR				"scan_elements/"
#define TRIG_SUFFIX				"-trigger"
#define GYRO_RINGBUF_LEN		32
#define SEC_MSEC				1000
#define MSEC_TO_FREQ(VAL)		((SEC_MSEC) / (VAL))
#define NSEC_TO_MUSEC(VAL)		((VAL) / 1000)

gyro_sensor_hal::gyro_sensor_hal()
: m_x(-1)
, m_y(-1)
, m_z(-1)
, m_node_handle(-1)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(0)
{

	const string sensorhub_interval_node_name = "gyro_poll_delay";
	CConfig &config = CConfig::get_instance();

	node_path_info_query query;
	node_path_info info;
	int input_method = IIO_METHOD;

	if (!get_model_properties(SENSOR_TYPE_GYRO, m_model_id, input_method)) {
		ERR("Failed to find model_properties");
		throw ENXIO;

	}

	query.input_method = input_method;
	query.sensorhub_controlled = m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_GYRO;
	query.input_event_key = "gyro_sensor";
	query.iio_enable_node_name = "gyro_enable";
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_path_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	show_node_path_info(info);

	m_data_node = info.data_node_path;
	m_interval_node = info.interval_node_path;
	m_gyro_dir = info.base_dir;
	m_trigger_path = info.trigger_node_path;
	m_buffer_enable_node_path = info.buffer_enable_node_path;
	m_buffer_length_node_path = info.buffer_length_node_path;
	m_available_freq_node_path = info.available_freq_node_path;
	m_available_scale_node_path = m_gyro_dir + string(SCALE_AVAILABLE_NODE);

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s\n",m_chip_name.c_str());

	if (input_method == IIO_METHOD) {
		m_trigger_name = m_model_id + TRIG_SUFFIX;
		if (!verify_iio_trigger(m_trigger_name)) {
			ERR("Failed verify trigger");
			throw ENXIO;
		}
		string scan_dir = m_gyro_dir + SCAN_EL_DIR;
		if (!get_generic_channel_names(scan_dir, string("_type"), m_generic_channel_names))
			ERR ("Failed to find any input channels");
		else {
			INFO ("generic channel names:");
			for (vector <string>::iterator it = m_generic_channel_names.begin();
					it != m_generic_channel_names.end(); ++it) {
				INFO ("%s", it->c_str());
			}
		}
	}

	long resolution;

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_RESOLUTION, resolution)) {
		ERR("[RESOLUTION] is empty\n");
		throw ENXIO;
	}

	m_resolution = (int)resolution;

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);

	if ((m_node_handle = open(m_data_node.c_str(), O_RDWR)) < 0) {
		ERR("gyro handle open fail for gyro processor, error:%s\n", strerror(errno));
		throw ENXIO;
	}

	if (setup_channels() == true)
		INFO("IIO channel setup successful");
	else {
		ERR("IIO channel setup failed");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %f\n",m_raw_data_unit);
	INFO("RAW_DATA_TO_DPS_UNIT(m_raw_data_unit) = [%f]",RAW_DATA_TO_DPS_UNIT(m_raw_data_unit));
	INFO("gyro_sensor is created!\n");
}

gyro_sensor_hal::~gyro_sensor_hal()
{
	enable_resource(false);
	if (m_data != NULL)
		delete []m_data;

	close(m_node_handle);
	m_node_handle = -1;

	INFO("gyro_sensor is destroyed!\n");
}

string gyro_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t gyro_sensor_hal::get_type(void)
{
	return GYROSCOPE_SENSOR;
}

bool gyro_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	if (!enable_resource(true))
		return false;

	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Gyro sensor real starting");
	return true;
}

bool gyro_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	if (!enable_resource(false))
		return false;

	INFO("Gyro sensor real stopping");
	return true;

}

bool gyro_sensor_hal::set_interval(unsigned long ms_interval)
{
	int freq, i;

	freq = (int)(MSEC_TO_FREQ(ms_interval));

	for (i=0; i < m_sample_freq_count; i++) {
		if (freq == m_sample_freq[i]) {
			if (update_sysfs_num(m_interval_node.c_str(), freq, true) == 0) {
				INFO("Interval is changed from %lums to %lums]", m_polling_interval, ms_interval);
				m_polling_interval = ms_interval;
				return true;
			}
			else {
				ERR("Failed to set data %lu\n", ms_interval);
				return false;
			}
		}
	}

	DBG("The interval not supported: %lu\n", ms_interval);
	ERR("Failed to set data %lu\n", ms_interval);
	return false;
}

bool gyro_sensor_hal::update_value(bool wait)
{
	int i;
	struct pollfd pfd;
	ssize_t read_size;
	const int TIMEOUT = 1000;

	pfd.fd = m_node_handle;
	pfd.events = POLLIN;
	if (wait)
		poll(&pfd, 1, TIMEOUT);
	else
		poll(&pfd, 1, 0);

	read_size = read(m_node_handle, m_data, GYRO_RINGBUF_LEN * m_scan_size);
	if (read_size <= 0) {
		ERR("Gyro:No data available\n");
		return false;
	}
	else {
		for (i = 0; i < (read_size / m_scan_size); i++)
			decode_data();
	}
	return true;
}

bool gyro_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int gyro_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);

	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time ;
	data.value_count = 3;
	data.values[0] = m_x;
	data.values[1] = m_y;
	data.values[2] = m_z;

	return 0;
}

bool gyro_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = MIN_RANGE(m_resolution)* RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	properties.max_range = MAX_RANGE(m_resolution)* RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	properties.min_interval = 1;
	properties.resolution = RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	properties.fifo_count = 0;
	properties.max_batch_count = 0;
	return true;

}

bool gyro_sensor_hal::add_gyro_channels_to_array(void)
{
	int i = 0;
	m_channels = (struct channel_parameters*) malloc(sizeof(struct channel_parameters) * m_generic_channel_names.size());
	for (vector <string>::iterator it = m_generic_channel_names.begin();
			it != m_generic_channel_names.end(); ++it) {
		if (add_channel_to_array(m_gyro_dir.c_str(), it->c_str() , &m_channels[i++]) < 0) {
			ERR("Failed to add channel %s to channel array", it->c_str());
			return false;
		}
	}
	return true;
}

bool gyro_sensor_hal::setup_channels(void)
{
	int freq, i;
	double sf;

	enable_resource(true);

	if (!add_gyro_channels_to_array()) {
		ERR("Failed to add channels to array!");
		return false;
	}

	INFO("Sorting channels by index");
	sort_channels_by_index(m_channels, m_generic_channel_names.size());
	INFO("Sorting channels by index completed");

	m_scan_size = get_channel_array_size(m_channels, m_generic_channel_names.size());
	if (m_scan_size == 0) {
		ERR("Channel array size is zero");
		return false;
	}

	m_data = new (std::nothrow) char[m_scan_size * GYRO_RINGBUF_LEN];
	if (m_data == NULL) {
		ERR("Couldn't create data buffer\n");
		return false;
	}

	FILE *fp = NULL;
	fp = fopen(m_available_freq_node_path.c_str(), "r");
	if (!fp) {
		ERR("Fail to open available frequencies file:%s\n", m_available_freq_node_path.c_str());
		return false;
	}

	for (i = 0; i < MAX_FREQ_COUNT; i++)
		m_sample_freq[i] = 0;

	i = 0;

	while (fscanf(fp, "%d", &freq) > 0)
		m_sample_freq[i++] = freq;

	m_sample_freq_count = i;

	fp = fopen(m_available_scale_node_path.c_str(), "r");
	if (!fp) {
		ERR("Fail to open available scale factors file:%s\n", m_available_scale_node_path.c_str());
		return false;
	}

	for (i = 0; i < MAX_SCALING_COUNT; i++)
		m_scale_factor[i] = 0;

	i = 0;

	while (fscanf(fp, "%lf", &sf) > 0)
		m_scale_factor[i++] = sf;

	m_scale_factor_count = i;

	return true;
}

void gyro_sensor_hal::decode_data(void)
{
	AUTOLOCK(m_value_mutex);

	m_x = convert_bytes_to_int(*(unsigned short int *)(m_data + m_channels[0].buf_index), &m_channels[0]);
	m_y = convert_bytes_to_int(*(unsigned short int *)(m_data + m_channels[1].buf_index), &m_channels[1]);
	m_z = convert_bytes_to_int(*(unsigned short int *)(m_data + m_channels[2].buf_index), &m_channels[2]);

	long long int val = *(long long int *)(m_data + m_channels[3].buf_index);
	if ((val >> m_channels[3].valid_bits) & 1)
		val = (val & m_channels[3].mask) | ~m_channels[3].mask;

	m_fired_time = (unsigned long long int)(NSEC_TO_MUSEC(val));
	INFO("m_x = %d, m_y = %d, m_z = %d, time = %lluus", m_x, m_y, m_z, m_fired_time);
}

bool gyro_sensor_hal::setup_trigger(const char* trig_name, bool verify)
{
	int ret = 0;

	ret = update_sysfs_string(m_trigger_path.c_str(), trig_name);
	if (ret < 0) {
		ERR("failed to write to current_trigger,%s,%s\n", m_trigger_path.c_str(), trig_name);
		return false;
	}
	INFO("current_trigger setup successfully\n");
	return true;
}

bool gyro_sensor_hal::setup_buffer(int enable)
{
	int ret;
	ret = update_sysfs_num(m_buffer_length_node_path.c_str(), GYRO_RINGBUF_LEN, true);
	if (ret < 0) {
		ERR("failed to write to buffer/length\n");
		return false;
	}
	INFO("buffer/length setup successfully\n");

	ret = update_sysfs_num(m_buffer_enable_node_path.c_str(), enable, true);
	if (ret < 0) {
		ERR("failed to write to buffer/enable\n");
		return false;
	}

	if (enable)
		INFO("buffer enabled\n");
	else
		INFO("buffer disabled\n");
	return true;
}

bool gyro_sensor_hal::enable_resource(bool enable)
{
	string temp;
	if(enable)
		setup_trigger(m_trigger_name.c_str(), enable);
	else
		setup_trigger("NULL", enable);

	for (vector <string>::iterator it = m_generic_channel_names.begin();
			it != m_generic_channel_names.end(); ++it) {
		temp = m_gyro_dir + string(SCAN_EL_DIR) + *it + string("_en");
		if (update_sysfs_num(temp.c_str(), enable) < 0)
			return false;
	}
	setup_buffer(enable);
	return true;
}

extern "C" void *create(void)
{
	gyro_sensor_hal *inst;

	try {
		inst = new gyro_sensor_hal();
	} catch (int err) {
		ERR("gyro_sensor class create fail , errno : %d , errstr : %s\n", err, strerror(err));
		return NULL;
	}

	return (void*)inst;
}

extern "C" void destroy(void *inst)
{
	delete (gyro_sensor_hal*)inst;
}
