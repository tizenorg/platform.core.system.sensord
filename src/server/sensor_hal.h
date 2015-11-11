/*
 * libsensord-share
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

#ifndef _SENSOR_HAL_H_
#define _SENSOR_HAL_H_

typedef enum {
	SENSOR_HAL_TYPE_ACCELEROMETER,
	SENSOR_HAL_TYPE_GEOMAGNETIC,
	SENSOR_HAL_TYPE_LIGHT,
	SENSOR_HAL_TYPE_PROXIMITY,
	SENSOR_HAL_TYPE_GYROSCOPE,
	SENSOR_HAL_TYPE_PRESSURE,
	SENSOR_HAL_TYPE_CONTEXT,
	SENSOR_HAL_TYPE_BIO,
	SENSOR_HAL_TYPE_BIO_HRM,
	SENSOR_HAL_TYPE_PIR,
	SENSOR_HAL_TYPE_PIR_LONG,
	SENSOR_HAL_TYPE_TEMPERATURE,
	SENSOR_HAL_TYPE_HUMIDITY,
	SENSOR_HAL_TYPE_ULTRAVIOLET,
	SENSOR_HAL_TYPE_DUST,
	SENSOR_HAL_TYPE_BIO_LED_IR,
	SENSOR_HAL_TYPE_BIO_LED_RED,
	SENSOR_HAL_TYPE_BIO_LED_GREEN,
	SENSOR_HAL_TYPE_RV_RAW,
	SENSOR_HAL_TYPE_GYROSCOPE_UNCAL,
	SENSOR_HAL_TYPE_GEOMAGNETIC_UNCAL,
	SENSOR_HAL_TYPE_FUSION,
} sensor_hal_type_t;



class sensor_hal
{
public:
	sensor_hal(){};
	virtual ~sensor_hal(){};

	virtual std::string get_model_id(void) = 0;
	virtual sensor_hal_type_t get_type(void) = 0;
	virtual bool enable(void) = 0;
	virtual bool disable(void) = 0;
	virtual bool is_data_ready(void) = 0;
	virtual bool set_interval(unsigned long val) = 0;
	virtual int get_sensor_data(sensor_data_t &data) = 0;
	virtual bool get_properties(sensor_properties_s &properties) = 0;
};
#endif /*_SENSOR_HAL_H_*/
