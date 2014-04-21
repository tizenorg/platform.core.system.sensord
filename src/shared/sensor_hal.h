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

#include <sys/time.h>
#include <sf_common.h>
#include <cmutex.h>
#include <common.h>
#include <sensor.h>
#include <string>

using std::string;

/*
* As of Linux 3.4, there is a new EVIOCSCLOCKID ioctl to set the desired clock
* Current kernel-headers package doesn't have it so we should define it here.
*/

#ifndef EVIOCSCLOCKID
#define EVIOCSCLOCKID		_IOW('E', 0xa0, int)			/* Set clockid to be used for timestamps */
#endif

class sensor_hal
{
public:
	sensor_hal() {}
	virtual ~sensor_hal() {}

	virtual bool init(void *data = NULL) {
		return true;
	}
	virtual string get_model_id(void) = 0;
	virtual sensor_type_t get_type(void) = 0;
	virtual bool enable(void) = 0;
	virtual bool disable(void) = 0;
	virtual bool set_interval(unsigned long val);
	virtual bool is_data_ready(bool wait) = 0;
	virtual bool get_properties(sensor_properties_t &properties) = 0;
	virtual int get_sensor_data(sensor_data_t &data);
	virtual int get_sensor_data(sensorhub_data_t &data);
	virtual long set_command(const unsigned int cmd, long val);
	virtual int send_sensorhub_data(const char *data, int data_len);
protected:
	cmutex m_mutex;

	unsigned long long get_timestamp(void);
	unsigned long long get_timestamp(timeval *t);
};
#endif /*_SENSOR_HAL_H_*/
