/*
 * libsensord-share
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

#include <map>
#include <poller.h>
#include <physical_sensor.h>

typedef std::multimap<int, physical_sensor *> fd_sensor_plugins;

class sensor_event_poller {
public:
	sensor_event_poller();
	virtual ~sensor_event_poller();

	bool poll();
private:
	poller m_poller;
	fd_sensor_plugins m_fd_sensors;

	void init_fd();
	void init_sensor_map();
	bool add_poll_fd(int fd);
	bool ready_event(int fd);
	bool process_event(int fd);
};
