/*
 * sensord
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

#include <hw/common.h>

/* the id of this device */
#define SENSOR_HARDWARE_DEVICE_ID "sensor"
#define SENSOR_HARDWARE_DEVICE_VERSION MAKE_VERSION(0,1)

/**
 * Sensor types
 * These types are used to control the sensors.
 */
enum sensor_type {
	/* TODO: Meta Data */
	SENSOR_TYPE_METADATA = -2,

	/* Accelerometer Sensor*/
	SENSOR_TYPE_ACCELEROMETER = 1,

	/* Geomagnetic Sensor */
	SENSOR_TYPE_GEOMAGNETIC = 2,

	/* Temperature Sensor */
	SENSOR_TYPE_TEMPERATURE = 10,

	/* Humidity Sensor */
	SENSOR_TYPE_HUMIDITY = 11,

	/*
	 * Base for device manufacturers private sensor types.
	 * These sensor types can't be exposed in the SDK.
	 * if new sensor type should be needed, use this type.
	 */
	SENSOR_TYPE_CUSTOM = 10000,
};

/* TBD: Sensor string type */
#define SENSOR_STRING_TYPE_ACCELEROMETER "accelerometer"

/**
 * sensor event data
 *
 * - base unit
 * acceleration values  : meter per second per second (m/s^2)
 * magnetic values      : micro-Tesla (uT)
 * orientation values   : degrees
 * gyroscope values     : degrees/s
 * temperature values   : degrees centigrade
 * proximitiy values    : distance(near or far)
 * light values         : lux
 * pressure values      : hectopascal (hPa)
 * humidity             : relative humidity (%)
 * uncalibrated gyro    : degrees/s
 * uncalibrated mag     : micro-Tesla (uT)
 * heart rate monitor   : bpm/..
 */
struct sensor_data {
	/* version of structure */
	int version;

	/* sensor identifier */
	int sensor;

	/* sensor(or data) type */
	int type;

	/* accuracy */
	int accuracy;

	/* count of values */
	int value_count;

	/* timestamp */
	unsigned long long timestamp;

	/* TODO: change to size */
	float *values;

	float reserved[4];
};

struct sensor_info {
	/* Name of this sensor. */
	const char *name;

	/* vendor of the hardware part */
	const char *vendor;

	/* version of the hardware part */
	int version;

	/* handle that identifies this sensors. */
	int handle;

	/* this sensor's type. */
	int type;

	/* minimum range of this sensor's value in SI units */
	float min_range;

	/* maximum range of this sensor's value in SI units */
	float max_range;

	/* smallest difference between two values reported by this sensor */
	float resolution;

	/*
	 *   continuous: minimum sample period allowed in microseconds
	 *   on-change : 0
	 *   one-shot  :-1
	 *   special   : 0, unless otherwise noted
	 */
	int min_interval;

	/* number of events reserved for this sensor in the batch mode FIFO.
	 * If there is a dedicated FIFO for this sensor, then this is the
	 * size of this FIFO. If the FIFO is shared with other sensors,
	 * this is the size reserved for that sensor and it can be zero.
	 */
	int fifo_count;

	/* maximum number of events of this sensor that could be batched.
	 * This is especially relevant when the FIFO is shared between
	 * several sensors; this value is then set to the size of that FIFO.
	 */
	int max_batch_count;

	/* TODO: permission required to see this sensor, register to it and receive data.
	 * Set to 0 if no permission is required.
	 */
	int permission;

	/* TODO: rrough estimate of this sensor's power consumption in mA */
	float power;

	/* TODO: max delay */
	/* This value is defined only for continuous mode and on-change sensors.
	 * It is the delay between two sensor events corresponding to the lowest
	 * frequency that this sensor supports. When lower frequencies are requested
	 * through batch()/setDelay() the events will be generated at this frequency
	 * instead. It can be used by the framework or applications to estimate
	 * when the batch FIFO may be full.
	 */
	int maxDelay;

	void* reserved[7];
};

/* TODO: who gives a permission? fw or hal? */
struct sensor_module {
	/* it must be first*/
	struct hw_common common;

	/*
	 * if there is a something to do during the module is loading,
	 * initialize() is called.
	 */
	int (*initialize)(struct sensor_module *dev);

	/*
	 * Enumerate all available sensor information list of this module.
	 */
	int (*get_sensor_infos)(struct sensor_module *dev, sensor_info **infos, int *count);

	/*
	 * A Handle identifies a given sensors.
	 * The handle is used to enable/disenable sensors.
	 * This handle must be given from sensor framework
	 */
	int (*set_handle)(struct sensor_module *dev, sensor_info info, int handle);

	/*
	 * Enable/Disable one sensor by using unique handle.
	 */
	int (*enable)(struct sensor_module *dev, int handle, int enabled);

	/*
	 * fd should be provided to sensor daemon for polling the event.
	 */
	int (*get_fd)(struct sensor_module *dev, int handle, int *fd);

	/*
	 * Read the most recent sensor data.
	 */
	int (*get_sensor_data)(struct sensor_module *dev,
			int handle, sensor_data *data);

	/*
	 * refer to Android:-
	 * Sets a sensor’s parameters, including sampling frequency and maximum
	 * report latency. This function can be called while the sensor is
	 * activated, in which case it must not cause any sensor measurements to
	 * be lost: transitioning from one sampling rate to the other cannot cause
	 * lost events, nor can transitioning from a high maximum report latency to
	 * a low maximum report latency.
	 */
	int (*batch)(struct sensor_module *dev, int handle, int flags,
			unsigned long long interval_ns, unsigned long long max_report_latency_ns);

	/*
	 * refer to Android:-
	 * Flush adds a META_DATA_FLUSH_COMPLETE event (sensors_event_meta_data_t)
	 * to the end of the "batch mode" FIFO for the specified sensor and flushes
	 * the FIFO.
	 * If the FIFO is empty or if the sensor doesn't support batching (FIFO size zero),
	 * it should return SUCCESS along with a trivial META_DATA_FLUSH_COMPLETE
	 * event added to the event stream. This applies to all sensors other than
	 * one-shot sensors. If the sensor is a one-shot sensor, flush must
	 * return -EINVAL and not generate any flush complete metadata.
	 * If the sensor is not active at the time flush() is called, flush() should
	 * return -EINVAL.
	 */
	int (*flush)(struct sensor_module *dev, int handle);

	int (*reserved_fp[8])(void);
};

#endif /* _SENSORS_HAL_INTERFACE_H_ */
