/*
 * libsensord-share
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#include <cmutex.h>
#include <sstream>

#include <string>
#include <vector>
#include <map>

class sensor_hal;
class sensor_base;
class sensor_fusion;

using std::pair;
using std::vector;
using std::multimap;
using std::string;
using std::istringstream;

#define PLUGINS_FILE_PATH "/usr/etc/sensor_plugins.xml"

typedef multimap<sensor_type_t, sensor_hal*> sensor_hal_plugins;
/*
* a hal_plugins is a group of hal plugin
* <HAL>
* ...
* </HAL>
*
*/

typedef multimap<sensor_type_t, sensor_base*> sensor_plugins;
/*
* a sensor_plugins is a group of sensor plugin
* <SENSOR>
* ...
* </SENSOR>
*
*/

typedef vector<sensor_fusion*> fusion_plugins;
/*
* a fusion_plugins is a group of fusion plugin
* <FUSION>
* ...
* </FUSION>
*
*/

class sensor_plugin_loader
{
private:
	sensor_plugin_loader();

	void* load_module(const char* path);
	bool insert_module(const char* node_name, const char* path);
	void show_sensor_info(void);

	sensor_hal_plugins m_sensor_hals;
	sensor_plugins m_sensors;
	fusion_plugins m_fusions;

public:
	static sensor_plugin_loader& get_instance()
	{
		static sensor_plugin_loader inst;
		return inst;
	}
	bool load_plugins(const string& plugins_path = PLUGINS_FILE_PATH);

	sensor_hal* get_sensor_hal(sensor_type_t type);
	vector<sensor_hal *> get_sensor_hals(sensor_type_t type);

	sensor_base* get_sensor(sensor_type_t type);
	vector<sensor_base *> get_sensors(sensor_type_t type);

	vector<sensor_base*> get_virtual_sensors(void);

	sensor_fusion* get_fusion(void);
	vector<sensor_fusion *> get_fusions(void);

	bool destroy();
};
#endif	/* _SENSOR_PLUGIN_LOADER_CLASS_H_ */

