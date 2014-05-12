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

#include <iio_common.h>
#include <fcntl.h>
#include <dirent.h>
#include <fstream>
#include <string>
#include <iostream>

#define SCAN_EL_DIR "scan_elements/"

#define BYTE_SIZE	8
#define ULONGLONG	64

using std::fstream;
using std::ifstream;
using std::string;
using std::ios;

void sort_channels_by_index(struct channel_parameters *channels, int count)
{
	struct channel_parameters temp;
	int i,j;

	for(i=0; i<count; i++)
		for(j=i;(j>0) && (channels[j].index < channels[j-1].index); j--)
		{
			temp = channels[j];
			channels[j] = channels[j-1];
			channels[j-1] = temp;
		}
}

int decode_channel_data_type(const char *device_dir, const char *ch_name, struct channel_parameters *ch_info)
{
	string file_type, type_string;
	ifstream ftype;
	char s_char, e_char;
	unsigned bit_count, pad_bit_count, shift_count;

	file_type = string(device_dir) + string(SCAN_EL_DIR) + string(ch_name) + string("_type");
	ftype.open(file_type.c_str());
	if (!ftype.is_open())
		return -1;
	ftype >> type_string;
	sscanf(type_string.c_str(), "%ce:%c%u/%u>>%u", &e_char,
			&s_char, &bit_count, &pad_bit_count, &shift_count);
	if (e_char == 'b')
		ch_info->big_endian = 1;
	else
		ch_info->big_endian = 0;

	if (s_char == 's')
		ch_info->is_signed = 1;
	else
		ch_info->is_signed = 0;

	ch_info->valid_bits = bit_count;
	ch_info->byte_count = pad_bit_count / BYTE_SIZE;

	if (bit_count == ULONGLONG)
		ch_info->mask = ~0;
	else
		ch_info->mask = (1 << bit_count) - 1;
	ch_info->shift = shift_count;
	return 0;
}

int add_channel_to_array(const char *device_dir, const char *ch_name, struct channel_parameters *channel)
{
	string file_en, file_type, file_index, file_scale, file_offset;
	ifstream ftemp;
	unsigned int i;

	file_en = string(device_dir) + string(SCAN_EL_DIR) + string(ch_name) + string("_en");
	file_index = string(device_dir) + string(SCAN_EL_DIR) + string(ch_name) + string("_index");
	file_scale = string(device_dir) + string(ch_name) + string("_scale");
	file_offset = string(device_dir) + string(ch_name) + string("_offset");

	ftemp.open(file_en.c_str());
	if (!ftemp.is_open())
		return -1;

	ftemp >> i;
	ftemp.close();
	if (i == 0)
		return -1;

	asprintf(&(channel->prefix_str), "%s", ch_name);
	channel->is_en = 1;
	channel->scale = 1.0;
	channel->offset = 0.0;

	ftemp.open(file_index.c_str());
	if (!ftemp.is_open())
		return -1;
	ftemp >> i;
	channel->index = i;
	ftemp.close();

	ftemp.open(file_scale.c_str());
	if (ftemp.is_open())
	{
		ftemp >> channel->scale;
		ftemp.close();
	}

	ftemp.open(file_offset.c_str());
	if (ftemp.is_open())
	{
		ftemp >> channel->offset;
		ftemp.close();
	}
	decode_channel_data_type(device_dir, ch_name, channel);
	return 0;
}

int get_channel_array_size(struct channel_parameters *channels, int num_channels)
{
	int bytes = 0;
	int i = 0;
	while (i < num_channels) {
		if (bytes % channels[i].byte_count == 0)
			channels[i].buf_index = bytes;
		else
			channels[i].buf_index = bytes - bytes % channels[i].byte_count
			+ channels[i].byte_count;
		bytes = channels[i].buf_index + channels[i].byte_count;
		i++;
	}
	return bytes;
}

int update_sysfs_num(const char *filepath, int val, bool verify)
{
	fstream sysfile;
	sysfile.open(filepath);
	if (!sysfile.is_open())
		return -1;
	sysfile << val;
	if (verify)
	{
		int test;
		sysfile.seekp(0, ios::beg);
		sysfile >> test;
		if (test != val)
			return -1;
	}
	return 0;
}

int update_sysfs_string(const char *filepath, char *val, bool verify)
{
	fstream sysfile;
	sysfile.open(filepath);
	if (!sysfile.is_open())
		return -1;
	sysfile << val;
	if (verify)
	{
		string test;
		sysfile.seekp(0, ios::beg);
		sysfile >> test;
		if (test != string(val))
			return -1;
	}
	return 0;
}
