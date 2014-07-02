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

#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/input.h>
#include <cconfig.h>
#include <gyro_sensor_hal.h>
#include <sys/poll.h>
#include <iio_common.h>

using std::ifstream;
using config::CConfig;

#define INITIAL_VALUE -1
#define INITIAL_TIME 0
#define DPS_TO_MDPS 1000
#define MIN_RANGE(RES) (-((2 << (RES))/2))
#define MAX_RANGE(RES) (((2 << (RES))/2)-1)
#define RAW_DATA_TO_DPS_UNIT(X) ((float)(X)/((float)DPS_TO_MDPS))

#define SEC_MSEC			1000
#define MSEC_TO_FREQ(VAL)	(int)((SEC_MSEC) / (VAL))

#define SENSOR_TYPE_GYRO		"GYRO"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ELEMENT_RAW_DATA_UNIT	"RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION		"RESOLUTION"
#define ATTR_VALUE				"value"

#define ENABLE_VAL				1
#define DISABLE_VAL				0
#define DEV_DIR					"/dev/"
#define TRIG_PATH				"trigger/current_trigger"

gyro_sensor_hal::gyro_sensor_hal()
: m_x(INITIAL_VALUE)
, m_y(INITIAL_VALUE)
, m_z(INITIAL_VALUE)
, m_polling_interval(POLL_1HZ_MS)
, m_fired_time(INITIAL_TIME)
, m_sensorhub_supported(false)
{
	if (!check_hw_node())
	{
		ERR("check_hw_node() fail");
		throw ENXIO;
	}

	CConfig &config = CConfig::get_instance();

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_VENDOR, m_vendor))
	{
		ERR("[VENDOR] is empty");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_NAME, m_chip_name))
	{
		ERR("[NAME] is empty");
		throw ENXIO;
	}

	INFO("m_chip_name = %s", m_chip_name.c_str());

	long resolution;

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_RESOLUTION, resolution))
	{
		ERR("[RESOLUTION] is empty");
		throw ENXIO;
	}

	m_resolution = (int)resolution;
	INFO("m_resolution = %d", m_resolution);

	double raw_data_unit;

	if (!config.get(SENSOR_TYPE_GYRO, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit))
	{
		ERR("[RAW_DATA_UNIT] is empty");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);
	INFO("m_raw_data_unit = %f", m_raw_data_unit);
	INFO("RAW_DATA_TO_DPS_UNIT(m_raw_data_unit) = [%f]", RAW_DATA_TO_DPS_UNIT(m_raw_data_unit));

	INFO("gyro_sensor_hal is created!");
}

gyro_sensor_hal::~gyro_sensor_hal()
{
	enable_resource(false);
	if (m_data != NULL)
		delete []m_data;
	if (m_fp_buffer > 0)
		close(m_fp_buffer);

	INFO("gyro_sensor_hal is destroyed!");
}

string gyro_sensor_hal::get_model_id(void)
{
	return m_model_id;
}

sensor_type_t gyro_sensor_hal::get_type(void)
{
	return GYROSCOPE_SENSOR;
}

bool gyro_sensor_hal::enable_resource(bool enable)
{
	string temp;
	int enable_val;

	if(enable)
		enable_val = ENABLE_VAL;
	else
		enable_val = DISABLE_VAL;

	temp = m_gyro_dir + string(SCAN_EL_DIR) + string(CHANNEL_NAME_X) + string(ENABLE_SUFFIX);
	update_sysfs_num(temp.c_str(), enable_val);
	temp = m_gyro_dir + string(SCAN_EL_DIR) + string(CHANNEL_NAME_Y) + string(ENABLE_SUFFIX);
	update_sysfs_num(temp.c_str(), enable_val);
	temp = m_gyro_dir + string(SCAN_EL_DIR) + string(CHANNEL_NAME_Z) + string(ENABLE_SUFFIX);
	update_sysfs_num(temp.c_str(), enable_val);
	temp = m_gyro_dir + string(SCAN_EL_DIR) + string(CHANNEL_NAME_TIME) + string(ENABLE_SUFFIX);
	update_sysfs_num(temp.c_str(), enable_val);
	setup_trigger(INPUT_TRIG_NAME, enable);
	setup_buffer(enable_val);

	return true;
}

bool gyro_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(true);
	set_interval(m_polling_interval);

	m_fired_time = INITIAL_TIME;
	INFO("Gyro sensor real starting");
	return true;
}

bool gyro_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	enable_resource(false);
	INFO("Gyro sensor real stopping");
	return true;
}

bool gyro_sensor_hal::set_interval(unsigned long ms_interval)
{
	int freq, i, approx_freq;
	freq = MSEC_TO_FREQ(ms_interval);

	for (i=0; i < m_sample_freq_count; i++)
	{
		if (freq == m_sample_freq[i])
		{
			if (update_sysfs_num(m_freq_resource.c_str(), freq, true) == 0)
			{
				INFO("Interval is changed from %lums to %lums]", m_polling_interval, ms_interval);
				m_polling_interval = ms_interval;
				return true;
			}
			else
			{
				ERR("Failed to set data %lu\n", ms_interval);
				return false;
			}
		}
	}

	INFO("The interval not supported: %lu\n", ms_interval);
	ERR("Failed to set data %lu\n", ms_interval);
	return false;
}

bool gyro_sensor_hal::update_value(bool wait)
{
	int i;
	struct pollfd pfd;
	ssize_t read_size;
	const int TIMEOUT = 1000;

	pfd.fd = m_fp_buffer;
	pfd.events = POLLIN;
	if (wait)
		poll(&pfd, 1, TIMEOUT);
	else
		poll(&pfd, 1, 0);

	read_size = read(m_fp_buffer, m_data, GYRO_RINGBUF_LEN * m_scan_size);

	if (read_size <= 0)
	{
		ERR("No gyro data available to read\n");
		return false;
	}

	for (i = 0; i < (read_size / m_scan_size); i++)
		decode_data();
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
	const int chance = 3;
	int retry = 0;

	while ((m_fired_time == INITIAL_TIME) && (retry++ < chance))
	{
		INFO("Try usleep for getting a valid BASE DATA value");
		usleep(m_polling_interval * MS_TO_SEC);
	}

	if (m_fired_time == INITIAL_TIME)
	{
		ERR("get_sensor_data failed");
		return -1;
	}

	data.data_accuracy = SENSOR_ACCURACY_GOOD;
	data.data_unit_idx = SENSOR_UNIT_VENDOR_UNIT;
	data.timestamp = m_fired_time ;
	data.values_num = 3;
	data.values[0] = m_x;
	data.values[1] = m_y;
	data.values[2] = m_z;

	return 0;
}

bool gyro_sensor_hal::get_properties(sensor_properties_t &properties)
{
	properties.sensor_unit_idx = SENSOR_UNIT_DEGREE_PER_SECOND;
	properties.sensor_min_range = MIN_RANGE(m_resolution) * RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	properties.sensor_max_range = MAX_RANGE(m_resolution) * RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	snprintf(properties.sensor_name,   sizeof(properties.sensor_name), "%s", m_chip_name.c_str());
	snprintf(properties.sensor_vendor, sizeof(properties.sensor_vendor), "%s", m_vendor.c_str());
	properties.sensor_resolution = RAW_DATA_TO_DPS_UNIT(m_raw_data_unit);
	return true;
}

bool gyro_sensor_hal::is_sensorhub_supported(void)
{
	return false;
}

bool gyro_sensor_hal::check_hw_node(void)
{
	string name_node;
	string hw_name;
	string file_name;
	string temp;
	DIR *main_dir = NULL;
	struct dirent *dir_entry = NULL;
	bool find_node = false;
	bool find_trigger = false;

	INFO("======================start check_hw_node=============================");

	m_sensorhub_supported = is_sensorhub_supported();
	main_dir = opendir(IIO_DIR);

	if (!main_dir)
	{
		ERR("Could not open IIO directory\n");
		return false;
	}

	m_channels = (struct channel_parameters*) malloc(sizeof(struct channel_parameters) * NO_OF_CHANNELS);

	while (!(find_node && find_trigger))
	{
		dir_entry = readdir(main_dir);
		if(dir_entry == NULL)
			break;

		if ((strncasecmp(dir_entry->d_name , ".", 1 ) != 0) && (strncasecmp(dir_entry->d_name , "..", 2 ) != 0) && (dir_entry->d_ino != 0))
		{
			file_name = string(IIO_DIR) + string(dir_entry->d_name) + string(NAME_NODE);
			ifstream infile(file_name.c_str());

			if (!infile)
				continue;

			infile >> hw_name;

			if (strncmp(dir_entry->d_name, IIO_DEV_BASE_NAME, IIO_DEV_STR_LEN) == 0)
			{
				if (CConfig::get_instance().is_supported(SENSOR_TYPE_GYRO, hw_name) == true)
				{
					m_gyro_dir = string(IIO_DIR) + string(dir_entry->d_name) + string("/");
					m_buffer_access = string(DEV_DIR) + string(dir_entry->d_name);
					m_name = m_model_id = hw_name;
					find_node = true;
					INFO("m_gyro_dir:%s\n", m_gyro_dir.c_str());
					INFO("m_buffer_access:%s\n", m_buffer_access.c_str());
					INFO("m_name:%s\n", m_name.c_str());
				}
			}

			if (strncmp(dir_entry->d_name, IIO_TRIG_BASE_NAME, IIO_TRIG_STR_LEN) == 0)
			{
				if (hw_name == string(INPUT_TRIG_NAME))
				{
					m_gyro_trig_dir = string(IIO_DIR) + string(dir_entry->d_name) + string("/");
					find_trigger = true;
					DBG("m_gyro_trig_dir:%s\n", m_gyro_trig_dir.c_str());
				}
			}

			if (find_node && find_trigger)
				break;
		}
	}

	closedir(main_dir);

	if (find_node && find_trigger)
	{
		if (setup_channels() == true)
			INFO("IIO channel setup successful");
		else
		{
			ERR("IIO channel setup failed");
			return false;
		}
	}
	return (find_node && find_trigger);
}

bool gyro_sensor_hal::add_gyro_channels_to_array(void)
{
	if (add_channel_to_array(m_gyro_dir.c_str(), CHANNEL_NAME_X, &m_channels[0]) < 0)
	{
		ERR("Failed to add %s to channel array", CHANNEL_NAME_X);
		return false;
	}
	if (add_channel_to_array(m_gyro_dir.c_str(), CHANNEL_NAME_Y, &m_channels[1]) < 0)
	{
		ERR("Failed to add %s to channel array", CHANNEL_NAME_Y);
		return false;
	}
	if (add_channel_to_array(m_gyro_dir.c_str(), CHANNEL_NAME_Z, &m_channels[2]) < 0)
	{
		ERR("Failed to add %s to channel array", CHANNEL_NAME_Z);
		return false;
	}
	if (add_channel_to_array(m_gyro_dir.c_str(), CHANNEL_NAME_TIME, &m_channels[3]) < 0)
	{
		ERR("Failed to add channel time_stamp to channel array");
		return false;
	}
	return true;
}

bool gyro_sensor_hal::setup_channels(void)
{
	int freq, i;
	double sf;
	string temp;

	enable_resource(true);

	if (!add_gyro_channels_to_array())
		return false;

	sort_channels_by_index(m_channels, NO_OF_CHANNELS);

	m_scan_size = get_channel_array_size(m_channels, NO_OF_CHANNELS);
	if (m_scan_size == 0)
	{
		ERR("Channel array size is zero");
		return false;
	}

	m_data = new (std::nothrow) char[m_scan_size * GYRO_RINGBUF_LEN];
	if (m_data == NULL)
	{
		ERR("Couldn't create data buffer\n");
		return false;
	}

	m_fp_buffer = open(m_buffer_access.c_str(), O_RDONLY | O_NONBLOCK);
	if (m_fp_buffer == -1)
	{
		ERR("Failed to open ring buffer(%s)\n", m_buffer_access.c_str());
		return false;
	}

	m_freq_resource = m_gyro_dir + string(GYRO_FREQ);
	temp = m_gyro_dir + string(GYRO_FREQ_AVLBL);

	FILE *fp = NULL;
	fp = fopen(temp.c_str(), "r");
	if (!fp)
	{
		ERR("Fail to open available frequencies file:%s\n", temp.c_str());
		return false;
	}

	for (i = 0; i < MAX_FREQ_COUNT; i++)
		m_sample_freq[i] = 0;

	i = 0;
	while (fscanf(fp, "%d", &freq) > 0)
		m_sample_freq[i++] = freq;
	m_sample_freq_count = i;

	temp = m_gyro_dir + string(GYRO_SCALE_AVLBL);
	fp = fopen(temp.c_str(), "r");
	if (!fp)
	{
		ERR("Fail to open available scale factors file:%s\n", temp.c_str());
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

	m_fired_time = (unsigned long long int)(val);
	DBG("m_x = %d, m_y = %d, m_z = %d, time = %lluus", m_x, m_y, m_z, m_fired_time);
}

bool gyro_sensor_hal::setup_trigger(char* trig_name, bool verify)
{
	string temp;
	int ret;

	temp = m_gyro_dir + string(TRIG_PATH);
	update_sysfs_string(temp.c_str(), trig_name, verify);
	if (ret < 0)
	{
		ERR("failed to write to current_trigger\n");
		return false;
	}
	INFO("current_trigger setup successfully\n");
	return true;
}

bool gyro_sensor_hal::setup_buffer(int enable)
{
	string temp;
	int ret;
	temp = m_gyro_dir + string(BUFFER_LEN);
	INFO("Buffer Length Setup: %s", temp.c_str());
	ret = update_sysfs_num(temp.c_str(), GYRO_RINGBUF_LEN, true);
	if (ret < 0)
	{
		ERR("failed to write to buffer/length\n");
		return false;
	}
	INFO("buffer/length setup successfully\n");

	temp = m_gyro_dir + string(BUFFER_EN);
	INFO("Buffer Enable: %s", temp.c_str());
	ret = update_sysfs_num(temp.c_str(), enable, true);
	if (ret < 0)
	{
		ERR("failed to write to buffer/enable\n");
		return false;
	}
	if (enable)
		INFO("buffer enabled\n");
	else
		INFO("buffer disabled\n");

	return true;
}

extern "C" void *create(void)
{
	gyro_sensor_hal *inst;

	try
	{
		inst = new gyro_sensor_hal();
	}
	catch (int err)
	{
		ERR("Failed to create gyro_sensor_hal class, errno : %d, errstr : %s", err, strerror(err));
		return NULL;
	}

	return (void *)inst;
}

extern "C" void destroy(void *inst)
{
	delete (gyro_sensor_hal *)inst;
}
