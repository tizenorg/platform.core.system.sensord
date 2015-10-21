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

#ifndef _CWAKEUP_INFO_LIST_CLASS_H_
#define _CWAKEUP_INFO_LIST_CLASS_H_

#include <list>

class cwakeup_info
{
public:
	cwakeup_info(int client_id, int wakeup);
	int client_id;
	int wakeup;
};

typedef std::list<cwakeup_info>::iterator cwakeup_info_iterator;

class cwakeup_info_list
{
private:
	cwakeup_info_iterator find_if(int client_id);

	std::list<cwakeup_info> m_list;

public:
	bool add_wakeup(int client_id, int wakeup);
	bool delete_wakeup(int client_id);
	int get_wakeup(int client_id);
	int is_wakeup_on(void);
};
#endif
