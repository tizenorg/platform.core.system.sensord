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

#include <sensor_common.h>
#include <common.h>
#include <sensor_auto_rotation.h>
#include <auto_rotation_alg_emul.h>
#include <stdlib.h>
#include <math.h>

#define ROTATION_RULE_CNT 4

struct rotation_rule {
	int tilt;
	int angle;
};

struct rotation_rule rot_rule[ROTATION_RULE_CNT] = {
	{40, 80},
	{50, 70},
	{60, 65},
	{90, 60},
};

auto_rotation_alg_emul::auto_rotation_alg_emul()
{
	prev_rotation_x = AUTO_ROTATION_DEGREE_UNKNOWN;
	prev_rotation_y = AUTO_ROTATION_DEGREE_UNKNOWN;
	prev_rotation_z = AUTO_ROTATION_DEGREE_UNKNOWN;
}

auto_rotation_alg_emul::~auto_rotation_alg_emul()
{
	close();
}

int auto_rotation_alg_emul::convert_rotation(int prev_rotation, float acc_theta)
{
	const int ROTATION_0 = 0;
	const int ROTATION_90 = 90;
	const int ROTATION_180 = 180;
	const int ROTATION_360 = 360;
	const int TILT_MIN = 30;
	int tilt;
	int angle;

	int new_rotation = AUTO_ROTATION_DEGREE_UNKNOWN;

	for (int i = 0; i < ROTATION_RULE_CNT; ++i) {
		tilt = rot_rule[i].tilt;

		if ((prev_rotation == AUTO_ROTATION_DEGREE_0) || (prev_rotation == AUTO_ROTATION_DEGREE_180))
			angle = rot_rule[i].angle;
		else
			angle = ROTATION_90 - rot_rule[i].angle;

		if ((acc_theta >= ROTATION_360 - angle && acc_theta <= ROTATION_360 - 1) ||
			(acc_theta >= ROTATION_0 && acc_theta <= ROTATION_0 + angle))
			new_rotation = AUTO_ROTATION_DEGREE_0;
		else if (acc_theta >= ROTATION_0 + angle && acc_theta <= ROTATION_180 - angle)
			new_rotation = AUTO_ROTATION_DEGREE_90;
		else if (acc_theta >= ROTATION_180 - angle && acc_theta <= ROTATION_180 + angle)
			new_rotation = AUTO_ROTATION_DEGREE_180;
		else if (acc_theta >= ROTATION_180 + angle && acc_theta <= ROTATION_360 - angle)
			new_rotation = AUTO_ROTATION_DEGREE_270;

		break;
	}

	return new_rotation;
}

int auto_rotation_alg_emul::overall_rotation(int rot_x, int rot_y, int rot_z)
{
	if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_180 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_180_0;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_180 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_180_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_180 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_180_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_270 &&
			rot_y == AUTO_ROTATION_DEGREE_180 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_180_270;
	else if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_0 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_0_0;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_0 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_0_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_0 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_0_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_270 &&
			rot_y == AUTO_ROTATION_DEGREE_0 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_0_270;
	else if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_90_0;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_90_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_90_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_270 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_90_270;
	else if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_270_0;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_270_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_270_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_270 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_90)
		return AUTO_ROTATION_DEGREE_90_270_270;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_0)
		return AUTO_ROTATION_DEGREE_0_270_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_180)
		return AUTO_ROTATION_DEGREE_180_270_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_0)
		return AUTO_ROTATION_DEGREE_0_90_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_90 &&
			rot_y == AUTO_ROTATION_DEGREE_90 && rot_z == AUTO_ROTATION_DEGREE_180)
		return AUTO_ROTATION_DEGREE_180_90_90;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_0)
		return AUTO_ROTATION_DEGREE_0_270_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_180 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_180)
		return AUTO_ROTATION_DEGREE_180_270_180;
	else if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_0)
		return AUTO_ROTATION_DEGREE_0_270_0;
	else if (rot_x == AUTO_ROTATION_DEGREE_0 &&
			rot_y == AUTO_ROTATION_DEGREE_270 && rot_z == AUTO_ROTATION_DEGREE_180)
		return AUTO_ROTATION_DEGREE_180_270_0;

	return AUTO_ROTATION_DEGREE_UNKNOWN;
}


bool auto_rotation_alg_emul::get_rotation(float acc[3],
		unsigned long long timestamp, int prev_rotation, int &cur_rotation)
{
	const int ROTATION_90 = 90;
	const int RADIAN = 57.29747;

	acc[1] += 180;
	acc[2] += 90;

	int rot1, rot2, rot3;

	rot1 = convert_rotation(prev_rotation_x, (int)acc[0]);
	rot2 = convert_rotation(prev_rotation_y, (int)acc[1]);
	rot3 = convert_rotation(prev_rotation_z, (int)acc[2]);

	INFO("rotation values - %d, %d, %d", rot1, rot2, rot3);

	prev_rotation_x = rot1;
	prev_rotation_y = rot2;
	prev_rotation_z = rot3;

	cur_rotation = overall_rotation(rot1, rot2, rot3);

	if (cur_rotation == AUTO_ROTATION_DEGREE_UNKNOWN)
		return false;

	if (cur_rotation != prev_rotation)
		return true;

	return false;
}
