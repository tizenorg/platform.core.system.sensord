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

#ifndef _EXTERNAL_SENSOR_H_
#define _EXTERNAL_SENSOR_H_

#include <sensor_base.h>
#include <sensor_common.h>
#include <cmutex.h>
#include <string>

class external_sensor : public sensor_base {
public:
	external_sensor();
	virtual ~external_sensor();
	int send_data(const char* data, int data_cnt);
	std::string get_key(void);
	bool set_source_connected(bool connected);
	bool get_source_connected(void);
	virtual void on_receive(unsigned long long timestamp, const float* data, int data_cnt) = 0;
	virtual int set_attribute(int32_t attribute, char *value, int value_size);
protected:
	bool register_key(const std::string &key);
private:
	bool unregister_key(void);

	std::string m_key;
	bool m_source_connected;
	cmutex m_source_mutex;
};
#endif /* _EXTERNAL_SENSOR_H_ */
