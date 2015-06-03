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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <common.h>
#include <sf_common.h>
#include <ambient_temperature_sensor.h>
#include <sensor_plugin_loader.h>
#include <cvirtual_sensor_config.h>

#define SENSOR_NAME						"AMBIENT_TEMPERATURE_SENSOR"
#define SENSOR_TYPE_AMBIENT_TEMPERATURE	"AMBIENT_TEMPERATURE"

#define INITIAL_VALUE -1

#define MS_TO_US 1000
#define MIN_DELIVERY_DIFF_FACTOR 0.75f

#define PI 3.141593
#define AZIMUTH_OFFSET_DEGREES 360
#define AZIMUTH_OFFSET_RADIANS (2 * PI)

#define ELEMENT_NAME											"NAME"
#define ELEMENT_VENDOR											"VENDOR"
#define ELEMENT_RAW_DATA_UNIT									"RAW_DATA_UNIT"
#define ELEMENT_DEFAULT_SAMPLING_TIME							"DEFAULT_SAMPLING_TIME"
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

ambient_temperature_sensor::ambient_temperature_sensor()
: m_temperature_sensor(NULL)
, m_engine(NULL)
{
	cvirtual_sensor_config &config = cvirtual_sensor_config::get_instance();

	sensor_hal *fusion_sensor_hal = sensor_plugin_loader::get_instance().get_sensor_hal(FUSION_SENSOR);
	if (!fusion_sensor_hal)
		m_hardware_fusion = false;
	else
		m_hardware_fusion = true;

	m_name = string(SENSOR_NAME);
	register_supported_event(TEMPERATURE_RAW_DATA_EVENT);

	if (!config.get(SENSOR_TYPE_AMBIENT_TEMPERATURE, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_AMBIENT_TEMPERATURE, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_AMBIENT_TEMPERATURE, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	m_interval = m_default_sampling_time * MS_TO_US;
}

ambient_temperature_sensor::~ambient_temperature_sensor()
{
	INFO("ambient_temperature_sensor is destroyed!\n");
}

bool ambient_temperature_sensor::init(void)
{
	m_temperature_sensor = sensor_plugin_loader::get_instance().get_sensor(TEMPERATURE_SENSOR);

	if (!m_temperature_sensor) {
		ERR("Failed to load sensors,  temp: 0x%x",
			m_temperature_sensor);
		return false;
	}

	INFO("%s is created!\n", sensor_base::get_name());

	return load_engine();
}

bool ambient_temperature_sensor::load_engine(void)
{
	// - load library
	dlerror();
	void* lib = dlopen(SENSIRION_COMP_ENGINE_LIBRARY, RTLD_NOW);
	if (!lib) {
		const char* error = dlerror();
		ERR("Failed to load library: %s", error);
		return false;
	}

	// - verify proper API version
	FP_GET_ENGINE_INTERFACE_VERSION fpGetLibraryId =
		(FP_GET_ENGINE_INTERFACE_VERSION) dlsym(
			lib, FP_GET_ENGINE_INTERFACE_VERSION_NAME);
	const char* error = dlerror();
	if (error != NULL) {
		ERR("Failed to load symbol: %s");
		return false;
	}

	const char* libraryId = fpGetLibraryId();
	if (strcmp(libraryId, SENSIRION_COMP_ENGINE_INTERFACE_VERSION) != 0) {
		ERR("Library ID mismatch: loaded: %s, linked: %s",
		      libraryId, SENSIRION_COMP_ENGINE_INTERFACE_VERSION);
		return false;
	}

	// - instantiate the engine
	FP_LOAD_COMP_ENGINE fpLoadEngine = (FP_LOAD_COMP_ENGINE) dlsym(
		lib, FP_LOAD_COMP_ENGINE_NAME);
	error = dlerror();
	if (error != NULL) {
		ERR("Failed to load symbol: %s", error);
		return false;
	}

	m_engine = fpLoadEngine();
	if (m_engine == NULL) {
		ERR("Error instantiating compensation engine");
		return false;
	} else {
		INFO("Loaded engine '%s", m_engine->getEngineVersion());
	}
	return true;
}

sensor_type_t ambient_temperature_sensor::get_type(void)
{
	return THERMOMETER_SENSOR;
}

bool ambient_temperature_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);

	m_temperature_sensor->add_client(ACCELEROMETER_RAW_DATA_EVENT);
	m_temperature_sensor->add_interval((intptr_t)this, (m_interval/MS_TO_US), false);
	m_temperature_sensor->start();

	activate();
	return true;
}

bool ambient_temperature_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);

	m_temperature_sensor->delete_client(ACCELEROMETER_RAW_DATA_EVENT);
	m_temperature_sensor->delete_interval((intptr_t)this, false);
	m_temperature_sensor->stop();

	deactivate();
	return true;
}

bool ambient_temperature_sensor::add_interval(int client_id, unsigned int interval)
{
	AUTOLOCK(m_mutex);

	m_temperature_sensor->add_interval(client_id, interval, false);
	return sensor_base::add_interval(client_id, interval, false);
}

bool ambient_temperature_sensor::delete_interval(int client_id)
{
	AUTOLOCK(m_mutex);

	m_temperature_sensor->delete_interval(client_id, false);
	return sensor_base::delete_interval(client_id, false);
}

void ambient_temperature_sensor::compensate(unsigned long long timestamp, float &temperature)
{
	if (m_engine != NULL) {
		double compensated_temperature;
		if (m_engine->compensate(timestamp, temperature, &compensated_temperature) == 0) {
			temperature = compensated_temperature;
		}
	}
}

void ambient_temperature_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	sensor_event_t ambient_temperature_event;
	if (event.event_type == TEMPERATURE_RAW_DATA_EVENT) {

		float temperature = event.data.values[0];
		compensate(event.data.timestamp, temperature);
		ambient_temperature_event.sensor_id = get_id();
		ambient_temperature_event.event_type = TEMPERATURE_RAW_DATA_EVENT;
		ambient_temperature_event.data.accuracy = event.data.accuracy;
		ambient_temperature_event.data.timestamp = event.data.timestamp;
		ambient_temperature_event.data.value_count = 1;
		ambient_temperature_event.data.values[0] = temperature;
		push(ambient_temperature_event);
	}

	return;
}

int ambient_temperature_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	if (event_type != TEMPERATURE_RAW_DATA_EVENT)
		return -1;

	int ret = m_temperature_sensor->get_sensor_data(TEMPERATURE_RAW_DATA_EVENT, data);
	if (ret < 0)
		return -1;
	float temperature = data.values[0];
	compensate(data.timestamp, temperature);

//	data.accuracy = fusion_data.accuracy;
//	data.timestamp = get_timestamp();
//	data.value_count = 3;
	data.values[0] = temperature;
	return 0;
}

bool ambient_temperature_sensor::get_properties(sensor_properties_s &properties)
{
	properties.min_range = -20;
	properties.max_range = 80;
	properties.resolution = 0.001;
	properties.vendor = m_vendor;
	properties.name = SENSOR_NAME;
	properties.min_interval = 1;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;

	return true;
}

extern "C" sensor_module* create(void)
{
	ambient_temperature_sensor *sensor;

	try {
		sensor = new(std::nothrow) ambient_temperature_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
