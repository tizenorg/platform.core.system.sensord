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

#ifdef _GRAVITY_SENSOR_H

sensor_data<float> gravity_sensor::get_gravity(const sensor_data<float> accel,
				const sensor_data<float> gyro, const sensor_data<float> magnetic)
{
	return comp_grav.orientation2gravity(accel, gyro, magnetic);
}

#endif
