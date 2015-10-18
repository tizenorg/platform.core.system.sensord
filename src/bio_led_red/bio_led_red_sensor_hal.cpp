/*
 * bio_led_red_sensor_hal
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
#include <csensor_config.h>
#include <bio_led_red_sensor_hal.h>
#include <sys/ioctl.h>
#include <fstream>

using std::ifstream;
using std::string;

#define SENSOR_TYPE_BIO_LED_RED	"BIO_LED_RED"
#define ELEMENT_NAME			"NAME"
#define ELEMENT_VENDOR			"VENDOR"
#define ATTR_VALUE				"value"
#define PATH_LED_RED_ENABLE		"/sys/class/sensors/hrm_sensor/led_current1";
#define LED_RED_ENABLE_VALUE	255

#define BIAS	1

bio_led_red_sensor_hal::bio_led_red_sensor_hal()
: m_polling_interval(POLL_1HZ_MS)
, m_bio_led_red(0)
, m_fired_time(0)
, m_node_handle(-1)
{
	const string sensorhub_interval_node_name = "hrm_poll_delay";
	csensor_config &config = csensor_config::get_instance();

	node_info_query query;
	node_info info;

	if (!find_model_id(SENSOR_TYPE_BIO_LED_RED, m_model_id)) {
		ERR("Failed to find model id");
		throw ENXIO;

	}

	query.sensorhub_controlled = m_sensorhub_controlled = is_sensorhub_controlled(sensorhub_interval_node_name);
	query.sensor_type = SENSOR_TYPE_BIO_LED_RED;
	query.key = "hrm_sensor";
	query.iio_enable_node_name = "hrm_raw_enable";
	query.sensorhub_interval_node_name = sensorhub_interval_node_name;

	if (!get_node_info(query, info)) {
		ERR("Failed to get node info");
		throw ENXIO;
	}

	show_node_info(info);

	m_data_node = info.data_node_path;
	m_enable_node = info.enable_node_path;
	m_interval_node = info.interval_node_path;

	if (!config.get(SENSOR_TYPE_BIO_LED_RED, m_model_id, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_BIO_LED_RED, m_model_id, ELEMENT_NAME, m_chip_name)) {
		ERR("[NAME] is empty\n");
		throw ENXIO;
	}

	INFO("m_chip_name = %s\n",m_chip_name.c_str());

	if ((m_node_handle = open(m_data_node.c_str(),O_RDWR)) < 0) {
		ERR("Failed to open handle(%d)", m_node_handle);
		throw ENXIO;
	}

	int clockId = CLOCK_MONOTONIC;
	if (ioctl(m_node_handle, EVIOCSCLOCKID, &clockId) != 0)
		ERR("Fail to set monotonic timestamp for %s", m_data_node.c_str());

	INFO("bio_led_red_sensor_hal is created!\n");

}

bio_led_red_sensor_hal::~bio_led_red_sensor_hal()
{
	close(m_node_handle);
	m_node_handle = -1;

	INFO("bio_led_red_sensor_hal is destroyed!\n");
}

string bio_led_red_sensor_hal::get_model_id(void)
{
	return m_model_id;
}


sensor_hal_type_t bio_led_red_sensor_hal::get_type(void)
{
	return SENSOR_HAL_TYPE_BIO_LED_RED;
}

bool bio_led_red_sensor_hal::enable(void)
{
	AUTOLOCK(m_mutex);
	const string led_red_enable_path = PATH_LED_RED_ENABLE;

	set_enable_node(m_enable_node, m_sensorhub_controlled, true, SENSORHUB_BIO_HRM_RAW_ENABLE_BIT);

	if (!set_node_value(led_red_enable_path, LED_RED_ENABLE_VALUE)) {
		ERR("Failed to set led red enable node: %s", led_red_enable_path.c_str());
		return false;
	}

	set_interval(m_polling_interval);

	m_fired_time = 0;
	INFO("bio_led_red sensor starting");
	return true;
}

bool bio_led_red_sensor_hal::disable(void)
{
	AUTOLOCK(m_mutex);

	set_enable_node(m_enable_node, m_sensorhub_controlled, false, SENSORHUB_BIO_HRM_RAW_ENABLE_BIT);

	INFO("bio_led_red sensor real stopping");
	return true;
}

bool bio_led_red_sensor_hal::set_interval(unsigned long val)
{
	unsigned long long polling_interval_ns;

	AUTOLOCK(m_mutex);

	polling_interval_ns = ((unsigned long long)(val) * 1000llu * 1000llu);

	if (!set_node_value(m_interval_node, polling_interval_ns)) {
		ERR("Failed to set polling resource: %s\n", m_interval_node.c_str());
		return false;
	}

	INFO("Interval is changed from %dms to %dms]", m_polling_interval, val);
	m_polling_interval = val;
	return true;

}


bool bio_led_red_sensor_hal::update_value(bool wait)
{
	int bio_led_red_raw = -1;
	bool bio_led_red = false;
	int read_input_cnt = 0;
	const int INPUT_MAX_BEFORE_SYN = 10;
	unsigned long long fired_time = 0;
	bool syn = false;

	struct input_event bio_led_red_event;
	DBG("bio_led_red event detection!");

	while ((syn == false) && (read_input_cnt < INPUT_MAX_BEFORE_SYN)) {
		int len = read(m_node_handle, &bio_led_red_event, sizeof(bio_led_red_event));
		if (len != sizeof(bio_led_red_event)) {
			ERR("bio_led_red file read fail, read_len = %d\n",len);
			return false;
		}

		++read_input_cnt;

		if (bio_led_red_event.type == EV_REL && bio_led_red_event.code == REL_X) {
			bio_led_red_raw = (int)bio_led_red_event.value;
			bio_led_red = true;
		} else if (bio_led_red_event.type == EV_REL) {
			ERR("bio_led_red event[type = %d, code = %d] is skipped.", bio_led_red_event.type, bio_led_red_event.code);
		} else if (bio_led_red_event.type == EV_SYN) {
			syn = true;
			fired_time = sensor_hal::get_timestamp(&bio_led_red_event.time);
		} else {
			ERR("bio_led_red event[type = %d, code = %d] is unknown.", bio_led_red_event.type, bio_led_red_event.code);
			return false;
		}
	}

	AUTOLOCK(m_value_mutex);

	if (bio_led_red)
		m_bio_led_red = bio_led_red_raw - BIAS;

	m_fired_time = fired_time;

	DBG("m_bio_led_red = %d, time = %lluus", m_bio_led_red, m_fired_time);

	return true;
}

bool bio_led_red_sensor_hal::is_data_ready(bool wait)
{
	bool ret;
	ret = update_value(wait);
	return ret;
}

int bio_led_red_sensor_hal::get_sensor_data(sensor_data_t &data)
{
	AUTOLOCK(m_value_mutex);
	data.accuracy = SENSOR_ACCURACY_GOOD;
	data.timestamp = m_fired_time;
	data.value_count = 1;
	data.values[0] = (float) m_bio_led_red;

	return 0;
}

bool bio_led_red_sensor_hal::get_properties(sensor_properties_s &properties)
{
	properties.name = m_chip_name;
	properties.vendor = m_vendor;
	properties.min_range = 0.0f;
	properties.max_range = 1.0f;
	properties.min_interval = 1;
	properties.resolution = 1.0f;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;

	return true;
}

extern "C" sensor_module* create(void)
{
	bio_led_red_sensor_hal *sensor;

	try {
		sensor = new(std::nothrow) bio_led_red_sensor_hal;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
