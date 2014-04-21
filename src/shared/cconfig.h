/*
 * libsensord-share
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
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

#if !defined(_CCONFIG_CLASS_H_)
#define _CCONFIG_CLASS_H_

#include <string>
#include <map>
#include <common.h>

using std::map;
using std::string;
using std::istringstream;

#define CONFIG_FILE_PATH "/usr/etc/sensors.xml"

typedef map<string,string> Element;
/*
* an Element  is a group of attributes
* <Element value1 = "10.0", value2 =  "20.0"/>
*
* "value" -> "LSM330DLC"
*
*/

typedef map<string,Element> Model;
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

typedef map<string,Model> Model_list;
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

typedef map<string,Model_list> Sensor_config;
/*
* a SensorConfig represents sensors.xml
* <ACCEL/>
* <GYRO/>
* <PROXIMITY/>
*
* "ACCEL" -> Model_list
*
*/

namespace config
{
	class CConfig
	{
	private:
		CConfig();
		CConfig(CConfig const&) {};
		CConfig& operator=(CConfig const&);
		bool load_config(const string& config_path = CONFIG_FILE_PATH);
		Sensor_config m_sensor_config;
		string m_device_id;
	public:
		static CConfig& get_instance(void)
		{
			static bool load_done = false;
			static CConfig inst;

			if (!load_done) {
				inst.load_config();
				inst.get_device_id();
				if (!inst.m_device_id.empty())
					INFO("Device ID = %s", inst.m_device_id.c_str());
				else
					ERR("Failed to get Device ID");
				load_done = true;
			}

			return inst;
		}
		bool get(const string& sensor_type, const string& model_id, const string& element, const string& attr, string& value);
		bool get(const string& sensor_type, const string& model_id, const string& element, const string& attr, double& value);
		bool get(const string& sensor_type, const string& model_id, const string& element, const string& attr, long& value);

		bool get(const string& sensor_type, const string& model_id, const string& element, string& value);
		bool get(const string& sensor_type, const string& model_id, const string& element, double& value);
		bool get(const string& sensor_type, const string& model_id, const string& element, long& value);

		bool is_supported(const string &sensor_type, const string &model_id);
		bool get_device_id(void);
	};
}
#endif
