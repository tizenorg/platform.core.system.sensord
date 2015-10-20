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

#if !defined(_CVIRTUAL_SENSOR_CONFIG_CLASS_H_)
#define _CVIRTUAL_SENSOR_CONFIG_CLASS_H_

#include <cconfig.h>

#define VIRTUAL_SENSOR_CONFIG_FILE_PATH "/usr/etc/virtual_sensors.xml"

typedef std::unordered_map<std::string,std::string> Element;
/*
* an Element  is a group of attributes
* <Element value1 = "10.0", value2 =  "20.0"/>
*
*/

typedef std::unordered_map<std::string,Element> Virtual_sensor;
/*
* a Virtual_sensor is a group of elements to consist of one virtual sensor's configuration
*	<ORIENTATION>
*		<NAME value="ORIENTATION_SENSOR"/>
*		<VENDOR value="SAMSUNG"/>
*		...
*/

typedef std::unordered_map<std::string,Virtual_sensor> virtual_sensor_config;
/*
* a Virtual_sensor_config represents virtual_sensors.xml
* <ORIENTATION/>
* <GRAVITY/>
* <LINEAR_ACCELERATION/>
*
*/

typedef std::unordered_map<std::string,virtual_sensor_config> virtual_sensor_device_config;
/*
* a virtual_sensor_device_config represents virtual_sensors.xml
* <emulator/>
* <RD_PQ/>
*
*/

class cvirtual_sensor_config : public cconfig
{
private:
	cvirtual_sensor_config();
	cvirtual_sensor_config(cvirtual_sensor_config const&) {};
	cvirtual_sensor_config& operator=(cvirtual_sensor_config const&);

	bool load_config(const std::string& config_path);

	virtual_sensor_device_config m_virtual_sensor_config;

public:
	static cvirtual_sensor_config& get_instance(void);

	bool get(const std::string& sensor_type, const std::string& element, const std::string& attr, std::string& value);
	bool get(const std::string& sensor_type, const std::string& element, const std::string& attr, float *value);
	bool get(const std::string& sensor_type, const std::string& element, const std::string& attr, int *value);

	bool get(const std::string& sensor_type, const std::string& element, std::string& value);
	bool get(const std::string& sensor_type, const std::string& element, float *value, int count =1);
	bool get(const std::string& sensor_type, const std::string& element, int *value, int count = 1);

	bool is_supported(const std::string &sensor_type);
};

#endif
