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

#include <linux/ioctl.h>
#include <fstream>
#include <string>
#include <common.h>

using std::string;
using std::ifstream;

#define NO_OF_ULL_BYTES		8
#define NO_OF_SHORT_VAL		4
#define CH0_INDEX			0
#define CH1_INDEX			1

#define GET_DIFF_BIT(val)	(((unsigned short)(val) >> 7) & 0x01)
#define GET_DIR_VAL(val)	((val) & 0x0F)

#define IOCTL_IIO_EVENT_FD _IOR('i', 0x90, int)

struct channel_parameters
{
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

typedef struct iio_event_struct
{
	unsigned long long int event_id;
	long long int timestamp;
} iio_event_t;

typedef enum event_id_field
{
	CH_TYPE = 4,
	MODIFIER,
	DIRECTION,
	EVENT_TYPE,
} event_id_field_t;

typedef union ull_bytes
{
	unsigned long long num;
	short int channels[NO_OF_SHORT_VAL];
	unsigned char bytes[NO_OF_ULL_BYTES];
} ull_bytes_t;

void sort_channels_by_index(struct channel_parameters *channels, int count);
int decode_channel_data_type(const char *device_dir, const char *ch_name, struct channel_parameters *ch_info);
int add_channel_to_array(const char *device_dir, const char *ch_name, struct channel_parameters *channel);
int get_channel_array_size(struct channel_parameters *channels, int num_channels);
int update_sysfs_num(const char *filepath, int val, bool verify = false);
int update_sysfs_string(const char *filepath, char *val, bool verify = false);
int convert_bytes_to_int(int input, struct channel_parameters *info);

template <typename value_t>
bool read_node_value(string node_path, value_t &value)
{
	ifstream handle;
	handle.open(node_path.c_str());
	if (!handle)
	{
		ERR("Failed to open handle(%s)", node_path.c_str());
		return false;
	}
	handle >> value;
	handle.close();

	return true;
}

#endif /* IIO_COMMON_H_ */
