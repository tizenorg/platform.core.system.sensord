/*
 * sensord
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#ifndef _fusion_H
#define _fusion_H

#include <sensor_hal_types.h>

typedef enum fusion_type {
    FUSION_TYPE_ACCEL_GYRO_MAG,
    FUSION_TYPE_ACCEL_GYRO,
    FUSION_TYPE_ACCEL_MAG,
} fusion_type;

class fusion {
public:
	fusion() {};
	fusion(fusion_type FUSION_TYPE) {};
	virtual ~fusion() {} ;

	virtual bool push_accel(sensor_data_t &data) = 0;
	virtual bool push_gyro(sensor_data_t &data) = 0;
	virtual bool push_mag(sensor_data_t &data) = 0;
	virtual bool get_rv(unsigned long long timestamp, float &w, float &x, float &y, float &z) = 0;
};



#endif /* _fusion_H */
