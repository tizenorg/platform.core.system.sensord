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
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <sensor_internal.h>
#include "sensor_log.h"
#include "injector_dbus.h"

#define NAME_MAX_TEST 32

enum {
	DBUS_TYPE_SIGNAL,
	DBUS_TYPE_METHOD_CALL
} dbus_event_type_e;

struct dbus_event_info {
	int sensor_type;
	int dbus_type;
	char event_name[NAME_MAX_TEST];
	char bus_path[NAME_MAX_TEST];
	char object_path[NAME_MAX_TEST];
	char event_path[NAME_MAX_TEST];
	int command;
};

/* wristup sensor type is not defined yet.
 * until it is created, MOTION_SENSOR is used temporarily*/
static struct dbus_event_info dbus_events[] = {
	{MOTION_SENSOR, DBUS_TYPE_SIGNAL, "auto",
		"org.tizen.system.sensord", "/Org/Tizen/System/SensorD", "algo", 0},
	{MOTION_SENSOR, DBUS_TYPE_SIGNAL, "green",
		"org.tizen.system.sensord", "/Org/Tizen/System/SensorD", "algo", 1},
	{MOTION_SENSOR, DBUS_TYPE_SIGNAL, "purple",
		"org.tizen.system.sensord", "/Org/Tizen/System/SensorD", "algo", 2},
	{MOTION_SENSOR, DBUS_TYPE_SIGNAL, "red",
		"org.tizen.system.sensord", "/Org/Tizen/System/SensorD", "algo", 3},
	/* new event should be here */
};

injector_dbus::injector_dbus()
	: m_connection(NULL)
{
#ifndef GLIB_VERSION_2_36
	g_type_init();
#endif
	dbus_init();
}

injector_dbus::~injector_dbus()
{
	if (m_connection) {
		g_dbus_connection_close_sync(m_connection, NULL, NULL);
		g_object_unref(m_connection);
		m_connection = NULL;
	}
}

bool injector_dbus::dbus_init()
{
	GError *error = NULL;
	gchar *gaddr;

	gaddr = g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, NULL, &error);

	if (!gaddr) {
		PRINT("Failed to get dbus address : %s", error->message);
		g_error_free(error);
		error = NULL;
		return false;
	}

	m_connection = g_dbus_connection_new_for_address_sync(gaddr,
			(GDBusConnectionFlags)(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT
			| G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
			NULL, NULL, &error);
	g_free(gaddr);

	if (!m_connection) {
		PRINT("Failed to get dbus connection : %s", error->message);
		g_error_free(error);
		error = NULL;
		return false;
	}

	PRINT("G-DBUS connected[%s]\n",
			g_dbus_connection_get_unique_name(m_connection));
	return true;
}

bool injector_dbus::init(sensor_type_t type, int argc, char *argv[])
{
	int count_event = sizeof(dbus_events)/sizeof(dbus_event_info);

	for (int i=0; i<count_event; ++i) {
		if (type == dbus_events[i].sensor_type) {
			if (!strcmp(dbus_events[i].event_name, argv[3])) {
				event = i;
				return true;
			}
		}
	}

	return false;
}

/*
bool injector_dbus::inject_method_call()
{
	g_dbus_connection_call(m_connection,
			dbus_events[event].bus_path, dbus_events[event].object_path,
			dbus_events[event].bus_path, dbus_events[event].event_path,
			g_variant_new("(i)", dbus_events[event].command), NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

}
*/

bool injector_dbus::inject()
{
	g_dbus_connection_emit_signal(m_connection, NULL,
			dbus_events[event].object_path,
			dbus_events[event].bus_path,
			dbus_events[event].event_path,
			g_variant_new("(i)", dbus_events[event].command),
			NULL);

	return true;
}

