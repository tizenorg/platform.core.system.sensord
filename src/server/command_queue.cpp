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

#include <sensor_log.h>
#include <command_queue.h>

command_queue& command_queue::get_instance(void)
{
	static command_queue inst;
	return inst;
}

void command_queue::push(std::shared_ptr<external_command_t> &command)
{
	lock l(m_mutex);

	bool wake = m_queue.empty();

	if (m_queue.size() >= QUEUE_FULL_SIZE) {
		_E("Queue is full, drop it!");
	} else {
		m_queue.push(command);
	}

	if (wake)
		m_cond_var.notify_one();
}

std::shared_ptr<external_command_t> command_queue::pop(void)
{
	ulock u(m_mutex);

	while (m_queue.empty())
		m_cond_var.wait(u);

	std::shared_ptr<external_command_t> command = m_queue.front();
	m_queue.pop();
	return command;
}
