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

#pragma once // _INJECTOR_H_

#include <sensor_internal.h>

#define SENSORD_BUS_NAME 		"org.tizen.system.sensord"
#define SENSORD_OBJ_PATH		"/Org/Tizen/System/SensorD"
#define SENSORD_INTERFACE_NAME	"org.tizen.system.sensord"

class injector_interface {
public:
	injector_interface() {}
	virtual ~injector_interface() {}

	virtual bool init(void) { return true; }
	virtual bool inject(int option_count, char *options[]) = 0;
};
