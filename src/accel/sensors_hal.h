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

#ifndef _SENSORS_HAL_INTERFACE_H_
#define _SENSORS_HAL_INTERFACE_H_

#define API __attribute__((visibility("default")))

#define SENSOR_DATA_VALUE_SIZE 16

/**
 * These APIs are used to control the sensors.
 */

typedef enum {
	ALL_SENSOR = -1,
	UNKNOWN_SENSOR = 0,
	ACCELEROMETER_SENSOR,              /**< Accelerometer */
	GEOMAGNETIC_SENSOR,                /**< Gravity sensor */
	TEMPERATURE_SENSOR,
	HUMIDITY_SENSOR,
} sensor_type_t;

typedef struct sensor_data_t {
	int accuracy;
	int value_count;
	unsigned long long timestamp;
	float values[SENSOR_DATA_VALUE_SIZE];
	float reserved[8];
} sensor_data_t;

typedef struct sensor_info_t {
	const char *name;
	const char *vendor;
	const char *string_type;
	int handle;
	int type;
	float min_range;
	float max_range;
	float resolution;
	int min_interval;
	int fifo_count;
	int max_batch_count;
	void* reserved[8];
} sensor_info_t;

struct sensors_module_t {
	int handle;

	int (*initialize)(struct sensors_module_t *dev);

	int (*get_sensor_infos)(struct sensors_module_t *dev, sensor_info_t **infos, int *count);

	/* sensor must be given a handle(id)*/
	int (*set_handle)(struct sensors_module_t *dev, sensor_info_t info, int handle);

	int (*enable)(struct sensors_module_t *dev, int handle, int enabled);

	int (*get_fd)(struct sensors_module_t *dev, int handle, int *fd);

	int (*get_sensor_data)(struct sensors_module_t *dev,
			int handle, sensor_data_t *data);

	int (*batch)(struct sensors_module_t *dev, int handle, int flags,
			unsigned long long interval_ns, unsigned long long max_report_latency_ns);

	int (*flush)(struct sensors_module_t *dev, int handle);

	int (*reserved_fp[8])(void);
};

#ifdef __cplusplus
extern "C"
{
#endif

API int create(const struct sensors_module_t **dev, int *count);

#ifdef __cplusplus
}
#endif

#endif /* _SENSORS_HAL_INTERFACE_H_ */
