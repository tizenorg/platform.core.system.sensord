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

#ifndef _VIRTUAL_SENSOR_H_
#define _VIRTUAL_SENSOR_H_

#include <sensor_base.h>

class virtual_sensor : public sensor_base
{
public:
	virtual_sensor();
	virtual ~virtual_sensor();

	/* initialize sensor */
	virtual bool init();

	/* module info */
	virtual sensor_type_t get_type() = 0;
	virtual unsigned int get_event_type(void) = 0;
	virtual const char* get_name(void) = 0;

	virtual bool get_sensor_info(sensor_info &info) = 0;

	/* synthesize event */
	virtual void synthesize(const sensor_event_t& event) = 0;

	/* get data */
	virtual int get_data(sensor_data_t **data) = 0;

	bool is_virtual(void);

protected:
	bool activate(void);
	bool deactivate(void);

private:
	bool m_hardware_fusion;

	virtual bool set_interval(unsigned long interval) = 0;
	virtual bool set_batch_latency(unsigned long latency) = 0;
	virtual bool set_wakeup(int wakeup) = 0;

	virtual bool on_start(void) = 0;
	virtual bool on_stop(void) = 0;
};

#endif /* _VIRTUAL_SENSOR_H_ */
