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

#include <errono.h>
#include <sensors_hal.h>

static accel_sensor *acc_sensor = NULL;

static int sensor_initialize(struct sensors_module_t *dev)
{
	acc_sensor = new accel_sensor();

	if (!sensor)
		return -ENOMEM;

	return 0;
}

static int sensor_get_sensor_infos(struct sensors_module_t *dev, sensor_info_t **infos, int *count)
{
	*infos = new sensor_info_t*[1];
	if (!acc_sensor.get_info(*infos))
		return -1;

	*count = 1;

	return 0;
}

static int sensor_set_handle(struct sensors_module_t *dev, sensor_info_t info, int handle)
{
	if (!acc_sensor.set_handle(handle))
		return -1;

	return 0;
}

static int sensor_enable(struct sensors_module_t *dev, int handle, int enabled)
{
	sensor_hal sensor = get_sensor(handle);

	if (!sensor)
		return -1;

	if (enabled) {
		if (!sensor.enable())
			return -1;
	} else {
		if (!sensor.disable())
			return -1;
	}

	return 0;
}

static int sensor_get_fd(struct sensors_module_t *dev, int handle, int *fd)
{
	sensor_hal sensor = get_sensor(handle);

	if (!sensor)
		return -1;

	if (!sensor.get_fd(fd))
		return -1;

	return 0;
}

static int sensor_get_sensor_data(struct sensors_module_t *dev, int handle, sensor_data_t *data)
{
	sensor_hal sensor = get_sensor(handle);

	if (!sensor)
		return -1;

	if (!sensor.get_sensor_data(data))
		return -1;

	return 0;
}

static int sensor_set_command(struct sensors_module_t *dev, int handle, const char *cmd)
}
	return 0;
}

static int sensor_batch(struct sensors_module_t *dev, int handle, int flags, unsigned long long interval_ns, unsigned long long max_report_latency_ns)
{
	sensor_hal sensor = get_sensor(handle);

	if (!sensor)
		return -1;

	if (!sensor.batch(flags, interval_ns, max_report_latency_ns))
		return -1;

	return 0;
}

static int sensor_flush(struct sensors_module_t *dev, int handle)
{
	sensor_hal sensor = get_sensor(handle);

	if (!sensor)
		return -1;

	if (!sensor.flush())
		return -1;

	return 0;
}

API int create(const struct sensors_module_t **dev, int *count)
{
	struct sensors_module_t *dev = new sensors_module_t();

	/*
	 * sensors_module_t can specify by handle.
	 * if there is the better way, it can be modified.
	 */
	dev->handle = 1;
	dev->enable = sensor_enable;
	dev->initialize = sensor_initialize;
	dev->get_sensor_infos = sensor_get_sensor_infos;
	dev->set_handle = sensor_set_handle;
	dev->enable = sensor_enable;
	dev->get_fd = sensor_get_fd;
	dev->get_sensor_data = sensor_get_sensor_data;
	dev->set_command = sensor_set_command;
	dev->batch = sensor_batch;
	dev->flush = sensor_flush;

	return 0;
}
