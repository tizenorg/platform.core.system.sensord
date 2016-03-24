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

#define TESTER_ARGC 3 /* e.g. {sensorctl, test, accelerometer} */

#define REGISTER_TESTER(sensor_type, event_name, tester_type) \
static tester_type tester(sensor_type, event_name);

#define FAIL_TEXT(condition, text) \
{ \
	_I("[   FAIL   ] "); \
	PRINT("%s(%d) -> %s(%s)\n", __FUNCTION__, __LINE__, text, #condition); \
	sensorctl::tester_bench::push_failure(__FUNCTION__, __LINE__, text); \
}

#define PASS_TEXT(condition, text) \
{ \
	if (sensorctl::tester_option::full_log()) { \
		_I("[   PASS   ] "); \
		PRINT("%s(%d) -> %s(%s)\n", __FUNCTION__, __LINE__, text, #condition); \
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

#define TESTCASE(tc_name) \
class test_case_##tc_name : public sensorctl::tester_case { \
public: \
	test_case_##tc_name() \
	: tester_case(#tc_name) \
	{ \
		register_func(static_cast<sensorctl::tester_case::test_func>(&test_case_##tc_name::test)); \
	} \
	bool test(void); \
} test_case_##tc_name_##instance; \
bool test_case_##tc_name::test(void)

namespace sensorctl {

/*
 * Declaration of tester
 */
class tester {
public:
	tester(sensor_type_t sensor_type, const char *event_name);
	virtual ~tester() {}

	virtual bool setup(void) { return true; }
	virtual bool teardown(void) { return true; }

	const std::string& name() const { return m_name; }
	const sensor_type_t type() const { return m_type; }

	virtual bool test(sensor_type_t type, int argc, char *argv[]) = 0;

private:
	sensor_type_t m_type;
	std::string m_name;
};

/*
 * Declaration of tester_manager
 */
class tester_manager : public sensor_manager {
public:
	static void register_tester(tester *test);

public:
	tester_manager() {}
	virtual ~tester_manager() {}

	bool run(int argc, char *argv[]);

private:
	static std::vector<tester *> testers;

	tester *get_tester(sensor_type_t type, const char *name);
	void usage(void);
};

/*
 * Declaration of tester_option
 */
class tester_option {
public:
	static bool m_full_log;

	static void show_full_log(bool show);
	static bool full_log(void);
};

/*
 * Declaration of tester_case
 */
class tester_case {
public:
	tester_case(const std::string &name);

	void run_testcase(void);

	const std::string& name() const { return m_name; }
protected:
	typedef bool (tester_case::*test_func)();

	void started(void);
	void stopped(void);
	void register_func(test_func func);

private:
	const std::string &m_name;
	test_func m_func;
};

class tester_result {
public:
	tester_result(const std::string &_function, long _line, const std::string &_msg)
	: function(_function)
	, line(_line)
	, msg(_msg)
	{
	}

	const std::string &function;
	long line;
	const std::string &msg;
};

/*
 * Declaration of tester_bench
 */
class tester_bench {
public:
	tester_bench() : m_failure_count(0) {}

	static void register_testcase(tester_case *testcase);
	static void push_failure(const std::string &function, long line, const std::string &msg);
	static void show_result(void);
	static void run_all_testcase(void);

private:
	static tester_bench& instance();

	void add_failure(const std::string &function, long line, const std::string &msg);
	void show_failures(void);

	void add_testcase(tester_case *testcase);
	void run(void);

private:
	std::vector<tester_case *> tests;
	std::vector<tester_result> failures;
	int m_failure_count;
};

} /* namespace sensorctl */

#define ASSERT_IF(condition) \
{ \
	const char *name = tester_case::name().c_str(); \
	ASSERT_IF_TEXT(condition, name); \
}

#define EXPECT_IF(condition) \
{ \
	const char *name = tester_case::name().c_str(); \
	EXPECT_IF_TEXT(condition, name); \
}
