/*
 * libsensord-share
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

#include <sensor_plugin_loader.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sensor_hal.h>
#include <sensor_base.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sensor_logs.h>
#include <unordered_set>
#include <algorithm>

#include <accel_sensor.h>
#include <gyro_sensor.h>
#include <geo_sensor.h>
#include <light_sensor.h>
#include <proxi_sensor.h>
#include <pressure_sensor.h>
#include <bio_led_red_sensor.h>
#include <temperature_sensor.h>
#include <ultraviolet_sensor.h>
#include <rv_raw_sensor.h>
#include <auto_rotation_sensor.h>
#include <tilt_sensor.h>
#include <gravity_sensor.h>
#include <fusion_sensor.h>
#include <linear_accel_sensor.h>
#include <orientation_sensor.h>
#include <gaming_rv_sensor.h>
#include <geomagnetic_rv_sensor.h>
#include <rv_sensor.h>
#include <uncal_gyro_sensor.h>


using std::make_pair;
using std::equal;
using std::unordered_set;
using std::pair;
using std::vector;
using std::string;
using std::shared_ptr;
using std::static_pointer_cast;

#define ROOT_ELEMENT "PLUGIN"
#define TEXT_ELEMENT "text"
#define PATH_ATTR "path"
#define HAL_ELEMENT "HAL"
#define SENSOR_ELEMENT "SENSOR"

#define ACCELEROMETER_ENABLED 0x01
#define GYROSCOPE_ENABLED 0x02
#define GEOMAGNETIC_ENABLED 0x04
#define TILT_ENABLED 1
#define FUSION_ENABLED 1
#define AUTO_ROTATION_ENABLED 1
#define GAMING_RV_ENABLED 3
#define GEOMAGNETIC_RV_ENABLED 5
#define ORIENTATION_ENABLED 7

#define SENSOR_PLUGINS_DIR_PATH "/usr/lib/libsensord-plugins.so"
#define HAL_PLUGINS_DIR_PATH "/usr/lib/libsensor-hal.so"

#define SENSOR_INDEX_SHIFT 16

sensor_plugin_loader::sensor_plugin_loader()
{
}

sensor_plugin_loader& sensor_plugin_loader::get_instance()
{
	static sensor_plugin_loader inst;
	return inst;
}

bool sensor_plugin_loader::load_module(const string &path, vector<void*> &sensors, void* &handle)
{
	void *_handle = dlopen(path.c_str(), RTLD_NOW);

	if (!_handle) {
		ERR("Failed to dlopen(%s), dlerror : %s", path.c_str(), dlerror());
		return false;
	}

	dlerror();

	create_t create_module = (create_t) dlsym(_handle, "create");

	if (!create_module) {
		ERR("Failed to find symbols in %s", path.c_str());
		dlclose(_handle);
		return false;
	}

	sensor_module *module = create_module();

	if (!module) {
		ERR("Failed to create module, path is %s\n", path.c_str());
		dlclose(_handle);
		return false;
	}

	sensors.clear();
	sensors.swap(module->sensors);

	delete module;
	handle = _handle;

	return true;
}

bool sensor_plugin_loader::insert_module(plugin_type type, const string &path)
{
	if (type == PLUGIN_TYPE_HAL) {
		DBG("Insert HAL plugin [%s]", path.c_str());

		std::vector<void *>hals;
		void *handle;

		if (!load_module(path, hals, handle))
			return false;

		shared_ptr<sensor_hal> hal;

		for (unsigned int i = 0; i < hals.size(); ++i) {
			hal.reset(static_cast<sensor_hal*> (hals[i]));
			sensor_hal_type_t sensor_hal_type = hal->get_type();
			m_sensor_hals.insert(make_pair(sensor_hal_type, hal));
		}
	} else if (type == PLUGIN_TYPE_SENSOR) {
		DBG("Insert Sensor plugin [%s]", path.c_str());

		std::vector<void *> sensors;
		void *handle;

		if (!load_module(path, sensors, handle))
			return false;

		shared_ptr<sensor_base> sensor;

		for (unsigned int i = 0; i < sensors.size(); ++i) {
			sensor.reset(static_cast<sensor_base*> (sensors[i]));

			if (!sensor->init()) {
				ERR("Failed to init [%s] module\n", sensor->get_name());
				continue;
			}

			DBG("init [%s] module", sensor->get_name());

			vector<sensor_type_t> sensor_types;

			sensor->get_types(sensor_types);

			for (unsigned int i = 0; i < sensor_types.size(); ++i) {
				int idx;
				idx = m_sensors.count(sensor_types[i]);
				sensor->add_id(idx << SENSOR_INDEX_SHIFT | sensor_types[i]);
				m_sensors.insert(make_pair(sensor_types[i], sensor));
			}
		}
	}else {
		ERR("Not supported type: %d", type);
		return false;
	}

	return true;
}

bool sensor_plugin_loader::load_plugins(void)
{
	insert_module(PLUGIN_TYPE_HAL, HAL_PLUGINS_DIR_PATH);
//	insert_module(PLUGIN_TYPE_SENSOR, SENSOR_PLUGINS_DIR_PATH);

	int enable_virtual_sensor = 0;

	vector<void*> sensors;

	sensor_hal *accel_hal = get_sensor_hal(SENSOR_HAL_TYPE_ACCELEROMETER);
	if (accel_hal != NULL) {
		enable_virtual_sensor |= ACCELEROMETER_ENABLED;

		accel_sensor* accel_sensor_ptr = NULL;
		try {
			accel_sensor_ptr = new(std::nothrow) accel_sensor;
		} catch (int err) {
			ERR("Failed to create accel_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (accel_sensor_ptr != NULL)
			sensors.push_back(accel_sensor_ptr);
	}

	sensor_hal *gyro_hal = get_sensor_hal(SENSOR_HAL_TYPE_GYROSCOPE);
	if (gyro_hal != NULL) {
		enable_virtual_sensor |= GYROSCOPE_ENABLED;

		gyro_sensor* gyro_sensor_ptr = NULL;
		try {
			gyro_sensor_ptr = new(std::nothrow) gyro_sensor;
		} catch (int err) {
			ERR("Failed to create gyro_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (gyro_sensor_ptr != NULL)
			sensors.push_back(gyro_sensor_ptr);
	}

	sensor_hal *geo_hal = get_sensor_hal(SENSOR_HAL_TYPE_GEOMAGNETIC);
	if (geo_hal != NULL) {
		enable_virtual_sensor |= GEOMAGNETIC_ENABLED;

		geo_sensor* geo_sensor_ptr = NULL;
		try {
			geo_sensor_ptr = new(std::nothrow) geo_sensor;
		} catch (int err) {
			ERR("Failed to create geo_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (geo_sensor_ptr != NULL)
			sensors.push_back(geo_sensor_ptr);
	}

	fusion_sensor* fusion_sensor_ptr = NULL;
	try {
		fusion_sensor_ptr = new(std::nothrow) fusion_sensor;
	} catch (int err) {
		ERR("Failed to create fusion_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (fusion_sensor_ptr != NULL)
		sensors.push_back(fusion_sensor_ptr);

	if (enable_virtual_sensor & TILT_ENABLED == TILT_ENABLED) {
		tilt_sensor* tilt_sensor_ptr = NULL;
		try {
			tilt_sensor_ptr = new(std::nothrow) tilt_sensor;
		} catch (int err) {
			ERR("Failed to create tilt_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (tilt_sensor_ptr != NULL)
			sensors.push_back(tilt_sensor_ptr);
	}
	if (enable_virtual_sensor & AUTO_ROTATION_ENABLED == AUTO_ROTATION_ENABLED) {
		auto_rotation_sensor* auto_rot_sensor_ptr = NULL;
		try {
			auto_rot_sensor_ptr = new(std::nothrow) auto_rotation_sensor;
		} catch (int err) {
			ERR("Failed to create auto_rotation_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (auto_rot_sensor_ptr != NULL)
			sensors.push_back(auto_rot_sensor_ptr);
	}
	if (enable_virtual_sensor & ORIENTATION_ENABLED == ORIENTATION_ENABLED) {
		gravity_sensor* gravity_sensor_ptr = NULL;
		try {
			gravity_sensor_ptr = new(std::nothrow) gravity_sensor;
		} catch (int err) {
			ERR("Failed to create tilt_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (gravity_sensor_ptr != NULL)
			sensors.push_back(gravity_sensor_ptr);

		linear_accel_sensor* linear_accel_sensor_ptr = NULL;
		try {
			linear_accel_sensor_ptr = new(std::nothrow) linear_accel_sensor;
		} catch (int err) {
			ERR("Failed to create linear_accel_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (linear_accel_sensor_ptr != NULL)
			sensors.push_back(linear_accel_sensor_ptr);

		orientation_sensor* orientation_sensor_ptr = NULL;
		try {
			orientation_sensor_ptr = new(std::nothrow) orientation_sensor;
		} catch (int err) {
			ERR("Failed to create orientation_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (orientation_sensor_ptr != NULL)
			sensors.push_back(orientation_sensor_ptr);

		rv_sensor* rv_sensor_ptr = NULL;
		try {
			rv_sensor_ptr = new(std::nothrow) rv_sensor;
		} catch (int err) {
			ERR("Failed to create rv_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (rv_sensor_ptr != NULL)
			sensors.push_back(rv_sensor_ptr);

		uncal_gyro_sensor* uncal_gyro_sensor_ptr = NULL;
		try {
			uncal_gyro_sensor_ptr = new(std::nothrow) uncal_gyro_sensor;
		} catch (int err) {
			ERR("Failed to create uncal_gyro_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (uncal_gyro_sensor_ptr != NULL)
			sensors.push_back(uncal_gyro_sensor_ptr);

	}

	if (enable_virtual_sensor & GAMING_RV_ENABLED == GAMING_RV_ENABLED) {
		gaming_rv_sensor* gaming_rv_sensor_ptr = NULL;
		try {
			gaming_rv_sensor_ptr = new(std::nothrow) gaming_rv_sensor;
		} catch (int err) {
			ERR("Failed to create gaming_rv_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (gaming_rv_sensor_ptr != NULL)
			sensors.push_back(gaming_rv_sensor_ptr);
	}

	if (enable_virtual_sensor & GEOMAGNETIC_RV_ENABLED == GEOMAGNETIC_RV_ENABLED) {
		geomagnetic_rv_sensor* geomagnetic_rv_sensor_ptr = NULL;
		try {
			geomagnetic_rv_sensor_ptr = new(std::nothrow) geomagnetic_rv_sensor;
		} catch (int err) {
			ERR("Failed to create geomagnetic_rv_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (geomagnetic_rv_sensor_ptr != NULL)
			sensors.push_back(geomagnetic_rv_sensor_ptr);
	}

	sensor_hal *light_hal = get_sensor_hal(SENSOR_HAL_TYPE_LIGHT);
	if (light_hal != NULL) {
		light_sensor* light_sensor_ptr = NULL;
		try {
			light_sensor_ptr = new(std::nothrow) light_sensor;
		} catch (int err) {
			ERR("Failed to create light_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (light_sensor_ptr != NULL)
			sensors.push_back(light_sensor_ptr);
	}

	sensor_hal *proxi_hal = get_sensor_hal(SENSOR_HAL_TYPE_PROXIMITY);
	if (proxi_hal != NULL) {
		proxi_sensor* proxi_sensor_ptr = NULL;
		try {
			proxi_sensor_ptr = new(std::nothrow) proxi_sensor;
		} catch (int err) {
			ERR("Failed to create proxi_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (proxi_sensor_ptr != NULL)
			sensors.push_back(proxi_sensor_ptr);
	}

	sensor_hal *pressure_hal = get_sensor_hal(SENSOR_HAL_TYPE_PRESSURE);
	if (pressure_hal != NULL) {
		pressure_sensor* pressure_sensor_ptr = NULL;
		try {
			pressure_sensor_ptr = new(std::nothrow) pressure_sensor;
		} catch (int err) {
			ERR("Failed to create pressure_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (pressure_sensor_ptr != NULL)
			sensors.push_back(pressure_sensor_ptr);
	}

	sensor_hal *temp_hal = get_sensor_hal(SENSOR_HAL_TYPE_TEMPERATURE);
	if (temp_hal != NULL) {
		temperature_sensor* temp_sensor_ptr = NULL;
		try {
			temp_sensor_ptr = new(std::nothrow) temperature_sensor;
		} catch (int err) {
			ERR("Failed to create temperature_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (temp_sensor_ptr != NULL)
			sensors.push_back(temp_sensor_ptr);
	}

	sensor_hal *ultra_hal = get_sensor_hal(SENSOR_HAL_TYPE_ULTRAVIOLET);
	if (ultra_hal != NULL) {
		ultraviolet_sensor* ultra_sensor_ptr = NULL;
		try {
			ultra_sensor_ptr = new(std::nothrow) ultraviolet_sensor;
		} catch (int err) {
			ERR("Failed to create ultraviolet_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (ultra_sensor_ptr != NULL)
			sensors.push_back(ultra_sensor_ptr);
	}

	sensor_hal *bio_led_red_hal = get_sensor_hal(SENSOR_HAL_TYPE_BIO_LED_RED);
	if (bio_led_red_hal != NULL) {
		bio_led_red_sensor* bio_led_red_sensor_ptr = NULL;
		try {
			bio_led_red_sensor_ptr = new(std::nothrow) bio_led_red_sensor;
		} catch (int err) {
			ERR("Failed to create bio_led_red_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (bio_led_red_sensor_ptr != NULL)
			sensors.push_back(bio_led_red_sensor_ptr);
	}

	sensor_hal *rv_raw_hal = get_sensor_hal(SENSOR_HAL_TYPE_RV_RAW);
	if (rv_raw_hal != NULL) {
		rv_raw_sensor* rv_raw_sensor_ptr = NULL;
		try {
			rv_raw_sensor_ptr = new(std::nothrow) rv_raw_sensor;
		} catch (int err) {
			ERR("Failed to create rv_raw_sensor module, err: %d, cause: %s", err, strerror(err));
		}
		if (rv_raw_sensor_ptr != NULL)
			sensors.push_back(rv_raw_sensor_ptr);
	}

	shared_ptr<sensor_base> sensor;

	for (unsigned int i = 0; i < sensors.size(); ++i) {
		sensor.reset(static_cast<sensor_base*> (sensors[i]));

		if (!sensor->init()) {
			ERR("Failed to init [%s] module\n", sensor->get_name());
			continue;
		}

		DBG("init [%s] module", sensor->get_name());

		vector<sensor_type_t> sensor_types;

		sensor->get_types(sensor_types);

		for (unsigned int i = 0; i < sensor_types.size(); ++i) {
			int idx;
			idx = m_sensors.count(sensor_types[i]);
			sensor->add_id(idx << SENSOR_INDEX_SHIFT | sensor_types[i]);
			m_sensors.insert(make_pair(sensor_types[i], sensor));
		}
	}

	INFO("enable_virtual_sensor = %d\n", enable_virtual_sensor);
	show_sensor_info();
	return true;
}

void sensor_plugin_loader::show_sensor_info(void)
{
	INFO("========== Loaded sensor information ==========\n");

	int index = 0;

	auto it = m_sensors.begin();

	while (it != m_sensors.end()) {
		shared_ptr<sensor_base> sensor = it->second;

		sensor_info info;
		sensor->get_sensor_info(it->first, info);
		INFO("No:%d [%s]\n", ++index, sensor->get_name());
		info.show();
		it++;
	}

	INFO("===============================================\n");
}

sensor_hal* sensor_plugin_loader::get_sensor_hal(sensor_hal_type_t type)
{
	auto it_plugins = m_sensor_hals.find(type);

	if (it_plugins == m_sensor_hals.end())
		return NULL;

	return it_plugins->second.get();
}

vector<sensor_hal *> sensor_plugin_loader::get_sensor_hals(sensor_hal_type_t type)
{
	vector<sensor_hal *> sensor_hal_list;
	pair<sensor_hal_plugins::iterator, sensor_hal_plugins::iterator> ret;
	ret = m_sensor_hals.equal_range(type);

	for (auto it = ret.first; it != ret.second; ++it)
		sensor_hal_list.push_back(it->second.get());

	return sensor_hal_list;
}

sensor_base* sensor_plugin_loader::get_sensor(sensor_type_t type)
{
	auto it_plugins = m_sensors.find(type);

	if (it_plugins == m_sensors.end())
		return NULL;

	return it_plugins->second.get();
}

vector<sensor_base *> sensor_plugin_loader::get_sensors(sensor_type_t type)
{
	vector<sensor_base *> sensor_list;
	pair<sensor_plugins::iterator, sensor_plugins::iterator> ret;

	if (type == ALL_SENSOR)
		ret = std::make_pair(m_sensors.begin(), m_sensors.end());
	else
		ret = m_sensors.equal_range(type);

	for (auto it = ret.first; it != ret.second; ++it)
		sensor_list.push_back(it->second.get());

	return sensor_list;
}


sensor_base* sensor_plugin_loader::get_sensor(sensor_id_t id)
{
	vector<sensor_base *> sensors;

	sensor_type_t type = (sensor_type_t) (id & SENSOR_TYPE_MASK);
	unsigned int index = id >> SENSOR_INDEX_SHIFT;

	sensors = get_sensors(type);

	if (sensors.size() <= index)
		return NULL;

	return sensors[index];
}


vector<sensor_base *> sensor_plugin_loader::get_virtual_sensors(void)
{
	vector<sensor_base *> virtual_list;
	sensor_base* module;

	for (auto sensor_it = m_sensors.begin(); sensor_it != m_sensors.end(); ++sensor_it) {
		module = sensor_it->second.get();

		if (module && module->is_virtual() == true) {
			virtual_list.push_back(module);
		}
	}

	return virtual_list;
}

