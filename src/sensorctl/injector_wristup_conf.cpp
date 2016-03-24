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
#include <glib.h>
#include <gio/gio.h>
#include "log.h"
#include "dbus_util.h"
#include "injector.h"

#define WRISTUP_CONF_SIGNAL			"conf"

class injector_wristup_conf: public injector_interface {
public:
	virtual ~injector_wristup_conf() {}

	bool inject(int argc, char *argv[]);
};

bool injector_wristup_conf::inject(int argc, char *argv[])
{
	GVariant *variant;

	RETVM_IF(option_count == 0, false, "ERROR: invalid argument");

	variant = sensorctl::make_variant_int(argc - ARGC_BASE, &argv[ARGC_BASE]);
	RETVM_IF(!variant, false, "ERROR: cannot make variant");

	sensorctl::dbus_emit_signal(NULL,
			(gchar *)SENSORD_OBJ_PATH,
			(gchar *)SENSORD_INTERFACE_NAME,
			(gchar *)WRISTUP_CONF_SIGNAL,
			variant,
			NULL);

	_I("set options to wristup:");
	for (int i = 0; i < argc - ARGC_BASE; ++i)
		_I("option %d: %s\n", i, argv[i]);

	return true;
}

REGISTER_INJECTOR(GESTURE_WRIST_UP_SENSOR, "conf", injector_wristup_conf)

