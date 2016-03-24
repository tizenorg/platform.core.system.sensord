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

#include <sensor_internal.h>
#include "tester.h"

class tester_manual : public tester {
public:
	tester_manual(sensor_type_t sensor_type, const char *event_name);
	bool test(sensor_type_t type, int argc, char *argv[]);
	bool test_sensor(sensor_type_t type, int interval, int latency, int cnt_event, int cnt_test);
};
