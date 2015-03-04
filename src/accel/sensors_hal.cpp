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

#include <sensors_hal.h>

static MySensor *sensor = NULL;

static int sensor_initialize(struct sensors_module_t *dev)
{

}

static int sensor_get_sensor_infos(struct sensors_device_t *dev, sensor_info_t **infos, int *count)
{

}

static int sensor_set_handle(struct sensors_device_t *dev, sensor_info_t info, int handle)
{

}

static int sensor_enable(struct sensors_device_t *ev, int handle, int enabled)
{

}

static int sensor_set_interval(struct sensors_device_t *dev, int handle, unsigned long long interval_ns)
{

}

static int sensor_get_fd(struct sensors_device_t *dev, int handle, int *fd)
{

}

static int sensor_get_sensor_data(struct sensors_device_t *dev, int handle, sensor_data_t *data)
{

}

static int sensor_batch(struct sensors_device_t *dev, int handle, int flags, unsigned long long interval_ns, unsigned long long max_report_latency_ns)
{

}

static int sensor_flush(struct sensors_device_t *dev, int handle)
{

}

API int create(const struct sensors_device_t **dev, int *count)
{
	struct sensors_device_t *dev = new sensors_device_t();

	dev->enable = sensor_enable;
	dev->initialize = sensor_initialize;
	dev->get_sensor_infos = sensor_get_sensor_infos;
	dev->set_handle = sensor_set_handle;
	dev->enable = sensor_enable;
	dev->set_interval = sensor_set_interval;
	dev->get_fd = sensor_get_fd;
	dev->get_sensor_data = sensor_get_sensor_data;
	dev->batch = sensor_batch;
	dev->flush = sensor_flush;

	return 0;
}
