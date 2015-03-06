/*
 * libsensord-share
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

#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_

#include <sensors_hal.h>

#define DEFAULT_WAIT_TIME 0

class sensor_hal
{
public:
	virtual bool initialize(void *data = NULL) = 0;
	virtual bool enable(void) = 0;
	virtual bool disable(void) = 0;
	virtual bool set_handle(int handle) = 0;
	virtual bool get_fd(int &fd) = 0;
	virtual bool get_info(sensor_info_t &properties) = 0;
	virtual int get_sensor_data(sensor_data_t &data) = 0;
	virtual bool set_command(unsigned int cmd, long val) = 0;
	virtual bool batch(int flags, unsigned long long interval_ns, unsigned long long max_report_latency_ns) = 0;
	virtual bool flush(void) = 0;
};
#endif /*_SENSOR_HAL_CLASS_H_*/
