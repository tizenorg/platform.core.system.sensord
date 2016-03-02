/*
 * sensord
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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

#include <virtual_sensor.h>
#include <sensor_event_dispatcher.h>


virtual_sensor::virtual_sensor()
: m_hardware_fusion(false)
{

}

virtual_sensor::~virtual_sensor()
{

}

bool virtual_sensor::init(void)
{
	return false;
}

bool virtual_sensor::is_virtual(void)
{
	return true;
}

bool virtual_sensor::activate(void)
{
	return sensor_event_dispatcher::get_instance().add_active_virtual_sensor(this);
}

bool virtual_sensor::deactivate(void)
{
	return sensor_event_dispatcher::get_instance().delete_active_virtual_sensor(this);
}
