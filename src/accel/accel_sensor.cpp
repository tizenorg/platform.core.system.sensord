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
#include <sensors_hal.h>
#include <accel_sensor.h>

using std::string;
using std::vector;

#define GRAVITY 9.80665
#define G_TO_MG 1000
#define RAW_DATA_TO_G_UNIT(X) (((float)(X))/((float)G_TO_MG))
#define RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(X) (GRAVITY * (RAW_DATA_TO_G_UNIT(X)))

#define SENSOR_NAME                     "ACCELEROMETER_SENSOR"

#define MIN_RANGE(RES)                  (-((1 << (RES))/2))
#define MAX_RANGE(RES)                  (((1 << (RES))/2)-1)

#define ELEMENT_NAME                    "NAME"
#define ELEMENT_VENDOR                  "VENDOR"
#define ELEMENT_RAW_DATA_UNIT           "RAW_DATA_UNIT"
#define ELEMENT_RESOLUTION              "RESOLUTION"
#define ATTR_VALUE                      "value"
#define INPUT_NAME                      "accelerometer_sensor"
#define ACCEL_SENSORHUB_POLL_NODE_NAME  "accel_poll_delay"

accel_sensor::accel_sensor()
: m_x(-1)
, m_y(-1)
, m_z(-1)
, m_fired_time(0)
, m_polling_interval(POLL_1HZ_MS)
, m_handle(-1)
, m_node_handle(-1)
{
	m_name = string(SENSOR_NAME);

	const string sensorhub_interval_node_name = "accel_poll_delay";
	CConfig &config = CConfig::get_instance();

	if (!find_model_id(SENSOR_TYPE_ACCEL, m_model_id)) {
		ERR("Failed to find model id");
		throw ENXIO;
	}

	node_info_query query;
	node_info info;

	m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensorhub_controlled = m_sensorhub_controlled;
	query.sensor_type = "ACCEL";
	query.key = "accelerometer_sensor";
	query.iio_enable_node_name = "accel_enable";
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	show_node_info(info);

	m_method = info.method;
	m_data_node = info.data_node_path;
	m_enable_node = info.enable_node_path;
	m_interval_node = info.interval_node_path;

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	long resolution;
	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_RESOLUTION, resolution)) {
		ERR("[RESOLUTION] is empty\n");
		throw ENXIO;
	}
	m_resolution = (int)resolution;

	double raw_data_unit;
	if (!config.get(SENSOR_TYPE_ACCEL, m_model_id, ELEMENT_RAW_DATA_UNIT, raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	m_raw_data_unit = (float)(raw_data_unit);

	if ((m_node_handle = open(m_data_node.c_str(), O_RDWR)) < 0) {
		ERR("accel handle open fail for accel processor, error:%s\n", strerror(errno));
		throw ENXIO;
	}

	int clockId = CLOCK_MONOTONIC;
	if (ioctl(m_node_handle, EVIOCSCLOCKID, &clockId) != 0) {
		ERR("Fail to set monotonic timestamp for %s", m_data_node.c_str());
		throw ENXIO;
	}

	/*
	register_supported_event(ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	register_supported_event(ACCELEROMETER_EVENT_UNPROCESSED_DATA_REPORT_ON_TIME);
	*/
	INFO("%s is created!", m_name);
}

accel_sensor::~accel_sensor()
{
	INFO("%s is destroyed!", m_name);
}

bool accel_sensor::initialize(void)
{
	close(m_node_handle);
	m_node_handle = -1;

	INFO("%s is created!\n", m_name);
	return true;
}

bool accel_sensor::enable(void)
{
	AUTOLOCK(m_mutex);

	set_enable_node(m_enable_node, m_sensorhub_controlled, true, SENSORHUB_ACCELEROMETER_ENABLE_BIT);
	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("Accel sensor real starting");
	return true;
}

bool disable(void)
{
	AUTOLOCK(m_mutex);

	set_enable_node(m_enable_node, m_sensorhub_controlled, false, SENSORHUB_ACCELEROMETER_ENABLE_BIT);

	INFO("Accel sensor real stopping");
	return true;
}

bool set_handle(int handle)
{
	m_handle = handle;
}

virtual bool get_fd(int &fd)
{
	return m_node_handle;
}

virtual bool get_info(sensor_info_t &info)
{
	info.name = m_chip_name;
	info.vendor = m_vendor;
	info.min_range = MIN_RANGE(m_resolution)* RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(m_raw_data_unit);
	info.max_range = MAX_RANGE(m_resolution)* RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(m_raw_data_unit);
	info.min_interval = 1;
	info.resolution = m_raw_data_unit;
	info.fifo_count = 0;
	info.max_batch_count = 0;
	return true;
}

virtual bool get_sensor_data(sensor_data_t &data)
{
	int accel_raw[3] = {0,};
	unsigned long long fired_time = 0;
	int read_input_cnt = 0;
	const int INPUT_MAX_BEFORE_SYN = 10;
	bool x, y, z;
	bool syn = false;
	x = y = z = false;

	struct input_event data;
	DBG("accel event detection!");

	while ((syn == false) && (read_input_cnt < INPUT_MAX_BEFORE_SYN)) {
		int len = read(m_node_handle, &data, sizeof(data));
		if (len != sizeof(data)) {
			ERR("accel_file read fail, read_len = %d\n",len);
			return false;
		}

		++read_input_cnt;

		if (data.type == EV_REL) {
			switch (data.code) {
				case REL_X:
					accel_raw[0] = (int)data.value;
					x = true;
					break;
				case REL_Y:
					accel_raw[1] = (int)data.value;
					y = true;
					break;
				case REL_Z:
					accel_raw[2] = (int)data.value;
					z = true;
					break;
				default:
					ERR("data event[type = %d, code = %d] is unknown.", data.type, data.code);
					return false;
			}
		} else if (data.type == EV_SYN) {
			syn = true;
			fired_time = sensor_hal::get_timestamp(&data.time);
		} else {
			ERR("data event[type = %d, code = %d] is unknown.", data.type, data.code);
			return false;
		}
	}

	if (syn == false) {
		ERR("EV_SYN didn't come until %d inputs had come", read_input_cnt);
		return false;
	}

	AUTOLOCK(m_value_mutex);

	if (x)
		m_x =  accel_raw[0];
	if (y)
		m_y =  accel_raw[1];
	if (z)
		m_z =  accel_raw[2];

	m_fired_time = fired_time;

	ERR("m_x = %d, m_y = %d, m_z = %d, time = %lluus", m_x, m_y, m_z, m_fired_time);

	return true;
}

virtual bool set_command(unsigned int cmd, long val)
{
	return false;
}

virtual bool batch(int flags,
		unsigned long long interval_ms,
		unsigned long long max_report_latency_ns)
{
	unsigned long long polling_interval_ns;

	AUTOLOCK(m_mutex);

	polling_interval_ns = ((unsigned long long)(interval_ms) * 1000llu * 1000llu);

	if (!set_node_value(m_interval_node, polling_interval_ns)) {
		ERR("Failed to set polling resource: %s\n", m_interval_node.c_str());
		return false;
	}

	INFO("Interval is changed from %dms to %dms]", m_polling_interval, val);
	m_polling_interval = val;
	return true;
}

virtual bool flush(void)
{
	return false;
}

void accel_sensor::raw_to_base(sensor_data_t &data)
{
	data.value_count = 3;
	data.values[0] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[0] * m_raw_data_unit);
	data.values[1] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[1] * m_raw_data_unit);
	data.values[2] = RAW_DATA_TO_METRE_PER_SECOND_SQUARED_UNIT(data.values[2] * m_raw_data_unit);
}
