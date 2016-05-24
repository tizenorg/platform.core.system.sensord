/*
 * sensord
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

#include <external_sensor.h>
#include <external_sensor_service.h>
#include <sensor_event_queue.h>
#include <command_queue.h>

using std::string;
using std::shared_ptr;
using std::make_shared;

external_sensor::external_sensor()
: m_source_connected(false)
{
}

external_sensor::~external_sensor()
{
	unregister_key();
}

bool external_sensor::register_key(const string &key)
{
	m_key = key;
	return external_sensor_service::get_instance().register_sensor(this);
}

bool external_sensor::unregister_key(void)
{
	return external_sensor_service::get_instance().unregister_sensor(this);
}

string external_sensor::get_key(void)
{
	return m_key;
}

bool external_sensor::set_source_connected(bool connected)
{
	AUTOLOCK(m_source_mutex);

	if (m_source_connected && connected) {
		_E("Source is already connected");
		return false;
	}

	m_source_connected = connected;
	return true;
}

bool external_sensor::get_source_connected(void)
{
	AUTOLOCK(m_source_mutex);

	return m_source_connected;
}

int external_sensor::set_attribute(int32_t attribute, char *value, int value_size)
{
	shared_ptr<external_command_t> external_command = make_shared<external_command_t>();

	external_command->header.sensor_id = get_id();
	external_command->header.command_len = value_size;
	external_command->command.assign(value, value + value_size);

	command_queue::get_instance().push(external_command);

	return 0;
}
