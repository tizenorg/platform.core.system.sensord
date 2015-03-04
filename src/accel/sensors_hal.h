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

#ifdef __cplusplus
extern "C"
{
#endif

#define SENSOR_DATA_VALUE_SIZE 16

/**
 * These APIs are used to control the sensors.
*/

typedef enum {
	ALL_SENSOR = -1,
	UNKNOWN_SENSOR = 0,
	ACCELEROMETER_SENSOR,
	GEOMAGNETIC_SENSOR,
	LIGHT_SENSOR,
	PROXIMITY_SENSOR,
	THERMOMETER_SENSOR,
	GYROSCOPE_SENSOR,
	PRESSURE_SENSOR,
	MOTION_SENSOR,
	FUSION_SENSOR,
	PEDOMETER_SENSOR,
	CONTEXT_SENSOR,
	FLAT_SENSOR,
	BIO_SENSOR,
	BIO_HRM_SENSOR,
	AUTO_ROTATION_SENSOR,
	GRAVITY_SENSOR,
	LINEAR_ACCEL_SENSOR,
	ROTATION_VECTOR_SENSOR,
	GEOMAGNETIC_RV_SENSOR,
	GAMING_RV_SENSOR,
	ORIENTATION_SENSOR,
	PIR_SENSOR,
	PIR_LONG_SENSOR,
	TEMPERATURE_SENSOR,
	HUMIDITY_SENSOR,
	ULTRAVIOLET_SENSOR,
	DUST_SENSOR,
	BIO_LED_GREEN_SENSOR,
	BIO_LED_IR_SENSOR,
	BIO_LED_RED_SENSOR,
	RV_RAW_SENSOR,
	UNCAL_GYROSCOPE_SENSOR,
	UNCAL_GEOMAGNETIC_SENSOR
} sensor_type_t;

typedef unsigned int sensor_id_t;

typedef struct sensors_data_t {
	int accuracy;
	unsigned long long timestamp;
	int value_count;
	float values[SENSOR_DATA_VALUE_SIZE];
	float reserved[8];
} sensor_data_t;

struct sensors_device_t {
	int (*initialize)(struct sensors_device_t *dev);
	int (*set_handle)(struct sensors_device_t *dev, int type, int handle);
	int (*get_type)(struct sensors_device_t *dev, int **types, int *count);
	int (*enable)(struct sensors_device_t *dev, int handle, int enabled);
	int (*set_interval)(struct sensors_device_t *dev, int handle,
			unsigned long long interval_ns);
	int (*get_fd)(struct sensors_device_t *dev, int handle, int **fd, int count);
	int (*get_sensor_data)(struct sensors_device_t *dev, int handle, sensor_data_t *data);
	int (*batch)(struct sensors_device_t *dev, int handle);
	int (*get_properties)(struct sensors_device_t *dev,
			sensor_properties_t &properties);
	int (*reserved_fp)(void);
};

API int create(const struct sensors_device_t** dev);


#endif /* _SENSORS_HAL_H_ */
