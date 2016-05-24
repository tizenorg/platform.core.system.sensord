/*
 * sensord
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
#include <sensor_common.h>
#include <command_common.h>
#include <sensor_internal.h>
#include <external_sensor_manager.h>
#include <external_data_channel.h>
#include <cmutex.h>

using std::vector;
using std::string;

static cmutex lock;

static void clean_up(void);
static void restore_session(void);

class initiator {
public:
	initiator()
	{
		external_sensor_manager::get_instance().set_hup_observer(restore_session);
	}
};

static initiator g_initiator;

void clean_up(void)
{
	vector<int> handles;

	external_sensor_manager::get_instance().get_all_handles(handles);

	auto it_handle = handles.begin();

	while (it_handle != handles.end()) {
		sensord_external_disconnect(*it_handle);
		++it_handle;
	}
}

void restore_session(void)
{
	AUTOLOCK(lock);

	_I("Trying to restore external source session for %s", get_client_name());

	external_data_channel *data_channel;
	int client_id;

	external_sensor_manager::get_instance().close_data_channel();
	external_sensor_manager::get_instance().set_client_id(CLIENT_ID_INVALID);

	vector<int> handles;

	external_sensor_manager::get_instance().get_all_handles(handles);

	bool first_connection = true;

	auto it_handle = handles.begin();

	while (it_handle != handles.end()) {
		data_channel = new(std::nothrow) external_data_channel();
		retm_if(!data_channel, "Failed to allocate memory");

		if (!data_channel->create_channel()) {
			_E("%s failed to create data channel", get_client_name());
			delete data_channel;
			goto FAILED;
		}

		external_sensor_manager::get_instance().add_data_channel(*it_handle, data_channel);

		if (first_connection) {
			first_connection = false;
			if (!data_channel->cmd_get_id(client_id)) {
				_E("Failed to get client id");
				goto FAILED;
			}

			external_sensor_manager::get_instance().set_client_id(client_id);
			external_sensor_manager::get_instance().start_command_listener();
		}

		data_channel->set_client_id(client_id);

		sensor_id_t dummy;
		if (!data_channel->cmd_connect(external_sensor_manager::get_instance().get_key(*it_handle), dummy)) {
			_E("Sending cmd_connect(%s) failed for %s", external_sensor_manager::get_instance().get_key(*it_handle).c_str(), get_client_name());
			goto FAILED;
		}

		++it_handle;
	}

	_I("Succeeded to restore external source session for %s", get_client_name());

	return;

FAILED:
	external_sensor_manager::get_instance().clear();
	_E("Failed to restore external source session for %s", get_client_name());
}

API int sensord_external_connect(const char *key, sensor_external_command_cb_t cb, void *user_data)
{
	external_data_channel *channel = NULL;
	int handle;
	int client_id;
	bool first_connection = false;

	retvm_if(!key, OP_ERROR, "client %s passes null key", get_client_name());

	AUTOLOCK(lock);

	handle = external_sensor_manager::get_instance().create_handle();

	// lazy loading after creating static variables
	atexit(clean_up);

	if (handle == MAX_HANDLE) {
		_E("Maximum number of handles reached, key %s in client %s", key, get_client_name());
		return OP_ERROR;
	}

	channel = new(std::nothrow) external_data_channel();
	if (!channel) {
		_E("Failed to allocated memory");
		external_sensor_manager::get_instance().delete_handle(handle);
		return OP_ERROR;
	}

	if (!channel->create_channel()) {
		_E("%s failed to create data channel for %s", get_client_name(), key);
		external_sensor_manager::get_instance().delete_handle(handle);
		delete channel;
		return OP_ERROR;
	}

	external_sensor_manager::get_instance().add_data_channel(handle, channel);

	if (!external_sensor_manager::get_instance().has_client_id()) {
		first_connection = true;
		if (!channel->cmd_get_id(client_id)) {
			_E("Sending cmd_get_id() failed for %s", key);
			external_sensor_manager::get_instance().close_data_channel(handle);
			external_sensor_manager::get_instance().delete_handle(handle);
			return OP_ERROR;
		}

		external_sensor_manager::get_instance().set_client_id(client_id);
		_I("%s gets client_id [%d]", get_client_name(), client_id);
		external_sensor_manager::get_instance().start_command_listener();
		_I("%s starts listening command with client_id [%d]", get_client_name(), client_id);
	}

	client_id = external_sensor_manager::get_instance().get_client_id();
	channel->set_client_id(client_id);

	sensor_id_t sensor;

	if (!channel->cmd_connect(key, sensor)) {
		_E("Failed to connect %s for %s", key, get_client_name());
		external_sensor_manager::get_instance().close_data_channel(handle);
		external_sensor_manager::get_instance().delete_handle(handle);

		if (first_connection) {
			external_sensor_manager::get_instance().set_client_id(CLIENT_ID_INVALID);
			external_sensor_manager::get_instance().stop_command_listener();
		}

		return OP_ERROR;
	}

	_I("%s[%d] connects with %s[%d]", get_client_name(), client_id, key, handle);

	external_sensor_manager::get_instance().set_handle(handle, sensor, string(key), (void *)cb, user_data);

	return handle;
}

API bool sensord_external_disconnect(int handle)
{
	external_data_channel *channel;

	AUTOLOCK(lock);

	retvm_if(!external_sensor_manager::get_instance().is_valid(handle), false, "Handle %d is not valid for %s",
		handle, get_client_name());

	if (!external_sensor_manager::get_instance().get_data_channel(handle, &channel)) {
		_E("client %s failed to get data channel", get_client_name());
		return false;
	}

	_I("%s disconnects with %s[%d]", get_client_name(), external_sensor_manager::get_instance().get_key(handle).c_str(), handle);

	if (!external_sensor_manager::get_instance().delete_handle(handle))
		return false;

	if (!channel->cmd_disconnect()) {
		_E("Sending cmd_disconnect() failed for %s", get_client_name());
		return false;
	}

	external_sensor_manager::get_instance().close_data_channel(handle);

	if (!external_sensor_manager::get_instance().is_active()) {
		_I("Stop listening command for client %s with client id [%d]", get_client_name(), external_sensor_manager::get_instance().get_client_id());
		external_sensor_manager::get_instance().set_client_id(CLIENT_ID_INVALID);
		external_sensor_manager::get_instance().stop_command_listener();
	}

	return true;
}

API bool sensord_external_post(int handle, unsigned long long timestamp, const float* data, int data_cnt)
{
	external_data_channel *channel;

	retvm_if(((data_cnt <= 0) || (data_cnt > POST_DATA_LEN_MAX)), false,
		"data_cnt(%d) is invalid for %s", data_cnt, get_client_name());

	AUTOLOCK(lock);

	retvm_if(!external_sensor_manager::get_instance().is_valid(handle), false, "Handle %d is not valid for %s",
		handle, get_client_name());

	if (!external_sensor_manager::get_instance().get_data_channel(handle, &channel)) {
		_E("client %s failed to get data channel", get_client_name());
		return false;
	}

	if (!channel->cmd_post(timestamp, data, data_cnt)) {
		_E("Failed to post data:%#x, data_cnt:%d", data, data_cnt);
		return false;
	}

	return true;
}
