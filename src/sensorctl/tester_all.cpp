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
	return (RUN_ALL_TESTS()==0) ? true : false;
}

static void basic_cb(sensor_t sensor, unsigned int event_type, sensor_data_t *data, void *user_data)
{
	tester_common::exit_loop();
	EXPECT_GT(data->timestamp, 0);
}

TEST(ACCEL, accelerometer_scenario_basic_p)
{
	tester_common tester;
	EXPECT_TRUE(tester.scenario_basic_p(ACCELEROMETER_SENSOR, 20, 100, basic_cb, NULL));
}

TEST(GYRO, gyroscope_scenario_basic_p)
{
	tester_common tester;
	EXPECT_TRUE(tester.scenario_basic_p(GYROSCOPE_SENSOR, 20, 100, basic_cb, NULL));
}

REGISTER_TESTER(ALL_SENSOR, "auto", tester_all)
