/*
 * sensorctl
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

#pragma once // _SENSORCTL_LOG_H_

#include <stdio.h>
#include <glib.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

#define PRINT(fmt, args...) \
	do { \
		g_print(fmt, ##args); \
	} while (0)

#define _E(fmt, args...) \
	do { \
		g_print("\x1B[31m" fmt "\033[0m", ##args); \
	} while (0)

#define _I(fmt, args...) \
	do { \
		g_print("\x1B[32m" fmt "\033[0m", ##args); \
	} while (0)

#define WARN_IF(expr, fmt, arg...) \
	do { \
		if(expr) { \
			_E(fmt, ##arg); \
		} \
	} while (0)

#define RET_IF(expr) \
	do { \
		if(expr) { \
			return; \
		} \
	} while (0)

#define RETV_IF(expr, val) \
	do { \
		if(expr) { \
			return (val); \
		} \
	} while (0)

#define RETM_IF(expr, fmt, arg...) \
	do { \
		if(expr) { \
			_E(fmt, ##arg); \
			return; \
		} \
	} while (0)

#define RETVM_IF(expr, val, fmt, arg...) \
	do { \
		if(expr) { \
			_E(fmt, ##arg); \
			return (val); \
		} \
	} while (0)

