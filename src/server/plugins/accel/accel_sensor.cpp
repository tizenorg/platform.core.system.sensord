/*
 * sensord
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

#include <sensor_common.h>
#include <sensor_logs.h>
#include <accel_sensor.h>

accel_sensor::accel_sensor()
{
	_E("accel_sensor is created : 0x%x", this);
}

accel_sensor::~accel_sensor()
{

}

sensor_type_t accel_sensor::get_type(void)
{
	return ACCELEROMETER_SENSOR;
}
