/*
 * sensorctl
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
#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>
#include <sensorctl_log.h>
#include <functional>
#include <tester_auto.h>

static void test_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{

}

static void test_func(const char *name, std::function<bool()> func)
{
	bool ret = func();

	/*
	if (ret) PRINT(GRN "[PASS] %s", name);
	else PRINT(RED "[FAIL] %s", name);
	*/
}

static bool operate_sensor(sensor_type_t type)
{
	sensor_t sensor = sensord_get_sensor(type);
	return false;
}

static bool start_sensor(sensor_t sensor)
{
	return false;
}

static bool read_sensor(sensor_t sensor)
{
	return false;
}

static bool is_supported(sensor_type_t type, sensor_t sensor)
{
	return false;
}

bool tester_auto::init(void)
{
	return true;
}

bool tester_auto::test(sensor_type_t type, int option_count, char *options[])
{
	/*
	sensor_t *list = sensord_get_sensors(ALL_SENSOR);
	for (int i = 0; i< ARRAY_SIZE(list); ++i) {
		sensor_type_t type = sensord_get_type(list[i]);
		*/
	sensor_t sensor = sensord_get_sensor(ACCELEROMETER_SENSOR);
	test_func("SENSOR_NAME", std::bind(operate_sensor, ACCELEROMETER_SENSOR));
	test_func("SENSOR_NAME", std::bind(start_sensor, sensor));
	test_func("SENSOR_NAME", std::bind(read_sensor, sensor));
	test_func("SENSOR_NAME", std::bind(is_supported, ACCELEROMETER_SENSOR, sensor));
	//}
}
