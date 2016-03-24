/*
 * sensorctl
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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

#include <sensor_internal.h>

#include "log.h"
#include "tester.h"

#define SENSOR_EVENT(type) ((type) << 16 | 0x1)

class tester_auto : public tester {
public:
	tester_auto(sensor_type_t sensor_type, const char *event_name);
	bool test(sensor_type_t type, int option_count, char *options[]);
};

tester_auto::tester_auto(sensor_type_t sensor_type, const char *event_name)
: tester(sensor_type, event_name)
{
}

bool tester_auto::test(sensor_type_t type, int option_count, char *options[])
{
	tester_option::show_full_log(true);
	tester_bench::run_all_testcase();
	return true;
}

REGISTER_TESTER(ALL_SENSOR, "auto", tester_auto)

static bool is_supported(sensor_type_t type)
{
	sensor_t sensor;

	sensord_get_default_sensor(type, &sensor);
	if (!sensor)
		return false;

	return true;
}

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_IF_TEXT((data->timestamp > 0), "auto.callback");
	mainloop::stop();
}

static bool scenario_basic_p(sensor_type_t type, sensor_cb_t cb)
{
	sensor_t sensor;
	int handle;
	int error;
	bool result;
	int interval = 100;
	int latency = 1000;

	error = sensord_get_default_sensor(type, &sensor);
	ASSERT_IF_TEXT((error == 0), "auto.sensord_get_default_sensor");

	handle = sensord_connect(sensor);
	ASSERT_IF_TEXT((handle >= 0), "auto.sensord_connect");

	result = sensord_register_event(handle, SENSOR_EVENT(type), interval, latency, cb, NULL);
	ASSERT_IF_TEXT((result == true), "auto.sensord_register_event");

	result = sensord_start(handle, SENSOR_OPTION_ALWAYS_ON);
	ASSERT_IF_TEXT((result == true), "auto.sensord_start");

	sensor_data_t data;
	result = sensord_get_data(handle, SENSOR_EVENT(type), &data);
	EXPECT_IF_TEXT((result == true), "auto.sensord_get_data");

	result = sensord_flush(handle);
	EXPECT_IF_TEXT((result == true), "auto.sensord_flush");

	mainloop::run();

	result = sensord_unregister_event(handle, SENSOR_EVENT(type));
	ASSERT_IF_TEXT((result == true), "auto.sensord_unregister_event");

	result = sensord_stop(handle);
	ASSERT_IF_TEXT((result == true), "auto.sensord_stop");

	result = sensord_disconnect(handle);
	ASSERT_IF_TEXT((result == true), "auto.sensord_disconnect");

	return true;
}

TESTCASE(all_sensor_scenario_basic_p)
{
	int error;
	bool result;
	sensor_t *sensors;
	int count;
	sensor_type_t type;

	error = sensord_get_sensors(ALL_SENSOR, &sensors, &count);
	ASSERT_IF(error == 0);

	for (int i = 0; i < count; ++i) {
		sensord_get_type(sensors[i], &type);

		result = scenario_basic_p(type, basic_cb);
		EXPECT_IF(result == true);
	}

	return true;
}

#if 0
TESTCASE(ALL, all_sensor_scenario_interval_10ms_p)
{
	int error;
	bool result;
	sensor_t *sensors;
	int count;

	error = sensord_get_sensors(ALL_SENSOR, &sensors, &count);
	ASSERT_EQ(error, 0);

	for (int i = 0; i < count; ++i) {
		sensord_get_type(sensors[i], &type);

		result = testcase_interval_10ms_p(sensors[i]);
		EXPECT_IF(result == true);
	}
}

static void accel_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	EXPECT_IF_TEXT((data->timestamp > 0), "auto.callback");
	EXPECT_IF_TEXT((data->values[0] < -19.6), "auto.callback");
	EXPECT_IF_TEXT((data->values[1] < -19.6), "auto.callback");
	EXPECT_IF_TEXT((data->values[2] < -19.6), "auto.callback");
	EXPECT_IF_TEXT((data->values[0] > 19.6), "auto.callback");
	EXPECT_IF_TEXT((data->values[1] > 19.6), "auto.callback");
	EXPECT_IF_TEXT((data->values[2] > 19.6), "auto.callback");

	mainloop::stop();
}

TESTCASE(accelerometer_value_p)
{
	scenario_basic_p(ACCELEROMETER_SENSOR, accel_cb);
}

TESTCASE(gyroscope_value_p)
{
	scenario_basic_p(GYROSCOPE_SENSOR);
}

TESTCASE(gravitye_value_p)
{
	scenario_basic_p(GRAVITY_SENSOR);
}

TESTCASE(linear_accel_value_p)
{
	value_p(LINEAR_ACCEL_SENSOR);
}

TESTCASE(proximity_value_p)
{
	value_p(PROXIMITY_SENSOR);
}

TESTCASE(pressure_value_p)
{
	value_p(PRESSURE_SENSOR);
}

TESTCASE(hrm_value_p)
{
	value_p(HRM_SENSOR);
}

TESTCASE(hrm_raw_value_p)
{
	value_p(HRM_RAW_SENSOR);
}

TESTCASE(hrm_led_green_value_p)
{
	value_p(HRM_LED_GREEN_SENSOR);
}

TESTCASE(wrist_up_value_p)
{
	value_p(GESTURE_WRIST_UP_SENSOR);
}
#endif
