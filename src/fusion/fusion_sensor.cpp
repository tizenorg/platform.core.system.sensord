/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include <fusion_sensor.h>
#include <sensor_plugin_loader.h>
#include <orientation_filter.h>
#include <cvirtual_sensor_config.h>

#define SENSOR_NAME "FUSION_SENSOR"
#define SENSOR_TYPE_FUSION		"FUSION"

#define ACCELEROMETER_ENABLED 0x01
#define GYROSCOPE_ENABLED 0x02
#define GEOMAGNETIC_ENABLED 0x04
#define GAMING_RV_ENABLED 3
#define GEOMAGNETIC_RV_ENABLED 5
#define ORIENTATION_ENABLED 7
#define ROTATION_VECTOR_ENABLED 7

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
#define ELEMENT_ACCEL_STATIC_BIAS								"ACCEL_STATIC_BIAS"
#define ELEMENT_GYRO_STATIC_BIAS								"GYRO_STATIC_BIAS"
#define ELEMENT_GEOMAGNETIC_STATIC_BIAS							"GEOMAGNETIC_STATIC_BIAS"
#define ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION			"ACCEL_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_GYRO_ROTATION_DIRECTION_COMPENSATION			"GYRO_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_GEOMAGNETIC_ROTATION_DIRECTION_COMPENSATION		"GEOMAGNETIC_ROTATION_DIRECTION_COMPENSATION"
#define ELEMENT_ACCEL_SCALE										"ACCEL_SCALE"
#define ELEMENT_GYRO_SCALE										"GYRO_SCALE"
#define ELEMENT_GEOMAGNETIC_SCALE								"GEOMAGNETIC_SCALE"
#define ELEMENT_MAGNETIC_ALIGNMENT_FACTOR						"MAGNETIC_ALIGNMENT_FACTOR"
#define ELEMENT_PITCH_ROTATION_COMPENSATION						"PITCH_ROTATION_COMPENSATION"
#define ELEMENT_ROLL_ROTATION_COMPENSATION						"ROLL_ROTATION_COMPENSATION"
#define ELEMENT_AZIMUTH_ROTATION_COMPENSATION					"AZIMUTH_ROTATION_COMPENSATION"

void pre_process_data(sensor_data<float> &data_out, const float *data_in, float *bias, int *sign, float scale)
{
	data_out.m_data.m_vec[0] = sign[0] * (data_in[0] - bias[0]) / scale;
	data_out.m_data.m_vec[1] = sign[1] * (data_in[1] - bias[1]) / scale;
	data_out.m_data.m_vec[2] = sign[2] * (data_in[2] - bias[2]) / scale;
}

fusion_sensor::fusion_sensor()
: m_accel_sensor(NULL)
, m_gyro_sensor(NULL)
, m_magnetic_sensor(NULL)
, m_time(0)
{
	cvirtual_sensor_config &config = cvirtual_sensor_config::get_instance();

	m_name = string(SENSOR_NAME);
	register_supported_event(FUSION_ORIENTATION_EVENT);
	register_supported_event(FUSION_ROTATION_VECTOR_EVENT);
	register_supported_event(FUSION_GEOMAGNETIC_ROTATION_VECTOR_EVENT);
	register_supported_event(FUSION_GAMING_ROTATION_VECTOR_EVENT);
	register_supported_event(FUSION_CALIBRATION_NEEDED_EVENT);
	m_enable_fusion = 0;

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_VENDOR, m_vendor)) {
		ERR("[VENDOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_vendor = %s", m_vendor.c_str());

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_RAW_DATA_UNIT, m_raw_data_unit)) {
		ERR("[RAW_DATA_UNIT] is empty\n");
		throw ENXIO;
	}

	INFO("m_raw_data_unit = %s", m_raw_data_unit.c_str());

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_DEFAULT_SAMPLING_TIME, &m_default_sampling_time)) {
		ERR("[DEFAULT_SAMPLING_TIME] is empty\n");
		throw ENXIO;
	}

	INFO("m_default_sampling_time = %d", m_default_sampling_time);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_ACCEL_STATIC_BIAS, m_accel_static_bias, 3)) {
		ERR("[ACCEL_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	INFO("m_accel_static_bias = (%f, %f, %f)", m_accel_static_bias[0], m_accel_static_bias[1], m_accel_static_bias[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GYRO_STATIC_BIAS, m_gyro_static_bias,3)) {
		ERR("[GYRO_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	INFO("m_gyro_static_bias = (%f, %f, %f)", m_gyro_static_bias[0], m_gyro_static_bias[1], m_gyro_static_bias[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GEOMAGNETIC_STATIC_BIAS, m_geomagnetic_static_bias, 3)) {
		ERR("[GEOMAGNETIC_STATIC_BIAS] is empty\n");
		throw ENXIO;
	}

	INFO("m_geomagnetic_static_bias = (%f, %f, %f)", m_geomagnetic_static_bias[0], m_geomagnetic_static_bias[1], m_geomagnetic_static_bias[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_ACCEL_ROTATION_DIRECTION_COMPENSATION, m_accel_rotation_direction_compensation, 3)) {
		ERR("[ACCEL_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_accel_rotation_direction_compensation = (%d, %d, %d)", m_accel_rotation_direction_compensation[0], m_accel_rotation_direction_compensation[1], m_accel_rotation_direction_compensation[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GYRO_ROTATION_DIRECTION_COMPENSATION, m_gyro_rotation_direction_compensation, 3)) {
		ERR("[GYRO_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_gyro_rotation_direction_compensation = (%d, %d, %d)", m_gyro_rotation_direction_compensation[0], m_gyro_rotation_direction_compensation[1], m_gyro_rotation_direction_compensation[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GEOMAGNETIC_ROTATION_DIRECTION_COMPENSATION, m_geomagnetic_rotation_direction_compensation, 3)) {
		ERR("[GEOMAGNETIC_ROTATION_DIRECTION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_geomagnetic_rotation_direction_compensation = (%d, %d, %d)", m_geomagnetic_rotation_direction_compensation[0], m_geomagnetic_rotation_direction_compensation[1], m_geomagnetic_rotation_direction_compensation[2]);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_ACCEL_SCALE, &m_accel_scale)) {
		ERR("[ACCEL_SCALE] is empty\n");
		throw ENXIO;
	}

	INFO("m_accel_scale = %f", m_accel_scale);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GYRO_SCALE, &m_gyro_scale)) {
		ERR("[GYRO_SCALE] is empty\n");
		throw ENXIO;
	}

	INFO("m_gyro_scale = %f", m_gyro_scale);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_GEOMAGNETIC_SCALE, &m_geomagnetic_scale)) {
		ERR("[GEOMAGNETIC_SCALE] is empty\n");
		throw ENXIO;
	}

	INFO("m_geomagnetic_scale = %f", m_geomagnetic_scale);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_MAGNETIC_ALIGNMENT_FACTOR, &m_magnetic_alignment_factor)) {
		ERR("[MAGNETIC_ALIGNMENT_FACTOR] is empty\n");
		throw ENXIO;
	}

	INFO("m_magnetic_alignment_factor = %d", m_magnetic_alignment_factor);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_AZIMUTH_ROTATION_COMPENSATION, &m_azimuth_rotation_compensation)) {
		ERR("[AZIMUTH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_azimuth_rotation_compensation = %d", m_azimuth_rotation_compensation);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_PITCH_ROTATION_COMPENSATION, &m_pitch_rotation_compensation)) {
		ERR("[PITCH_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_pitch_rotation_compensation = %d", m_pitch_rotation_compensation);

	if (!config.get(SENSOR_TYPE_FUSION, ELEMENT_ROLL_ROTATION_COMPENSATION, &m_roll_rotation_compensation)) {
		ERR("[ROLL_ROTATION_COMPENSATION] is empty\n");
		throw ENXIO;
	}

	INFO("m_roll_rotation_compensation = %d", m_roll_rotation_compensation);

	m_interval = m_default_sampling_time * MS_TO_US;

}

fusion_sensor::~fusion_sensor()
{
	INFO("fusion_sensor is destroyed!\n");
}

bool fusion_sensor::init(void)
{
	INFO("%s is created!", sensor_base::get_name());
	return true;
}

sensor_type_t fusion_sensor::get_type(void)
{
	return FUSION_SENSOR;
}

bool fusion_sensor::on_start(void)
{
	AUTOLOCK(m_mutex);
	activate();
	return true;
}

bool fusion_sensor::on_stop(void)
{
	AUTOLOCK(m_mutex);
	deactivate();
	return true;
}

bool fusion_sensor::add_interval(int client_id, unsigned int interval)
{
	bool retval;

	AUTOLOCK(m_mutex);
	retval = sensor_base::add_interval(client_id, interval, false);

	m_interval = sensor_base::get_interval(client_id, false);

	if (m_interval != 0)
		retval = true;

	return retval;
}

bool fusion_sensor::delete_interval(int client_id)
{
	bool retval;

	AUTOLOCK(m_mutex);
	retval = sensor_base::delete_interval(client_id, false);

	m_interval = sensor_base::get_interval(client_id, false);

	if (m_interval != 0)
		retval = true;

	return retval;
}

void fusion_sensor::synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs)
{
	unsigned long long diff_time;
	euler_angles<float> euler_orientation;
	float azimuth_offset;

	if (event.event_type == ACCELEROMETER_RAW_DATA_EVENT) {
		diff_time = event.data.timestamp - m_time;

		if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
			return;

		pre_process_data(m_accel, event.data.values, m_accel_static_bias, m_accel_rotation_direction_compensation, m_accel_scale);

		m_accel.m_time_stamp = event.data.timestamp;

		m_enable_fusion |= ACCELEROMETER_ENABLED;
	}

	if (is_supported(FUSION_ORIENTATION_EVENT) || is_supported(FUSION_ROTATION_VECTOR_EVENT) ||
			is_supported(FUSION_GAMING_ROTATION_VECTOR_EVENT)) {
		if (event.event_type == GYROSCOPE_RAW_DATA_EVENT) {
				diff_time = event.data.timestamp - m_time;

				if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
					return;

				pre_process_data(m_gyro, event.data.values, m_gyro_static_bias, m_gyro_rotation_direction_compensation, m_gyro_scale);

				m_gyro.m_time_stamp = event.data.timestamp;

				m_enable_fusion |= GYROSCOPE_ENABLED;
		}
	}

	if (is_supported(FUSION_ORIENTATION_EVENT) || is_supported(FUSION_ROTATION_VECTOR_EVENT) ||
			is_supported(FUSION_GEOMAGNETIC_ROTATION_VECTOR_EVENT)) {
		if (event.event_type == GEOMAGNETIC_RAW_DATA_EVENT) {
			diff_time = event.data.timestamp - m_time;

			if (m_time && (diff_time < m_interval * MIN_DELIVERY_DIFF_FACTOR))
				return;

			pre_process_data(m_magnetic, event.data.values, m_geomagnetic_static_bias, m_geomagnetic_rotation_direction_compensation, m_geomagnetic_scale);

			m_magnetic.m_time_stamp = event.data.timestamp;

			m_enable_fusion |= GEOMAGNETIC_ENABLED;
		}
	}

	if (m_enable_fusion == ORIENTATION_ENABLED && is_supported(FUSION_ORIENTATION_EVENT)) {
		sensor_event_t fusion_event;

		m_orientation_filter.m_pitch_phase_compensation = m_pitch_rotation_compensation;
		m_orientation_filter.m_roll_phase_compensation = m_roll_rotation_compensation;
		m_orientation_filter.m_azimuth_phase_compensation = m_azimuth_rotation_compensation;
		m_orientation_filter.m_magnetic_alignment_factor = m_magnetic_alignment_factor;

		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, &m_magnetic);

		euler_orientation = m_orientation_filter.m_orientation;

		if(m_raw_data_unit == "DEGREES") {
			euler_orientation = rad2deg(euler_orientation);
			azimuth_offset = AZIMUTH_OFFSET_DEGREES;
		}
		else {
			azimuth_offset = AZIMUTH_OFFSET_RADIANS;
		}

		m_time = get_timestamp();
		fusion_event.sensor_id = get_id();
		fusion_event.event_type = ORIENTATION_RAW_DATA_EVENT;
		fusion_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		fusion_event.data.timestamp = m_time;
		fusion_event.data.value_count = 3;
		fusion_event.data.values[1] = m_orientation_filter.m_orientation.m_ang.m_vec[0];
		fusion_event.data.values[2] = m_orientation_filter.m_orientation.m_ang.m_vec[1];
		if (m_orientation_filter.m_orientation.m_ang.m_vec[2] >= 0)
			fusion_event.data.values[0] = m_orientation_filter.m_orientation.m_ang.m_vec[2];
		else
			fusion_event.data.values[0] = m_orientation_filter.m_orientation.m_ang.m_vec[2] + azimuth_offset;

		push(fusion_event);
	}

	if (m_enable_fusion == ROTATION_VECTOR_ENABLED && is_supported(FUSION_ROTATION_VECTOR_EVENT)) {
		sensor_event_t fusion_event;

		m_orientation_filter.m_pitch_phase_compensation = m_pitch_rotation_compensation;
		m_orientation_filter.m_roll_phase_compensation = m_roll_rotation_compensation;
		m_orientation_filter.m_azimuth_phase_compensation = m_azimuth_rotation_compensation;
		m_orientation_filter.m_magnetic_alignment_factor = m_magnetic_alignment_factor;

		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, &m_magnetic);

		m_time = get_timestamp();
		fusion_event.sensor_id = get_id();
		fusion_event.event_type = ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME;
		fusion_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		fusion_event.data.timestamp = m_time;
		fusion_event.data.value_count = 4;
		fusion_event.data.values[0] = m_orientation_filter.m_quat_9axis.m_quat.m_vec[1];
		fusion_event.data.values[1] = m_orientation_filter.m_quat_9axis.m_quat.m_vec[2];
		fusion_event.data.values[2] = m_orientation_filter.m_quat_9axis.m_quat.m_vec[3];
		fusion_event.data.values[3] = m_orientation_filter.m_quat_9axis.m_quat.m_vec[0];

		push(fusion_event);
	}

	if (m_enable_fusion == GAMING_RV_ENABLED && is_supported(FUSION_GAMING_ROTATION_VECTOR_EVENT)) {
		sensor_event_t fusion_event;

		m_orientation_filter.get_device_orientation(&m_accel, &m_gyro, NULL);

		m_time = get_timestamp();
		fusion_event.sensor_id = get_id();
		fusion_event.event_type = GAMING_RV_RAW_DATA_EVENT;
		fusion_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		fusion_event.data.timestamp = m_time;
		fusion_event.data.value_count = 4;
		fusion_event.data.values[0] = m_orientation_filter.m_quat_gaming_rv.m_quat.m_vec[1];
		fusion_event.data.values[1] = m_orientation_filter.m_quat_gaming_rv.m_quat.m_vec[2];
		fusion_event.data.values[2] = m_orientation_filter.m_quat_gaming_rv.m_quat.m_vec[3];
		fusion_event.data.values[3] = m_orientation_filter.m_quat_gaming_rv.m_quat.m_vec[0];

		push(fusion_event);
	}

	if (m_enable_fusion == GEOMAGNETIC_RV_ENABLED && is_supported(FUSION_GEOMAGNETIC_ROTATION_VECTOR_EVENT)) {
		sensor_event_t fusion_event;

		m_orientation_filter.m_magnetic_alignment_factor = m_magnetic_alignment_factor;

		m_orientation_filter.get_device_orientation(&m_accel, NULL, &m_magnetic);

		m_time = get_timestamp();
		fusion_event.sensor_id = get_id();
		fusion_event.event_type = GEOMAGNETIC_RV_RAW_DATA_EVENT;
		fusion_event.data.accuracy = SENSOR_ACCURACY_GOOD;
		fusion_event.data.timestamp = m_time;
		fusion_event.data.value_count = 4;
		fusion_event.data.values[0] = m_orientation_filter.m_quat_aid.m_quat.m_vec[1];
		fusion_event.data.values[1] = m_orientation_filter.m_quat_aid.m_quat.m_vec[2];
		fusion_event.data.values[2] = m_orientation_filter.m_quat_aid.m_quat.m_vec[3];
		fusion_event.data.values[3] = m_orientation_filter.m_quat_aid.m_quat.m_vec[0];

		push(fusion_event);
	}

	m_enable_fusion = 0;
	return;
}

int fusion_sensor::get_sensor_data(const unsigned int event_type, sensor_data_t &data)
{
	return 0;
}

bool fusion_sensor::get_properties(sensor_properties_s &properties)
{
	properties.min_range = 0;
	properties.max_range = 0;
	properties.resolution = 0;
	properties.vendor = m_vendor;
	properties.name = SENSOR_NAME;
	properties.min_interval = 0;
	properties.fifo_count = 0;
	properties.max_batch_count = 0;

	return true;
}

extern "C" sensor_module* create(void)
{
	fusion_sensor *sensor;

	try {
		sensor = new(std::nothrow) fusion_sensor;
	} catch (int err) {
		ERR("Failed to create module, err: %d, cause: %s", err, strerror(err));
		return NULL;
	}

	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module || !sensor, NULL, "Failed to allocate memory");

	module->sensors.push_back(sensor);
	return module;
}
