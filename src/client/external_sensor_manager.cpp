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
#include <sensor_internal.h>
#include <sensor_common.h>
#include <command_common.h>
#include <external_sensor_manager.h>
#include <external_data_channel.h>
#include <poller.h>
#include <thread>

using std::pair;
using std::vector;
using std::thread;
using std::string;

external_sensor_manager::external_sensor_manager()
: m_client_id(CLIENT_ID_INVALID)
, m_poller(NULL)
, m_thread_state(THREAD_STATE_TERMINATE)
, m_hup_observer(NULL)
{
}

external_sensor_manager::~external_sensor_manager()
{
	stop_command_listener();
}

external_sensor_manager& external_sensor_manager::get_instance(void)
{
	static external_sensor_manager inst;
	return inst;
}

int external_sensor_manager::create_handle(void)
{
	sensor_ext_handle_info handle_info;
	int handle = 0;

	AUTOLOCK(m_handle_info_lock);

	while (m_sensor_handle_infos.count(handle) > 0)
		handle++;

	if (handle >= MAX_HANDLE) {
		_E("Handles of client %s are full", get_client_name());
		return MAX_HANDLE_REACHED;
	}

	handle_info.m_handle = handle;
	handle_info.m_sensor = UNKNOWN_SENSOR;
	handle_info.m_cb = NULL;
	handle_info.m_user_data = NULL;

	m_sensor_handle_infos.insert(pair<int, sensor_ext_handle_info>(handle, handle_info));

	return handle;
}

bool external_sensor_manager::delete_handle(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	m_sensor_handle_map.erase(it_handle->second.m_sensor);
	m_sensor_handle_infos.erase(it_handle);

	return true;
}

bool external_sensor_manager::set_handle(int handle, sensor_id_t sensor, const string& key, void* cb, void* user_data)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	it_handle->second.m_sensor = sensor;
	it_handle->second.m_key = key;
	it_handle->second.m_cb = cb;
	it_handle->second.m_user_data = user_data;

	m_sensor_handle_map.insert(pair<sensor_id_t, int>(sensor, handle));

	return true;
}

bool external_sensor_manager::get_sensor(int handle, sensor_id_t &sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	sensor = it_handle->second.m_sensor;

	return true;
}

int external_sensor_manager::get_handle(sensor_id_t sensor)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_map.find(sensor);

	if (it_handle == m_sensor_handle_map.end()) {
		_E("Handle is not found for client %s with sensor: %d", get_client_name(), sensor);
		return -1;
	}

	return it_handle->second;
}

bool external_sensor_manager::get_handle_info(int handle, const sensor_ext_handle_info*& handle_info)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		_E("Handle[%d] is not found for client %s", handle, get_client_name());
		return false;
	}

	handle_info = &(it_handle->second);
	return true;
}

string external_sensor_manager::get_key(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end()) {
		return string("INVALID_KEY");
	}

	return it_handle->second.m_key;
}

bool external_sensor_manager::has_client_id(void)
{
	return (m_client_id != CLIENT_ID_INVALID);
}

int external_sensor_manager::get_client_id(void)
{
	return m_client_id;
}

void external_sensor_manager::set_client_id(int client_id)
{
	m_client_id = client_id;
}

bool external_sensor_manager::add_data_channel(int handle, external_data_channel *channel)
{
	auto it_channel = m_data_channels.find(handle);

	if (it_channel != m_data_channels.end()) {
		_E("%s alreay has data_channel for %s", get_client_name(), get_key(handle).c_str());
		return false;
	}

	m_data_channels.insert(pair<int, external_data_channel *>(handle, channel));
	return true;
}

bool external_sensor_manager::get_data_channel(int handle, external_data_channel **channel)
{
	auto it_channel = m_data_channels.find(handle);

	if (it_channel == m_data_channels.end()) {
		_E("%s doesn't have data_channel for %s", get_client_name(), get_key(handle).c_str());
		return false;
	}

	*channel = it_channel->second;

	return true;
}

bool external_sensor_manager::close_data_channel(void)
{
	auto it_channel = m_data_channels.begin();

	if (it_channel != m_data_channels.end()) {
		delete it_channel->second;
		++it_channel;
	}

	m_data_channels.clear();

	return true;
}

bool external_sensor_manager::close_data_channel(int handle)
{
	auto it_channel = m_data_channels.find(handle);

	if (it_channel == m_data_channels.end()) {
		_E("%s doesn't have data_channel for %s", get_client_name(), get_key(handle).c_str());
		return false;
	}

	delete it_channel->second;

	m_data_channels.erase(it_channel);

	return true;
}

bool external_sensor_manager::is_valid(int handle)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.find(handle);

	if (it_handle == m_sensor_handle_infos.end())
		return false;

	return true;
}

bool external_sensor_manager::is_active(void)
{
	AUTOLOCK(m_handle_info_lock);

	return !m_sensor_handle_infos.empty();
}

void external_sensor_manager::get_all_handles(vector<int> &handles)
{
	AUTOLOCK(m_handle_info_lock);

	auto it_handle = m_sensor_handle_infos.begin();

	while (it_handle != m_sensor_handle_infos.end()) {
		handles.push_back(it_handle->first);
		++it_handle;
	}
}

bool external_sensor_manager::create_command_channel(void)
{
	const int client_type = CLIENT_TYPE_EXTERNAL_SOURCE;
	int client_id;
	channel_ready_t channel_ready;

	if (!m_command_socket.create(SOCK_SEQPACKET))
		return false;

	if (!m_command_socket.connect(EVENT_CHANNEL_PATH)) {
		_E("Failed to connect command channel for client %s, command socket fd[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	m_command_socket.set_connection_mode();

	if (m_command_socket.send(&client_type, sizeof(client_type)) <= 0) {
		_E("Failed to send client type in client %s, event socket fd[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	client_id = get_client_id();

	if (m_command_socket.send(&client_id, sizeof(client_id)) <= 0) {
		_E("Failed to send client id for client %s on command socket[%d]", get_client_name(), m_command_socket.get_socket_fd());
		return false;
	}

	if (m_command_socket.recv(&channel_ready, sizeof(channel_ready)) <= 0) {
		_E("%s failed to recv command channel ready packet on command socket[%d] with client id [%d]",
			get_client_name(), m_command_socket.get_socket_fd(), client_id);
		return false;
	}

	if ((channel_ready.magic != CHANNEL_MAGIC_NUM) || (channel_ready.client_id != client_id)) {
		_E("Command channel ready packet is wrong, magic = %#x, client id = %d",
			channel_ready.magic, channel_ready.client_id);
		return false;
	}

	_I("Command channel is established for client %s on socket[%d] with client id : %d",
		get_client_name(), m_command_socket.get_socket_fd(), client_id);

	return true;
}

void external_sensor_manager::close_command_channel(void)
{
	m_command_socket.close();
}

bool external_sensor_manager::start_command_listener(void)
{
	if (!create_command_channel()) {
		_E("Command channel is not established for %s", get_client_name());
		return false;
	}

	m_command_socket.set_transfer_mode();

	m_poller = new(std::nothrow) poller(m_command_socket.get_socket_fd());
	retvm_if(!m_poller, false, "Failed to allocate memory");

	set_thread_state(THREAD_STATE_START);

	thread listener(&external_sensor_manager::listen_command, this);
	listener.detach();

	return true;
}

void external_sensor_manager::stop_command_listener(void)
{
	const int THREAD_TERMINATING_TIMEOUT = 2;

	ulock u(m_thread_mutex);

	if (m_thread_state != THREAD_STATE_TERMINATE) {
		m_thread_state = THREAD_STATE_STOP;

		_D("%s is waiting listener thread[state: %d] to be terminated", get_client_name(), m_thread_state);
		if (m_thread_cond.wait_for(u, std::chrono::seconds(THREAD_TERMINATING_TIMEOUT))
			== std::cv_status::timeout)
			_E("Fail to stop listener thread after waiting %d seconds", THREAD_TERMINATING_TIMEOUT);
		else
			_D("Listener thread for %s is terminated", get_client_name());
	}
}

void external_sensor_manager::set_thread_state(thread_state state)
{
	lock l(m_thread_mutex);
	m_thread_state = state;
}

bool external_sensor_manager::get_cb_info(sensor_id_t sensor, char* data, int data_cnt, command_cb_info &cb_info)
{
	int handle;
	const sensor_ext_handle_info *handle_info;

	AUTOLOCK(m_handle_info_lock);

	handle = get_handle(sensor);

	if (handle < 0)
		return false;

	get_handle_info(handle, handle_info);

	cb_info.handle = handle_info->m_handle;
	cb_info.sensor = handle_info->m_sensor;
	cb_info.cb = handle_info->m_cb;
	cb_info.data = data;
	cb_info.data_cnt = data_cnt;
	cb_info.user_data = handle_info->m_user_data;

	return true;
}

bool external_sensor_manager::sensor_command_poll(void* buffer, int buffer_len, struct epoll_event &event)
{
	ssize_t len;

	len = m_command_socket.recv(buffer, buffer_len);

	if (!len) {
		if (!m_poller->poll(event))
			return false;
		len = m_command_socket.recv(buffer, buffer_len);

		if (len <= 0) {
			_I("%s failed to read after poll!", get_client_name());
			return false;
		}
	} else if (len < 0) {
		_I("%s failed to recv command from command socket", get_client_name());
		return false;
	}

	return true;
}

void external_sensor_manager::post_callback_to_main_loop(command_cb_info* cb_info)
{
	g_idle_add_full(G_PRIORITY_DEFAULT, callback_dispatcher, cb_info, NULL);
}

void external_sensor_manager::handle_command(sensor_id_t sensor, char* data, int data_cnt)
{
	command_cb_info *cb_info = NULL;

	{	/* scope for the lock */
		AUTOLOCK(m_handle_info_lock);

		cb_info = new(std::nothrow) command_cb_info;
		if (!cb_info) {
			_E("Failed to allocate memory");
			delete[] data;
		}

		if (!get_cb_info(sensor, data, data_cnt, *cb_info)) {
			delete[] data;
			delete cb_info;
			_E("Sensor %d is not connected, so command is discarded", sensor);
			return;
		}
	}

	if (cb_info)
		post_callback_to_main_loop(cb_info);
}

void external_sensor_manager::listen_command(void)
{
	external_command_header_t command_header;
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLPRI;

	do {
		lock l(m_thread_mutex);
		if (m_thread_state == THREAD_STATE_START) {
			if (!sensor_command_poll(&command_header, sizeof(command_header), event)) {
				_I("Failed to poll command header");
				break;
			}

			char *command = new(std::nothrow) char[command_header.command_len];
			if (!command) {
				_E("Failed to allocated memory");
				break;
			}

			if (!sensor_command_poll(command, command_header.command_len, event)) {
				_I("Failed to poll command data");
				delete []command;
				break;
			}

			handle_command(command_header.sensor_id, command, command_header.command_len);
		} else {
			break;
		}
	} while (true);

	if (m_poller != NULL) {
		delete m_poller;
		m_poller = NULL;
	}

	close_command_channel();

	{ /* the scope for the lock */
		lock l(m_thread_mutex);
		m_thread_state = THREAD_STATE_TERMINATE;
		m_thread_cond.notify_one();
	}

	_I("Command listener thread is terminated.");

	if (has_client_id() && (event.events & EPOLLHUP)) {
		if (m_hup_observer)
			m_hup_observer();
	}
}

bool external_sensor_manager::is_valid_callback(const command_cb_info *cb_info)
{
	sensor_id_t sensor;

	if (!external_sensor_manager::get_instance().get_sensor(cb_info->handle, sensor))
		return false;

	return (cb_info->sensor == sensor);
}

gboolean external_sensor_manager::callback_dispatcher(gpointer data)
{
	const command_cb_info *cb_info = reinterpret_cast<const command_cb_info*>(data);

	if (external_sensor_manager::get_instance().is_valid_callback(cb_info)) {
		reinterpret_cast<sensor_external_command_cb_t>(cb_info->cb)(cb_info->handle, cb_info->data, cb_info->data_cnt, cb_info->user_data);
	} else {
		_W("Discard invalid callback cb(%#x)(%d, %#x, %d, %#x)",
		cb_info->cb, cb_info->handle, cb_info->data, cb_info->data_cnt, cb_info->user_data);
	}

	delete[] cb_info->data;
	delete cb_info;

/*
* 	To be called only once, it returns false
*/
	return false;
}

void external_sensor_manager::clear(void)
{
	close_command_channel();
	stop_command_listener();
	close_data_channel();
	m_sensor_handle_infos.clear();
	set_client_id(CLIENT_ID_INVALID);
}

void external_sensor_manager::set_hup_observer(hup_observer_t observer)
{
	m_hup_observer = observer;
}
