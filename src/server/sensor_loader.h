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

#ifndef _SENSOR_PLUGIN_LOADER_H_
#define _SENSOR_PLUGIN_LOADER_H_

#include <sensor_common.h>
#include <sensor_types.h>
#include <sensor_hal.h>

#include <cmutex.h>
#include <sstream>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <physical_sensor.h>
#include <virtual_sensor.h>

class sensor_base;

typedef std::multimap<sensor_type_t, std::shared_ptr<sensor_base>> sensor_map_t;
typedef std::map<const sensor_handle_t *, sensor_device *> sensor_device_map_t;

class sensor_loader {
private:
	sensor_loader();

	bool load_hal(const std::string &path, std::vector<sensor_device_t> &devices, void* &handle);

	template <typename _sensor> sensor_base* create_sensor(void);

	template <typename _sensor> void load_physical_sensor(sensor_type_t type = UNKNOWN_SENSOR);
	bool load_devices(std::vector<sensor_device_t> &devices);
	void load_physical_sensors(void);

	template <typename _sensor> void load_virtual_sensor(const char *name);
	void load_virtual_sensors(void);

	void show_sensor_info(void);
	bool get_paths_from_dir(const std::string &dir_path, std::vector<std::string> &plugin_paths);

	sensor_map_t m_sensors;
	sensor_device_map_t m_devices;
public:
	static sensor_loader& get_instance();
	bool load_sensors(void);

	sensor_base* get_sensor(sensor_type_t type);
	sensor_base* get_sensor(sensor_id_t id);

	std::vector<sensor_type_t> get_sensor_types(void);
	std::vector<sensor_base *> get_sensors(sensor_type_t type);
	std::vector<sensor_base *> get_virtual_sensors(void);
};
#endif	/* _SENSOR_PLUGIN_LOADER_H_ */
