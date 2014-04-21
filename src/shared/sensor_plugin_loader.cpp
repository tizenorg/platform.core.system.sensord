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
#include <sensor_fusion.h>
#include <dlfcn.h>
#include <common.h>

using std::make_pair;

#define ROOT_ELEMENT "PLUGIN"
#define TEXT_ELEMENT "text"
#define PATH_ATTR "path"
#define HAL_ELEMENT "HAL"
#define SENSOR_ELEMENT "SENSOR"

sensor_plugin_loader::sensor_plugin_loader()
{
}

void *sensor_plugin_loader::load_module(const char *path)
{
	void *handle = dlopen(path, RTLD_NOW);

	if (!handle) {
		DBG("Target file is %s , dlerror : %s", path, dlerror());
		return NULL;
	}

	dlerror();

	typedef void *create_t(void);
	typedef void destroy_t(void *);
	create_t *init_module = (create_t *) dlsym(handle, "create");
	const char *dlsym_error = dlerror();

	if (dlsym_error) {
		ERR("Failed to find \"create\" %s", dlsym_error);
		dlclose(handle);
		return NULL;
	}

	destroy_t *exit_module = (destroy_t *) dlsym(handle, "destroy");
	dlsym_error = dlerror();

	if (dlsym_error) {
		ERR("Failed to find \"destroy\" %s", dlsym_error);
		dlclose(handle);
		return NULL;
	}

	void *module = init_module();

	if (!module) {
		ERR("Failed to init the module => dlerror : %s , Target file is %s", dlerror(), path);
		dlclose(handle);
		return NULL;
	}

	return module;
}

bool sensor_plugin_loader::insert_module(const char *node_name, const char *path)
{
	if (strcmp(node_name, HAL_ELEMENT) == 0) {
		DBG("insert sensor plugin [%s]", path);
		sensor_hal *module;
		module = (sensor_hal *)load_module(path);

		if (!module)
			return false;

		sensor_type_t sensor_type = module->get_type();
		m_sensor_hals.insert(make_pair(sensor_type, module));
	}

	if (strcmp(node_name, SENSOR_ELEMENT) == 0) {
		DBG("insert sensor plugin [%s]", path);
		sensor_base *module;
		module = (sensor_base *)load_module(path);

		if (!module)
			return false;

		if (!module->init()) {
			ERR("Failed to init [%s] module", module->get_name());
			delete module;
			return false;
		}

		DBG("init [%s] module", module->get_name());
		sensor_type_t sensor_type = module->get_type();

		if (module->is_fusion())
			m_fusions.push_back((sensor_fusion *) module);
		else
			m_sensors.insert(make_pair(sensor_type, module));
	}

	return true;
}

bool sensor_plugin_loader::load_plugins(const string &plugins_path)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	DBG("sensor_plugin_load::load_plugins(\"%s\") is called!", plugins_path.c_str());
	doc = xmlParseFile(plugins_path.c_str());

	if (doc == NULL) {
		ERR("There is no %s", plugins_path.c_str());
		return false;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		ERR("There is no root element in %s", plugins_path.c_str());
		xmlFreeDoc(doc);
		return false;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *)ROOT_ELEMENT)) {
		ERR("Wrong type document: there is no [%s] root element in %s", ROOT_ELEMENT, plugins_path.c_str());
		xmlFreeDoc(doc);
		return false;
	}

	xmlNodePtr plugin_list_node_ptr;
	xmlNodePtr module_node_ptr;
	char *prop = NULL;
	plugin_list_node_ptr = cur->xmlChildrenNode;

	while (plugin_list_node_ptr != NULL) {
		//skip garbage element, [text]
		if (!xmlStrcmp(plugin_list_node_ptr->name, (const xmlChar *)TEXT_ELEMENT)) {
			plugin_list_node_ptr = plugin_list_node_ptr->next;
			continue;
		}

		DBG("<%s>", (const char *)plugin_list_node_ptr->name);

		module_node_ptr = plugin_list_node_ptr->xmlChildrenNode;

		while (module_node_ptr != NULL) {
			if (!xmlStrcmp(module_node_ptr->name, (const xmlChar *)TEXT_ELEMENT)) {
				module_node_ptr = module_node_ptr->next;
				continue;
			}

			string path;
			prop = (char *)xmlGetProp(module_node_ptr, (const xmlChar *)PATH_ATTR);
			path = prop;
			free(prop);
			DBG("<%s path=\"%s\">", (const char *) module_node_ptr->name, path.c_str());
			bool error = insert_module((const char *) plugin_list_node_ptr->name, path.c_str());

			if (!error) {
				//ERR("Fail to insert module : [%s]", path.c_str()) ;
			}

			DBG("");
			module_node_ptr = module_node_ptr->next;
		}

		DBG("");
		plugin_list_node_ptr = plugin_list_node_ptr->next;
	}

	xmlFreeDoc(doc);
	show_sensor_info();
	return true;
}

void sensor_plugin_loader::show_sensor_info(void)
{
	int index = 0;
	sensor_plugins::iterator it = m_sensors.begin();

	INFO("========== Loaded sensor information ==========");

	while (it != m_sensors.end()) {
		sensor_base *sensor = it->second;
		sensor_properties_t properties;
		int default_type = sensor->get_type() << SENSOR_TYPE_SHIFT | 0x1;

		if (sensor->get_properties(default_type, properties)) {
			INFO("[%d] %s", ++index, sensor->get_name());
			INFO("name : %s", properties.sensor_name);
			INFO("vendor : %s", properties.sensor_vendor);
			INFO("unit_idx : %d", properties.sensor_unit_idx);
			INFO("min_range : %f", properties.sensor_min_range);
			INFO("max_range : %f", properties.sensor_max_range);
			INFO("resolution : %f", properties.sensor_resolution);
		}

		it++;
	}

	INFO("===============================================");
}

sensor_hal *sensor_plugin_loader::get_sensor_hal(sensor_type_t type)
{
	sensor_hal_plugins::iterator it_plugins;
	it_plugins = m_sensor_hals.find(type);

	if (it_plugins == m_sensor_hals.end())
		return NULL;

	return it_plugins->second;
}

vector<sensor_hal *> sensor_plugin_loader::get_sensor_hals(sensor_type_t type)
{
	vector<sensor_hal *> sensor_hal_list;
	pair<sensor_hal_plugins::iterator, sensor_hal_plugins::iterator> ret;
	ret = m_sensor_hals.equal_range(type);
	sensor_hal_plugins::iterator it;

	for (it = ret.first; it != ret.second; ++it) {
		sensor_hal_list.push_back(it->second);
	}

	return sensor_hal_list;
}

sensor_base *sensor_plugin_loader::get_sensor(sensor_type_t type)
{
	sensor_plugins::iterator it_plugins;
	it_plugins = m_sensors.find(type);

	if (it_plugins == m_sensors.end())
		return NULL;

	return it_plugins->second;
}

vector<sensor_base *> sensor_plugin_loader::get_sensors(sensor_type_t type)
{
	vector<sensor_base *> sensor_list;
	pair<sensor_plugins::iterator, sensor_plugins::iterator> ret;
	ret = m_sensors.equal_range(type);
	sensor_plugins::iterator it;

	for (it = ret.first; it != ret.second; ++it) {
		sensor_list.push_back(it->second);
	}

	return sensor_list;
}

vector<sensor_base *> sensor_plugin_loader::get_virtual_sensors(void)
{
	vector<sensor_base *> virtual_list;
	sensor_plugins::iterator sensor_it;
	sensor_base *module;

	for (sensor_it = m_sensors.begin(); sensor_it != m_sensors.end(); ++sensor_it) {
		module = sensor_it->second;

		if (module && module->is_virtual() == true) {
			virtual_list.push_back(module);
		}
	}

	return virtual_list;
}

sensor_fusion *sensor_plugin_loader::get_fusion(void)
{
	if (m_fusions.empty())
		return NULL;

	return m_fusions.front();
}

vector<sensor_fusion *> sensor_plugin_loader::get_fusions(void)
{
	return m_fusions;
}

bool sensor_plugin_loader::destroy()
{
	sensor_base *sensor;
	sensor_plugins::iterator sensor_it;

	for (sensor_it = m_sensors.begin(); sensor_it != m_sensors.end(); ++sensor_it) {
		sensor = sensor_it->second;
		delete sensor;
	}

	sensor_hal *sensor_hal;
	sensor_hal_plugins::iterator sensor_hal_it;

	for (sensor_hal_it = m_sensor_hals.begin(); sensor_hal_it != m_sensor_hals.end(); ++sensor_hal_it) {
		sensor_hal = sensor_hal_it->second;
		delete sensor_hal;
	}

	m_sensors.clear();
	m_sensor_hals.clear();
	return true;
}

