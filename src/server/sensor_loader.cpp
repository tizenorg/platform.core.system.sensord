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

#include <sensor_loader.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <sensor_hal.h>
#include <sensor_base.h>
#include <physical_sensor.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sensor_logs.h>
#include <unordered_set>
#include <algorithm>

#include <accel_sensor.h>
#ifdef ENABLE_AUTO_ROTATION
#include <auto_rotation_sensor.h>
#endif

using std::vector;
using std::string;

#define DEVICE_PLUGINS_DIR_PATH "/usr/lib/sensor"
#define SENSOR_TYPE_SHIFT 32
#define SENSOR_INDEX_MASK 0xFFFFFFFF

sensor_loader::sensor_loader()
{
}

sensor_loader& sensor_loader::get_instance()
{
	static sensor_loader inst;
	return inst;
}

bool sensor_loader::load_hal(const string &path, vector<sensor_device_t> &devices, void* &handle)
{
	int size;
	sensor_device_t *_devices = NULL;

	_I("load device: [%s]", path.c_str());

	void *_handle = dlopen(path.c_str(), RTLD_NOW);

	if (!_handle) {
		_E("Failed to dlopen(%s), dlerror : %s", path.c_str(), dlerror());
		return false;
	}

	dlerror();

	create_t create_devices = (create_t) dlsym(_handle, "create");

	if (!create_devices) {
		_E("Failed to find symbols in %s", path.c_str());
		dlclose(_handle);
		return false;
	}

	size = create_devices(&_devices);

	if (!_devices) {
		_E("Failed to create devices, path is %s\n", path.c_str());
		dlclose(_handle);
		return false;
	}

	for (int i = 0; i < size; ++i)
		devices.push_back(_devices[i]);

	handle = _handle;

	delete _devices;
	return true;
}

template <typename _sensor>
sensor_base* sensor_loader::create_sensor(void)
{
	sensor_base *instance = NULL;

	try {
		instance = new _sensor;
	} catch (std::exception &e) {
		_E("Failed to create sensor, exception: %s", e.what());
		return NULL;
	} catch (int err) {
		_E("Failed to create sensor err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	return instance;
}

template<typename _sensor>
void sensor_loader::load_physical_sensor(sensor_type_t type)
{
	int32_t index;
	const sensor_handle_t *handle;
	physical_sensor *sensor;
	sensor_device *device;

	sensor_device_map_t::iterator it = m_devices.begin();

	for (sensor_device_map_t::iterator it = m_devices.begin(); it != m_devices.end(); ++it) {
		handle = it->first;
		device = it->second;
		if (m_devices[handle] == NULL)
			continue;

		if (type != UNKNOWN_SENSOR) {
			if (type != (sensor_type_t)(handle->type))
				continue;
		}

		sensor_base *object = create_sensor<_sensor>();
		sensor = reinterpret_cast<physical_sensor *>(object);

		if (!sensor) {
			_E("Memory allocation failed[%s]", handle->name);
			return;
		}

		index = (int32_t) (m_sensors.count(type));

		sensor->set_id(((int64_t)handle->type << SENSOR_TYPE_SHIFT) | index);
		sensor->set_sensor_handle(handle);
		sensor->set_sensor_device(device);

		std::shared_ptr<sensor_base> sensor_ptr(static_cast<sensor_base *>(sensor));
		m_sensors.insert(std::make_pair(type, sensor_ptr));

		_I("loaded [%s] sensor", sensor->get_name());

		m_devices[handle] = NULL;
	}
	return;
}

bool sensor_loader::load_devices(std::vector<sensor_device_t> &devices)
{
	int size;
	sensor_device *device;
	const sensor_handle_t *handles;

	for (sensor_device_t device_ptr : devices) {
		device = static_cast<sensor_device *>(device_ptr);

		size = device->get_sensors(&handles);

		for (int i = 0; i < size; ++i)
			m_devices[&handles[i]] = device;
	}

	return true;
}

template <typename _sensor>
void sensor_loader::load_virtual_sensor(const char *name)
{
	sensor_type_t type;
	int16_t index;
	sensor_base *instance;

	instance = create_sensor<_sensor>();
	if (!instance) {
		_E("Memory allocation failed[%s]", name);
		return;
	}

	std::shared_ptr<sensor_base> sensor(instance);
	type = sensor->get_type();
	index = (int16_t)(m_sensors.count(type));

	sensor->set_id((int64_t)type << SENSOR_TYPE_SHIFT | index);

	m_sensors.insert(std::make_pair(type, sensor));

	_I("inserted [%s] sensor", sensor->get_name());
}

void sensor_loader::load_physical_sensors(void)
{
	load_physical_sensor<accel_sensor>(ACCELEROMETER_SENSOR);

	load_physical_sensor<physical_sensor>();
}

void sensor_loader::load_virtual_sensors(void)
{
	load_virtual_sensor<auto_rotation_sensor>("Auto Rotation");
}

bool sensor_loader::load_sensors(void)
{
	std::vector<sensor_device_t> devices;
	std::vector<string> device_plugin_paths;
	std::vector<string> unique_device_plugin_paths;

	get_paths_from_dir(string(DEVICE_PLUGINS_DIR_PATH), device_plugin_paths);

	std::unordered_set<string> s;
	auto unique = [&s](vector<string> &paths, const string &path) {
		if (s.insert(path).second)
			paths.push_back(path);
	};

	for_each(device_plugin_paths.begin(), device_plugin_paths.end(),
		[&](const string &path) {
			unique(unique_device_plugin_paths, path);
		}
	);

	for_each(unique_device_plugin_paths.begin(), unique_device_plugin_paths.end(),
		[&](const string &path) {
			void *handle;
			load_hal(path, devices, handle);
			load_devices(devices);
		}
	);

	load_physical_sensors();
	load_virtual_sensors();

	show_sensor_info();

	devices.clear();
	return true;
}

void sensor_loader::show_sensor_info(void)
{
	_I("========== Loaded sensor information ==========\n");

	int index = 0;

	auto it = m_sensors.begin();

	while (it != m_sensors.end()) {
		sensor_base *sensor = it->second.get();

		sensor_info info;
		sensor->get_sensor_info(info);
		_I("No:%d [%s]\n", ++index, sensor->get_name());
		info.show();
		it++;
	}

	_I("===============================================\n");
}

bool sensor_loader::get_paths_from_dir(const string &dir_path, vector<string> &plugin_paths)
{
	DIR *dir = NULL;
	struct dirent *dir_entry = NULL;

	dir = opendir(dir_path.c_str());

	if (!dir) {
		_E("Failed to open dir: %s", dir_path.c_str());
		return false;
	}

	string name;

	while ((dir_entry = readdir(dir))) {
		name = string(dir_entry->d_name);
		plugin_paths.push_back(dir_path + "/" + name);
	}

	closedir(dir);
	return true;
}

sensor_base* sensor_loader::get_sensor(sensor_type_t type)
{
	auto it_plugins = m_sensors.find(type);

	if (it_plugins == m_sensors.end())
		return NULL;

	return it_plugins->second.get();
}

sensor_base* sensor_loader::get_sensor(sensor_id_t id)
{
	vector<sensor_base *> sensors;

	sensor_type_t type = static_cast<sensor_type_t> (id >> SENSOR_TYPE_SHIFT);
	unsigned int index = (id & SENSOR_INDEX_MASK);

	sensors = get_sensors(type);

	if (index >= sensors.size())
		return NULL;

	return sensors[index];
}

vector<sensor_type_t> sensor_loader::get_sensor_types(void)
{
	vector<sensor_type_t> sensor_types;

	auto it = m_sensors.begin();

	while (it != m_sensors.end()) {
		sensor_types.push_back((sensor_type_t)(it->first));
		it = m_sensors.upper_bound(it->first);
	}

	return sensor_types;
}

vector<sensor_base *> sensor_loader::get_sensors(sensor_type_t type)
{
	vector<sensor_base *> sensor_list;
	std::pair<sensor_map_t::iterator, sensor_map_t::iterator> ret;

	if ((int)(type) == (int)SENSOR_DEVICE_ALL)
		ret = std::make_pair(m_sensors.begin(), m_sensors.end());
	else
		ret = m_sensors.equal_range(type);

	for (auto it = ret.first; it != ret.second; ++it)
		sensor_list.push_back(it->second.get());

	return sensor_list;
}

vector<sensor_base *> sensor_loader::get_virtual_sensors(void)
{
	vector<sensor_base *> virtual_list;
	sensor_base* sensor;

	for (auto sensor_it = m_sensors.begin(); sensor_it != m_sensors.end(); ++sensor_it) {
		sensor = sensor_it->second.get();

		if (sensor && sensor->is_virtual() == true) {
			virtual_list.push_back(sensor);
		}
	}

	return virtual_list;
}
