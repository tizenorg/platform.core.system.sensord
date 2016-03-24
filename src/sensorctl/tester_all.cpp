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
#include <sensorctl_log.h>
#include "gtest/gtest.h"

#include "tester.h"
#include "tester_manager.h"
#include "tester_common.h"

class tester_all : public tester_interface {
public:
	bool test(sensor_type_t type, int option_count, char *options[]);
};

bool tester_all::test(sensor_type_t type, int option_count, char *options[])
{
	::testing::InitGoogleTest(&option_count, options);
	RUN_ALL_TESTS();
	return true;
}

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	tester_common::exit_loop();
	EXPECT_GT(data->timestamp, 0);
}

static void scenario_basic_p(sensor_type_t type)
{
	tester_common tester;

	if (tester.is_supported(type))
		EXPECT_TRUE(tester.scenario_basic_p(type, 20, 100, basic_cb, NULL));
}

TEST(ACCEL, accelerometer_scenario_basic_p)
{
	scenario_basic_p(ACCELEROMETER_SENSOR);
}

TEST(GYRO, gyroscope_scenario_basic_p)
{
	scenario_basic_p(GYROSCOPE_SENSOR);
}

TEST(GRAVITY, gravity_scenario_basic_p)
{
	scenario_basic_p(GRAVITY_SENSOR);
}

TEST(LINEAR_ACCEL, linear_accel_scenario_basic_p)
{
	scenario_basic_p(LINEAR_ACCEL_SENSOR);
}

TEST(PROXI, proximity_scenario_basic_p)
{
	scenario_basic_p(PROXIMITY_SENSOR);
}

TEST(PRESSURE, pressure_scenario_basic_p)
{
	scenario_basic_p(PRESSURE_SENSOR);
	EXPECT_TRUE(false);
}

TEST(HRM, hrm_scenario_basic_p)
{
	scenario_basic_p(HRM_SENSOR);
	EXPECT_TRUE(false);
}

TEST(HRM_RAW, hrm_raw_scenario_basic_p)
{
	scenario_basic_p(HRM_RAW_SENSOR);
}

TEST(HRM_LED_GREEN, hrm_led_green_scenario_basic_p)
{
	scenario_basic_p(HRM_LED_GREEN_SENSOR);
}

TEST(WRIST_UP, wrist_up_scenario_basic_p)
{
	scenario_basic_p(GESTURE_WRIST_UP_SENSOR);
}

REGISTER_TESTER(ALL_SENSOR, "auto", tester_all)
