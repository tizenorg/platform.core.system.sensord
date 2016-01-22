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

#ifndef _PHYSICAL_SENSOR_H_
#define _PHYSICAL_SENSOR_H_

#include <sensor_base.h>
#include <sf_common.h>
#include <worker_thread.h>

class physical_sensor : public sensor_base
{
public:
	physical_sensor();
	virtual ~physical_sensor();

	/* setting module */
	void set_sensor_handle(sensor_handle_t handle);
	void set_sensor_device(sensor_device *device);

	/* module info */
	virtual sensor_type_t get_type(void);
	virtual unsigned int get_event_type(void);
	virtual const char* get_name(void);

	int get_poll_fd();

	/* get data */
	bool is_data_ready(void);
	virtual int get_sensor_data(sensor_data_t &data);
	virtual int get_sensor_event(sensor_event_t **event);

private:
	sensor_handle_t m_handle;
	sensor_device *m_sensor_device;

	virtual bool set_interval(unsigned long interval);
	virtual bool set_wakeup(int wakeup);
	virtual bool set_batch(unsigned long latency);
	virtual bool on_start(void);
	virtual bool on_stop(void);
	virtual long set_command(unsigned int cmd, long value);
	virtual bool get_properties(sensor_properties_s &properties);
};

#endif /* _PHYSICAL_SENSOR_H_ */
