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

#include <stdio.h>
#include <sensor_internal.h>
#include <glib.h>
#include <unistd.h>

int handle;

int main(int argc, char* argv[])
{
	int result;
	event_condition_t event_condition;

	printf("start test!\n");

	handle = sf_connect(GEOMAGNETIC_SENSOR);

	result = sf_start(handle, 1);

	if (result < 0) {
		printf("can't start Geo SENSOR\n");
		return -1;
         }
	else
		printf("Started Geo Sensor\n");

	sensor_data_t values;

	while(1)
	{
		result = sf_get_data(handle, GEOMAGNETIC_BASE_DATA_SET, &values);
		if (result < 0) {
			printf("Failed to read data\n");
			return -1;
		}
		else
			printf("Geomagnetic : [%f] [%f] [%f]\n", values.values[0], values.values[1], values.values[2]);
		usleep(100000);
	}

	result = sf_disconnect(handle);

	if (result < 0) 
		printf("can't disconnect Geo SENSOR\n");			

	return 0;
}
