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

#if !defined(_SENSOR_PLUGIN_LOADER_CLASS_H_)
#define _SENSOR_PLUGIN_LOADER_CLASS_H_

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

class sensor_hal;
class sensor_base;

typedef std::multimap<sensor_hal_type, std::shared_ptr<sensor_base> > sensor_plugins;

/* the key is poll_fd */
typedef std::map<int, std::shared_ptr<sensor_hal> > sensor_hal_plugins;

class sensor_plugin_loader
{
private:
	sensor_plugin_loader();

	bool load_plugin(const std::string &path, std::vector<void *> &sensors, void* &handle);
	bool insert_plugins(std::vector<void *> hals);
	bool insert_sensors(sensor_hal *hal);
	void show_sensor_info(void);

	bool get_paths_from_dir(const std::string &dir_path, std::vector<std::string> &hal_paths);

	sensor_hal_plugins m_sensor_hals;
	sensor_plugins m_sensors;
public:
	static sensor_plugin_loader& get_instance();
	bool load_plugins(void);

	sensor_base* get_sensor(sensor_type_t type);
	sensor_base* get_sensor(sensor_id_t id);

	std::vector<sensor_base *> get_sensors(sensor_type_t type);

	std::vector<sensor_hal *> get_sensor_hals();
	sensor_hal* get_sensor_hals(int fd);

	std::vector<sensor_base*> get_virtual_sensors(void);
};
#endif	/* _SENSOR_PLUGIN_LOADER_CLASS_H_ */
