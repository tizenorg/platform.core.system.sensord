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

#if !defined(_CRW_LOCK_CLASS_H_)
#define _CRW_LOCK_CLASS_H_

#include "cbase_lock.h"

class crw_lock : public cbase_lock
{
public:
	crw_lock();
	virtual ~crw_lock();

	void read_lock();
	void write_lock();

protected:
	int read_lock_impl(void);
	int write_lock_impl(void);

	int try_read_lock_impl(void);
	int try_write_lock_impl(void);
	int unlock_impl();
private:
	pthread_rwlock_t m_rw_lock;
};

#endif
// End of a file

