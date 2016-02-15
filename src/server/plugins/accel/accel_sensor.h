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

#ifndef _ACCEL_SENSOR_H_
#define _ACCEL_SENSOR_H_

#include <physical_sensor.h>

class accel_sensor : public physical_sensor {
public:
	accel_sensor();
	~accel_sensor();

	sensor_type_t get_type(void);
};

#endif /* _ACCEL_SENSOR_H_ */

