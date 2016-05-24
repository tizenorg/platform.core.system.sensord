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

#ifndef _COMMAND_QUEUE_H_
#define _COMMAND_QUEUE_H_

#include <command_common.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class command_queue {
private:
	command_queue() {};
	~command_queue() {};
	command_queue(const command_queue&) {};
	command_queue& operator=(const command_queue&);

	static const unsigned int QUEUE_FULL_SIZE = 1000;

	std::queue<std::shared_ptr<external_command_t>> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cond_var;

	typedef std::lock_guard<std::mutex> lock;
	typedef std::unique_lock<std::mutex> ulock;
public:
	static command_queue& get_instance();
	void push(std::shared_ptr<external_command_t> &command);
	std::shared_ptr<external_command_t> pop(void);
};

#endif
