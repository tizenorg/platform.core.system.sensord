/*
 * libsensord
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
#include <client_common.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>
using std::unordered_map;

#define FILL_LOG_ELEMENT(ID, TYPE, CNT, PRINT_PER_CNT) {ID, TYPE, {#TYPE, CNT, PRINT_PER_CNT} }

log_element g_log_elements[] = {
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, UNKNOWN_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, ACCELEROMETER_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, GEOMAGNETIC_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, LIGHT_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, PROXIMITY_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, GYROSCOPE_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, PRESSURE_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, CONTEXT_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, AUTO_ROTATION_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, GRAVITY_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, LINEAR_ACCEL_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, ORIENTATION_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, TEMPERATURE_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, ROTATION_VECTOR_SENSOR, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_SENSOR_TYPE, GEOMAGNETIC_RV_SENSOR, 0, 1),

	FILL_LOG_ELEMENT(LOG_ID_EVENT, GEOMAGNETIC_CALIBRATION_NEEDED_EVENT, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, PROXIMITY_CHANGE_STATE_EVENT, 0,1),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, LIGHT_CHANGE_LEVEL_EVENT, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, PROXIMITY_STATE_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, PROXIMITY_DISTANCE_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, CONTEXT_REPORT_EVENT, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, AUTO_ROTATION_CHANGE_STATE_EVENT, 0, 1),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, ACCELEROMETER_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, GYROSCOPE_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, GEOMAGNETIC_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, PRESSURE_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, LIGHT_LEVEL_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, LIGHT_LUX_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, GRAVITY_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, LINEAR_ACCEL_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, ORIENTATION_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, PRESSURE_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, TEMPERATURE_RAW_DATA_EVENT, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, ROTATION_VECTOR_EVENT_RAW_DATA_REPORT_ON_TIME, 0, 10),
	FILL_LOG_ELEMENT(LOG_ID_EVENT, GEOMAGNETIC_RV_EVENT_RAW_DATA_REPORT_ON_TIME, 0, 10),
};

typedef unordered_map<unsigned int, log_attr* > log_map;
log_map g_log_maps[LOG_ID_END];

extern void init_client(void);
static void init_log_maps(void);


class initiator
{
public:
	initiator()
	{
		init_log_maps();
		init_client();
	}
} g_initiatior;

static void init_log_maps(void)
{
	int cnt;

	cnt = sizeof(g_log_elements) / sizeof(g_log_elements[0]);

	for (int i = 0; i < cnt; ++i) {
		g_log_maps[g_log_elements[i].id][g_log_elements[i].type] = &g_log_elements[i].log_attr;
	}

}


const char* get_log_element_name(log_id id, unsigned int type)
{
	const char* p_unknown = "UNKNOWN";

	auto iter = g_log_maps[id].find(type);

	if (iter == g_log_maps[id].end()) {
		INFO("Unknown type value: 0x%x", type);
		return p_unknown;
	}

	return iter->second->name;
}

const char* get_sensor_name(sensor_id_t sensor_id)
{
	const int SENSOR_TYPE_MASK = 0x0000FFFF;

	sensor_type_t sensor_type = (sensor_type_t) (sensor_id & SENSOR_TYPE_MASK);

	return get_log_element_name(LOG_ID_SENSOR_TYPE, sensor_type);
}

const char* get_event_name(unsigned int event_type)
{
	return get_log_element_name(LOG_ID_EVENT, event_type);
}


const char* get_data_name(unsigned int data_id)
{
	return get_log_element_name(LOG_ID_DATA, data_id);
}

bool is_one_shot_event(unsigned int event_type)
{
	return false;
}

bool is_ontime_event(unsigned int event_type)
{
	switch (event_type ) {
	case ACCELEROMETER_RAW_DATA_EVENT:
	case PROXIMITY_STATE_EVENT:
	case GYROSCOPE_RAW_DATA_EVENT:
	case LIGHT_LEVEL_DATA_EVENT:
	case GEOMAGNETIC_RAW_DATA_EVENT:
	case LIGHT_LUX_DATA_EVENT:
	case PROXIMITY_DISTANCE_DATA_EVENT:
	case GRAVITY_RAW_DATA_EVENT:
	case LINEAR_ACCEL_RAW_DATA_EVENT:
	case ORIENTATION_RAW_DATA_EVENT:
	case PRESSURE_RAW_DATA_EVENT:
		return true;
		break;
	}

	return false;
}

bool is_panning_event(unsigned int event_type)
{
	return false;
}

bool is_single_state_event(unsigned int event_type)
{
	switch (event_type) {
	case GEOMAGNETIC_CALIBRATION_NEEDED_EVENT:
	case LIGHT_CHANGE_LEVEL_EVENT:
	case PROXIMITY_CHANGE_STATE_EVENT:
	case AUTO_ROTATION_CHANGE_STATE_EVENT:
		return true;
		break;
	}

	return false;
}

unsigned int get_calibration_event_type(unsigned int event_type)
{
	sensor_type_t sensor;

	sensor = (sensor_type_t)(event_type >> SENSOR_TYPE_SHIFT);

	switch (sensor) {
	case GEOMAGNETIC_SENSOR:
		return GEOMAGNETIC_CALIBRATION_NEEDED_EVENT;
	case ORIENTATION_SENSOR:
		return ORIENTATION_CALIBRATION_NEEDED_EVENT;
	default:
		return 0;
	}
}

unsigned long long get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

void print_event_occurrence_log(csensor_handle_info &sensor_handle_info, const creg_event_info *event_info)
{
	log_attr *log_attr;

	auto iter = g_log_maps[LOG_ID_EVENT].find(event_info->type);

	if (iter == g_log_maps[LOG_ID_EVENT].end())
		return;

	log_attr = iter->second;

	log_attr->cnt++;

	if ((log_attr->cnt != 1) && ((log_attr->cnt % log_attr->print_per_cnt) != 0)) {
		return;
	}

	INFO("%s receives %s with %s[%d][state: %d, option: %d count: %d]", get_client_name(), log_attr->name,
			get_sensor_name(sensor_handle_info.m_sensor_id), sensor_handle_info.m_handle, sensor_handle_info.m_sensor_state,
			sensor_handle_info.m_sensor_option, log_attr->cnt);

	INFO("0x%x(cb_event_type = %s, &user_data, client_data = 0x%x)\n", event_info->m_cb,
			log_attr->name, event_info->m_user_data);
}

/*
 *	To prevent user mistakenly freeing sensor_info using sensor_t
 */
static const int SENSOR_TO_SENSOR_INFO = 4;
static const int SENSOR_INFO_TO_SENSOR = -SENSOR_TO_SENSOR_INFO;

sensor_info *sensor_to_sensor_info(sensor_t sensor)
{
	if (!sensor)
		return NULL;

	sensor_info* info = (sensor_info *)((char *)sensor + SENSOR_TO_SENSOR_INFO);

	return info;
}

sensor_t sensor_info_to_sensor(const sensor_info *info)
{
	if (!info)
		return NULL;

	sensor_t sensor = (sensor_t)((char *)info + SENSOR_INFO_TO_SENSOR);

	return sensor;
}
