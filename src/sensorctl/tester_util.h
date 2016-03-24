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

#pragma once // _TEST_MANAGER_H_

#include <sensor_internal.h>
#include <sensorctl_log.h>
#include "tester.h"
#include "sensor_manager.h"

#define REGISTER_TESTER(sensor_type, sensor_name, tester_type) \
static void __attribute__((constructor)) reg_tester(void) \
{ \
	struct tester_info info; \
	info.type = (sensor_type); \
	info.name = (sensor_name); \
	info.tester = new(std::nothrow) (tester_type)(); \
	if (!info.tester) { \
		_E("ERROR: Failed to allocate memory(%s)", #tester_type); \
		return; \
	} \
	tester_manager::register_tester(info); \
}

struct tester_info {
	sensor_type_t type;
	const char *name;
	tester_interface *tester;
};

