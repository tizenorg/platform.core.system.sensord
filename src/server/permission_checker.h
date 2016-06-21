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

#ifndef _PERMISSION_CHECKER_H_
#define _PERMISSION_CHECKER_H_

#include <cmutex.h>
#include <string>
#include <vector>
#include <memory>

class permission_checker {
public:
	static permission_checker& get_instance(void);

	int get_permission(int sock_fd);

private:
	class permission_info {
		public:
		permission_info(int _permission, std::string _priv, std::string _access)
		: permission(_permission)
		, privilege(_priv)
		, access(_access)
		{
		}
		int permission;
		std::string privilege;
		std::string access;
	};

	typedef std::vector<std::shared_ptr<permission_info>> permission_info_vector;

	permission_checker();
	permission_checker(permission_checker const&) {};
	permission_checker& operator=(permission_checker const&);

	~permission_checker();

	void init(void);

private:
	permission_info_vector m_permission_infos;
	int m_permission_set;
	cmutex m_mutex;

private:
	void init_cynara(void);
	void deinit_cynara(void);
};

#endif /* _PERMISSION_CHECKER_H_ */
