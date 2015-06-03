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

#ifndef _AMBIENT_TEMPERATURE_SENSOR_H_
#define _AMBIENT_TEMPERATURE_SENSOR_H_

#include <sensor_internal.h>
#include <virtual_sensor.h>
#include <sensor_data.h>

#include "EngineInterface.h"

class ambient_temperature_sensor : public virtual_sensor {
public:
    ambient_temperature_sensor();
    virtual ~ambient_temperature_sensor();

    bool init(void);

    void synthesize(const sensor_event_t &event, vector<sensor_event_t> &outs);

    bool add_interval(int client_id, unsigned int interval);
    bool delete_interval(int client_id);
    bool get_properties(sensor_properties_s &properties);
    sensor_type_t get_type(void);

    int get_sensor_data(const unsigned int event_type, sensor_data_t &data);

private:
    sensor_base *m_temperature_sensor;
    EngineInterface * m_engine;

    cmutex m_value_mutex;

    unsigned int m_interval;

    string m_vendor;
    string m_raw_data_unit;
    int m_default_sampling_time;

    bool on_start(void);
    bool on_stop(void);
    bool load_engine(void);
    void compensate(unsigned long long timestamp, float &temperature);
};

#endif
