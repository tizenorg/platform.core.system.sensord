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

#include <cvirtual_sensor_config.h>
#include "common.h"
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using std::ifstream;
using std::string;
using std::stringstream;

#define ROOT_ELEMENT	"VIRTUAL_SENSOR"
#define TEXT_ELEMENT 	"text"
#define MODEL_ID_ATTR 	"id"
#define DEFAULT_ATTR	"value"
#define DEFAULT_ATTR1	"value1"
#define DEFAULT_ATTR2	"value2"
#define DEFAULT_ATTR3	"value3"

cvirtual_sensor_config::cvirtual_sensor_config()
{
}

cvirtual_sensor_config& cvirtual_sensor_config::get_instance(void)
{
	static bool load_done = false;
	static cvirtual_sensor_config inst;

	if (!load_done) {
		inst.load_config(VIRTUAL_SENSOR_CONFIG_FILE_PATH);
		inst.get_device_id();
		if (!inst.m_device_id.empty())
			INFO("Device ID = %s", inst.m_device_id.c_str());
		else
			ERR("Failed to get Device ID");
		load_done = true;
	}

	return inst;
}

bool cvirtual_sensor_config::load_config(const string& config_path)
{
	xmlDocPtr doc;
	xmlNodePtr cur;

	DBG("cvirtual_sensor_config::load_config(\"%s\") is called!\n",config_path.c_str());

	doc = xmlParseFile(config_path.c_str());

	if (doc == NULL) {
		ERR("There is no %s\n",config_path.c_str());
		return false;
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL) {
		ERR("There is no root element in %s\n",config_path.c_str());
		xmlFreeDoc(doc);
		return false;
	}

	if(xmlStrcmp(cur->name, (const xmlChar *)ROOT_ELEMENT)) {
		ERR("Wrong type document: there is no [%s] root element in %s\n",ROOT_ELEMENT,config_path.c_str());
		xmlFreeDoc(doc);
		return false;
	}

	xmlNodePtr virtual_sensor_node_ptr;
	xmlNodePtr element_node_ptr;
	xmlAttrPtr attr_ptr;
	char* prop = NULL;

	virtual_sensor_node_ptr = cur->xmlChildrenNode;

	while (virtual_sensor_node_ptr != NULL) {
		//skip garbage element, [text]
		if (!xmlStrcmp(virtual_sensor_node_ptr->name,(const xmlChar *)TEXT_ELEMENT)) {
			virtual_sensor_node_ptr = virtual_sensor_node_ptr->next;
			continue;
		}

		//insert Model_list to config map
		m_virtual_sensor_config[(const char*)virtual_sensor_node_ptr->name];
		DBG("<%s>\n",(const char*)virtual_sensor_node_ptr->name);

		element_node_ptr = virtual_sensor_node_ptr->xmlChildrenNode;
		while (element_node_ptr != NULL) {
			//skip garbage element, [text]
			if (!xmlStrcmp(element_node_ptr->name,(const xmlChar *)TEXT_ELEMENT)) {
				element_node_ptr = element_node_ptr->next;
				continue;
			}

			//insert Element to Model
			m_virtual_sensor_config[(const char*)virtual_sensor_node_ptr->name][(const char*)element_node_ptr->name];
			DBG("<%s><%s>\n",(const char*)virtual_sensor_node_ptr->name,(const char*)element_node_ptr->name);

			attr_ptr = element_node_ptr->properties;
			while (attr_ptr != NULL) {

				string key,value;
				key = (char*)attr_ptr->name;
				prop = (char*)xmlGetProp(element_node_ptr,attr_ptr->name);
				value = prop;
				free(prop);

				//insert attribute to Element
				m_virtual_sensor_config[(const char*)virtual_sensor_node_ptr->name][(const char*)element_node_ptr->name][key]=value;
				DBG("<%s><%s \"%s\"=\"%s\">\n",(const char*)virtual_sensor_node_ptr->name,(const char*)element_node_ptr->name,key.c_str(),value.c_str());
				attr_ptr = attr_ptr->next;
			}


			element_node_ptr = element_node_ptr->next;
		}

		DBG("\n");
		virtual_sensor_node_ptr = virtual_sensor_node_ptr->next;
	}

	xmlFreeDoc(doc);
	return true;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, const string& attr, string& value)
{
	auto it_virtual_sensor_list = m_virtual_sensor_config.find(sensor_type);

	if (it_virtual_sensor_list == m_virtual_sensor_config.end())	{
		ERR("There is no <%s> element\n",sensor_type.c_str());
		return false;
	}

	auto it_element = it_virtual_sensor_list->second.find(element);

	if (it_element == it_virtual_sensor_list->second.end()) {
		ERR("There is no <%s><%s> element\n",sensor_type.c_str(),element.c_str());
		return false;
	}

	auto it_attr = it_element->second.find(attr);

	if (it_attr == it_element->second.end()) {
		DBG("There is no <%s><%s \"%s\">\n",sensor_type.c_str(),element.c_str(),attr.c_str());
		return false;
	}

	value = it_attr->second;

	return true;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, const string& attr, float *value)
{
	string str_value;

	if (get(sensor_type,element,attr,str_value) == false)
		return false;

	stringstream str_stream(str_value);

	str_stream >> *value;

	return true;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, const string& attr, int *value)
{
	string str_value;

	if (get(sensor_type,element,attr,str_value) == false)
		return false;

	stringstream str_stream(str_value);

	str_stream >> *value;

	return true;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, string& value)
{
	if (get(sensor_type, element, DEFAULT_ATTR, value))
		return true;

	return false;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, float *value, int count)
{
	if (count == 1)
	{
		if (get(sensor_type, element, DEFAULT_ATTR, value))
			return true;
	}
	else if (count == 3)
	{
		if (!get(sensor_type, element, DEFAULT_ATTR1, value))
			return false;

		value++;

		if (!get(sensor_type, element, DEFAULT_ATTR2, value))
			return false;

		value++;

		if (!get(sensor_type, element, DEFAULT_ATTR3, value))
			return false;

		return true;
	}
	else
	{
		DBG("Count value not supported.\n");
	}

	return false;
}

bool cvirtual_sensor_config::get(const string& sensor_type, const string& element, int *value, int count)
{
	if (count == 1)
	{
		if (get(sensor_type, element, DEFAULT_ATTR, value))
			return true;
	}
	else if (count == 3)
	{
		if (!get(sensor_type, element, DEFAULT_ATTR1, value))
			return false;

		value++;

		if (!get(sensor_type, element, DEFAULT_ATTR2, value))
			return false;

		value++;

		if (!get(sensor_type, element, DEFAULT_ATTR3, value))
			return false;

		return true;
	}
	else
	{
		DBG("Count value not supported.\n");
	}

	return false;
}

bool cvirtual_sensor_config::is_supported(const string& sensor_type)
{
	auto it_virtual_sensor_list = m_virtual_sensor_config.find(sensor_type);

	if (it_virtual_sensor_list == m_virtual_sensor_config.end())
		return false;

	return true;
}

bool cvirtual_sensor_config::get_device_id(void)
{
	const string INFO_INI_PATH = "/etc/info.ini";
	const string START_DELIMETER = "Model=";
	const string END_DELIMETER = ";";
	string line;
	ifstream in_file;
	std::size_t start_pos, end_pos;
	bool ret = false;

	in_file.open(INFO_INI_PATH);

	if (!in_file.is_open())
		return false;

	while (!in_file.eof()) {
		getline(in_file, line);
		start_pos = line.find(START_DELIMETER);

		if (start_pos != std::string::npos) {
			start_pos = start_pos + START_DELIMETER.size();
			end_pos = line.find(END_DELIMETER, start_pos);

			if (end_pos != std::string::npos) {
				m_device_id = line.substr(start_pos, end_pos - start_pos);
				ret = true;
				break;
			}
		}
	}

	in_file.close();

	return ret;
}
