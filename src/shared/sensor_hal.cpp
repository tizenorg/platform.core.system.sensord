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

#include <sensor_hal.h>



bool sensor_hal::set_interval(unsigned long val)
{
	return true;
}

long sensor_hal::set_command(const unsigned int cmd, long val)
{
	return -1;
}

int sensor_hal::send_sensorhub_data(const char* data, int data_len)
{
	return -1;
}

int sensor_hal::get_sensor_data(sensor_data_t &data)
{
	return -1;
}

int sensor_hal::get_sensor_data(sensorhub_data_t &data)
{
	return -1;
}

unsigned long long sensor_hal::get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

unsigned long long sensor_hal::get_timestamp(timeval *t)
{
	if (!t) {
		ERR("t is NULL");
		return 0;
	}

	return ((unsigned long long)(t->tv_sec)*1000000LL +t->tv_usec);
}

