/*
 * sensorctl
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

#pragma once // _TESTER_H_

#include <sensor_internal.h>
#include <string>
#include <vector>

#include "sensor_manager.h"

#define ARGC_BASE 3 /* e.g. {sensorctl, test, accelerometer} */

#define REGISTER_TESTER(sensor_type, event_name, tester_type) \
static tester_type tester(sensor_type, event_name);

#define FAIL_TEXT(condition, text) \
{ \
	_I("[   FAIL   ] "); \
	PRINT("%s(%d) -> %s(%s)\n", __FUNCTION__, __LINE__, condition, text, #condition); \
	sensorctl::tester_bench::push_failure(__FUNCTION__, __LINE__, text); \
}

#define PASS_TEXT(condition, text) \
{ \
	if (sensorctl::tester_option::full_log()) { \
		_I("[   PASS   ] "); \
		PRINT("%s(%d) -> %s(%s)\n", __FUNCTION__, __LINE__, condition, text, #condition); \
	} \
}

#define ASSERT_IF_TEXT(condition, text) \
{ \
	if (!(condition)) { \
		FAIL_TEXT(condition, text) \
		return false; \
	} \
	PASS_TEXT(condition, text) \
}

#define EXPECT_IF_TEXT(condition, text)\
{ \
	if (!(condition)) \
		FAIL_TEXT(condition, text)\
	PASS_TEXT(condition, text) \
}

#define ASSERT_IF(condition) \
	ASSERT_IF_TEXT(condition, sensorctl::tester_case::name()); \

#define EXPECT_IF(condition) \
	EXPECT_IF_TEXT(condition, sensorctl::tester_case::name()); \

namespace sensorctl {

class tester_manager;

class tester {
public:
	tester(sensor_type_t sensor_type, const char *event_name)
	: m_type(sensor_type)
	, m_name(event_name)
	{
		tester_manager::register_tester(this);
	}
	virtual ~tester() {}

	virtual void setup(void) { return true; }
	virtual void teardown(void) { return true; }

	const std::string& name() const { return m_name; }
	const sensor_type_t type() const { return m_type; }

	virtual bool test(int argc, char *argv[]) = 0;

private:
	sensor_type_t m_type;
	std::string m_name;
};

class tester_manager : public sensor_manager {
public:
	static bool register_tester(tester *test);

public:
	tester_manager() {}
	virtual ~tester_manager();

	void run(int argc, char *argv[]);

private:
	static std::vector<tester *> testers;

	tester *get_tester(sensor_type_t type, const char *name);
	void usage(void);
};

class tester_option {
public:
	static bool show_full_log(bool show) { instance().m_full_log = show; }
	static bool full_log(void) { return instance().m_full_log; }

public:
	bool m_full_log;

private:
	static tester_option& instance();
};

typedef struct result {
	std::string &function;
	long line;
	const std::string &msg;
};

class tester_bench {
public:
	tester_result() : m_failure_count(0) {}

	static void push_failure(const std::string &function, long line, const std::string &msg)
	{
		instance().add_failure(function, line, msg);
	}

	static void show_result(void)
	{
		instance().show_failures(void);
	}

private:
	static tester_result& instance();

	void add_failure(const std::string &function, long line, const std::string &msg)
	{
		struct result fail;
		fail.function = function;
		fail.line = line;
		fail.msg = msg;

		failures.push_back(fail);
		m_failure_count++;                                  
	}

	void show_failures(void)
	{
		PRINT("================================\n");

		if (m_failer_count == 0) {
			PRINT("there was no fail case\n");
			return;
		}

		PRINT("%d case(s) are failed, listed below:\n", m_failure_count);

		for (int i = 0; i < failures.size(); ++i) {
			PRINT("%s(%d) -> %s\n", 
				failures[i].m_function.c_str(),
				failures[i].line,
				failures[i].msg.c_str());
		}
		m_failure_count = 0;
	}

private:
	std::vector<tester_case> tests;
	std::vector<result> failures;
	int m_failure_count;
};

class tester_case {
public:
	tester_case(const std::string &name)
	: m_name(name)
	{
		sensorctl::test_bench::register_testcase(this);
	}
}

} /* namespace sensorctl */
