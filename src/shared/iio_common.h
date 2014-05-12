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

#ifndef IIO_COMMON_H_
#define IIO_COMMON_H_

struct channel_parameters {
	char *prefix_str;
	float scale;
	float offset;
	unsigned int index;
	unsigned int byte_count;
	unsigned int valid_bits;
	unsigned int shift;
	unsigned long long int mask;
	unsigned int big_endian;
	unsigned int is_signed;
	unsigned int is_en;
	unsigned int buf_index;
};

void sort_channels_by_index(struct channel_parameters *channels, int count);
int decode_channel_data_type(const char *device_dir, const char *ch_name, struct channel_parameters *ch_info);
int add_channel_to_array(const char *device_dir, const char *ch_name, struct channel_parameters *channel);
int get_channel_array_size(struct channel_parameters *channels, int num_channels);
int update_sysfs_num(const char *filepath, int val, bool verify = false);
int update_sysfs_string(const char *filepath, char *val, bool verify = false);

#endif /* IIO_COMMON_H_ */
