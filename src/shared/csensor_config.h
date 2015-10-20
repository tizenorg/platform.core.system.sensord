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

#if !defined(_CSENSOR_CONFIG_CLASS_H_)
#define _CSENSOR_CONFIG_CLASS_H_

#include <cconfig.h>

#define SENSOR_CONFIG_FILE_PATH "/usr/etc/sensors.xml"

typedef std::unordered_map<std::string,std::string> Element;
/*
* an Element  is a group of attributes
* <Element value1 = "10.0", value2 =  "20.0"/>
*
* "value" -> "LSM330DLC"
*
*/

typedef std::unordered_map<std::string,Element> Model;
/*
* a Model is a group of elements to consist of  specific vendor's one sensor configuration
* <NAME value = "LSM330DLC" />
* <VENDOR value = "ST Microelectronics"/>
* <RAW_DATA_UNIT value = "1" />
* <RESOLUTION value = "12" />
*
* <NAME> -> <value = "LSM330DLC"/>
*
*/

typedef std::unordered_map<std::string,Model> Model_list;
/*
* a Model_list is  a group of Model
* <MODEL id = "lsm330dlc_accel">
* </MODEL>
* <MODEL id = "mpu6500">
* </MODEL>
*
* "lsm330dlc_accel" -> <Model>
*
*/

typedef std::unordered_map<std::string,Model_list> Sensor_config;
/*
* a SensorConfig represents sensors.xml
* <ACCEL/>
* <GYRO/>
* <PROXIMITY/>
*
* "ACCEL" -> Model_list
*
*/

class csensor_config : public cconfig
{
private:
	csensor_config();
	csensor_config(csensor_config const&) {};
	csensor_config& operator=(csensor_config const&);

	bool load_config(const std::string& config_path);

	Sensor_config m_sensor_config;
public:
	static csensor_config& get_instance(void);

	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, const std::string& attr, std::string& value);
	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, const std::string& attr, double& value);
	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, const std::string& attr, long& value);

	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, std::string& value);
	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, double& value);
	bool get(const std::string& sensor_type, const std::string& model_id, const std::string& element, long& value);

	bool is_supported(const std::string &sensor_type, const std::string &model_id);
};

#endif
