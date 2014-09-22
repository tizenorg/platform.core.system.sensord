/*
 * libsensord
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

#ifndef _SENSOR_H_
#define _SENSOR_H_

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>

/*header for common sensor type*/
#include <sensor_common.h>

/*header for each sensor type*/
#include <sensor_accel.h>
#include <sensor_geomag.h>
#include <sensor_light.h>
#include <sensor_proxi.h>
#include <sensor_motion.h>
#include <sensor_gyro.h>
#include <sensor_pressure.h>
#include <sensor_gravity.h>
#include <sensor_linear_accel.h>
#include <sensor_orientation.h>
#include <sensor_context.h>

typedef struct {
	condition_op_t cond_op;
	float cond_value1;
} event_condition_t;

typedef struct {
	size_t event_data_size;
	void *event_data;
} sensor_event_data_t;

typedef void (*sensor_callback_func_t)(unsigned int, sensor_event_data_t *, void *);

typedef struct {
	int x;
	int y;
	int z;
} sensor_panning_data_t;

typedef struct {
	int sensor_unit_idx;
	float sensor_min_range;
	float sensor_max_range;
	float sensor_resolution;
} sensor_data_properties_t;

/**
 * @fn int sf_is_sensor_event_available(sensor_type_t desired_sensor_type, unsigned int desired_event_type)
 * @brief This API loads the in parameter available list with the type of sensor available for operation . Some of the supported types are ACCELEROMETER_SENSOR_TYPE, GEOMAGNETIC_SENSOR_TYPE etc. This API will return 0 when it is available and negative value when it does not available.
 * @param[in] desired_sensor_type your desired sensor type to check
 * @param[in] desired_event_type your desired event type to check, if you want to check only sensor-available , set "0" value
 * @return if it succeed, it return zero value(available), otherwise negative value return (not available)
 */
int sf_is_sensor_event_available(sensor_type_t desired_sensor_type, unsigned int desired_event_type);

/**
 * @fn int sf_get_data_properties(unsigned data_id, sensor_dada_properties_t *return_data_properties)
 * @brief This API loads the properties of data ID like unit of sensor data, max/min range of sensor data etc to the output parameter sensor_data_properties.
 * @param[in] data_id your desired data ID
 * @param[out] return_data_properties property information of your desired data ID
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_get_data_properties(unsigned int data_id, sensor_data_properties_t *return_data_properties);

/**
 * @fn int sf_get_properties(sensor_type_t sensor, sensor_properties_t *return_properties)
 * @brief This API loads the properties of sensor type like unit of sensor data, max/min range of sensor data etc to the output parameter sensor_properties.
 * @param[in] sensor your desired sensor type
 * @param[out] return_properties property information of your desired sensor
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_get_properties(sensor_type_t sensor, sensor_properties_t *return_properties);

/**
 * @fn int sf_set_property(sensor_type_t sensor, unsigned int property_id, long value)
 * @brief This API set the property of sensor type like calibration, enable wakeup event, etc
 * @param[in] sensor your desired sensor type, property_id your desired property ID, value for property input
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_set_property(sensor_type_t sensor, unsigned int property_id, long value);

/**
 * @fn int sf_connect(sensor_type_t sensor)
 * @brief  This API connects a sensor type to respective sensor. The application calls with the type of the sensor (ex. ACCELEROMETER_SENSOR) and on basis of that server takes decision of which plug-in to be connected. Once sensor connected application can proceed for data processing. This API returns a positive handle which should be used by application to communicate on sensor type.
 * @param[in] sensor your desired sensor type
 * @return if it succeed, it return handle value( >=0 ), otherwise negative value return
 */
int sf_connect(sensor_type_t sensor);

/**
 * @fn int sf_disconnect(int handle)
 * @brief This API disconnects an attached sensor from an application. Application must use the handle retuned after attaching the sensor. After detaching, the corresponding handle will be released.
 * @param[in] handle received handle value by sf_connect()
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_disconnect(int handle);

/**
 * @fn int sf_start(int handle, int option)
 * @brief This API sends a start command to sensor server. This intimates server that the client side is ready to handle data and start processing. The parameter option should be '0' for current usages.
 * @param[in] handle received handle value by sf_connect()
 * @param[in] option With SENSOR_OPTION_DEFAULT, it stops to sense when LCD is off, and with SENSOR_OPTION_ALWAYS_ON, it continues to sense even when LCD is off
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_start(int handle, int option);

/**
 * @fn int sf_stop(int handle)
 * @brief This API sends a stop command to the Sensor server indicating that the data processing is stopped from application side for this time.
 * @param[in] handle received handle value by sf_connect()
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_stop(int handle);

/**
 * @fn int sf_register_event(int handle, unsigned int event_type, event_conditon_t *event_condition, sensor_callback_func_t cb, void *cb_data)
 * @brief This API registers a user defined callback function with a connected sensor for a particular event. This callback function will be called when there is a change in the state of respective sensor. cb_data will be the parameter used during the callback call. Callback interval can be adjusted using even_contion_t argument.
 * @param[in] handle received handle value by sf_connect()
 * @param[in] event_type your desired event_type to register it
 * @param[in] event_condition input event_condition for special event. if you want to register without event_condition, just use a NULL value
 * @param[in] cb your define callback function
 * @param[in] cb_data	your option data that will be send when your define callback function called. if you don't have any option data, just use a NULL value
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_register_event(int handle, unsigned int event_type, event_condition_t *event_condition, sensor_callback_func_t cb, void *cb_data);

/**
 * @fn int sf_unregister_event(int handle, unsigned int event_type)
 * @brief This API de-registers a user defined callback function with a sensor registered with the specified handle. After unsubscribe, no event will be sent to the application.
 * @param[in] handle received handle value by sf_connect()
 * @param[in] event_type your desired event_type that you want to unregister event
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_unregister_event(int handle, unsigned int event_type);

/**
 * @fn int sf_get_data(int handle, unsigned int data_id, sensor_data_t* values)
 * @brief This API gets raw data from a sensor with connecting the sensor-server. The type of sensor is supplied and return data is stored in the output parameter values [].
 * @param[in] handle received handle value by sf_connect()
 * @param[in] data_id predefined data_ID as every sensor in own header - sensor_xxx.h, enum xxx_data_id {}
 * @param[out] values return values
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_get_data(int handle, unsigned int data_id, sensor_data_t *values);

/**
 * @fn int sf_check_rotation( unsigned long *curr_state)
 * @brief  This API used to get the current rotation state. (i.e. ROTATION_EVENT_0, ROTATION_EVENT_90, ROTATION_EVENT_180 & ROTATION_EVENT_270 ). This API will directly access the sensor without connection process with the sensor-server. Result will be stored in the output parameter state.
 * @param[out] curr_state it will return enum accelerometer_rotate_state value
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_check_rotation(unsigned long *curr_state);

/**
 * @fn int sf_change_event_condition(int handle, unsigned int event_type, event_condition_t *event_condition)
 * @brief This API change a user defined callback function condition with a sensor registered with the specified handle.
 * @param[in] handle received handle value by sf_connect()
 * @param[in] event_type your desired event_type that you want to unregister event
 * @param[in] event_condition your desired event condition that you want to change event
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_change_event_condition(int handle, unsigned int event_type, event_condition_t *event_condition);

/**
 * @fn int sf_change_sensor_option(int handle, int option)
 * @brief This API change sensor option .
 * @param[in] handle received handle value by sf_connect()
 * @param[in] option your desired option that you want to turn on sensor during LCD OFF
 * @return if it succeed, it return zero value, otherwise negative value return
 */
int sf_change_sensor_option(int handle, int option);

/**
 * @fn int sf_send_sensorhub_data(int handle, const char* buffer, int data_len)
 * @brief This API sends data to sensorhub.
 * @param[in] handle received handle by sf_connect()
 * @param[in] data it holds data to send to sensorhub
 * @param[in] data_len the length of data
 * @return if it succeed, it returns zero, otherwise negative value
 */
int sf_send_sensorhub_data(int handle, const char *data, int data_len);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*_SENSOR_H_*/
