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

#pragma once // _WRISTUP_INJECTOR_H_

#include <glib.h>
#include <gio/gio.h>
#include "injector.h"

class injector_dbus: public injector_interface {
public:
	injector_dbus();
	virtual ~injector_dbus();

	bool init(sensor_type_t type, int argc, char *argv[]);
	bool inject();
private:
	bool dbus_init();

	int event;
	GDBusConnection *m_connection;
};
