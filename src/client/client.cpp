/*
 * sensord
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

#include <sensor_common.h>
#include <sensor_internal.h>
#include <sensor_event_listener.h>
#include <sensor_client_info.h>
#include <client_common.h>
#include <vconf.h>
#include <cmutex.h>
#include <sensor_log.h>
#include <sensor_info.h>
#include <sensor_info_manager.h>
#include <vector>
#include <algorithm>

using std::vector;

#ifndef API
#define API __attribute__((visibility("default")))
#endif

#define DEFAULT_INTERVAL POLL_10HZ_MS

static cmutex lock;

static int g_power_save_state = 0;

static int get_power_save_state(void);
static void power_save_state_cb(keynode_t *node, void *data);
static void clean_up(void);
static bool change_sensor_rep(sensor_id_t sensor_id, sensor_rep &prev_rep, sensor_rep &cur_rep);
static void restore_session(void);
static bool register_event(int handle, unsigned int event_type, unsigned int interval, int max_batch_latency, int cb_type, void* cb, void *user_data);

class initiator {
public:
	initiator()
	{
		sensor_event_listener::get_instance().set_hup_observer(restore_session);
	}
};

void good_bye(void)
{
	_I("Good bye! %s\n", get_client_name());
	clean_up();
}

static initiator g_initiator;

static int g_power_save_state_cb_cnt = 0;

static void set_power_save_state_cb(void)
{
	if (g_power_save_state_cb_cnt < 0)
		_E("g_power_save_state_cb_cnt(%d) is wrong", g_power_save_state_cb_cnt);

	++g_power_save_state_cb_cnt;

	if (g_power_save_state_cb_cnt == 1) {
		_D("Power save callback is registered");
		g_power_save_state = get_power_save_state();
		_D("power_save_state = [%d]", g_power_save_state);
		vconf_notify_key_changed(VCONFKEY_PM_STATE, power_save_state_cb, NULL);
	}
}

static void unset_power_save_state_cb(void)
{
	--g_power_save_state_cb_cnt;

	if (g_power_save_state_cb_cnt < 0)
		_E("g_power_save_state_cb_cnt(%d) is wrong", g_power_save_state_cb_cnt);

	if (g_power_save_state_cb_cnt == 0) {
		_D("Power save callback is unregistered");
		vconf_ignore_key_changed(VCONFKEY_PM_STATE, power_save_state_cb);
	}
}

void clean_up(void)
{
	handle_vector handles;

	sensor_client_info::get_instance().get_all_handles(handles);

	auto it_handle = handles.begin();

	while (it_handle != handles.end()) {
		sensord_disconnect(*it_handle);
		++it_handle;
	}
}

static int get_power_save_state (void)
{
	int state = 0;
	int pm_state;

	vconf_get_int(VCONFKEY_PM_STATE, &pm_state);

	if (pm_state == VCONFKEY_PM_STATE_LCDOFF)
		state |= SENSOR_OPTION_ON_IN_SCREEN_OFF;

	return state;
}

static void power_save_state_cb(keynode_t *node, void *data)
{
	int cur_power_save_state;
	sensor_id_vector sensors;
	sensor_rep prev_rep, cur_rep;

	AUTOLOCK(lock);

	cur_power_save_state = get_power_save_state();

	if (cur_power_save_state == g_power_save_state) {
		_D("g_power_save_state NOT changed : [%d]", cur_power_save_state);
		return;
	}

	g_power_save_state = cur_power_save_state;
	_D("power_save_state: %d noti to %s", g_power_save_state, get_client_name());

	sensor_client_info::get_instance().get_listening_sensors(sensors);

	auto it_sensor = sensors.begin();

	while (it_sensor != sensors.end()) {
		sensor_client_info::get_instance().get_sensor_rep(*it_sensor, prev_rep);
		sensor_event_listener::get_instance().operate_sensor(*it_sensor, cur_power_save_state);
		sensor_client_info::get_instance().get_sensor_rep(*it_sensor, cur_rep);
		change_sensor_rep(*it_sensor, prev_rep, cur_rep);

		++it_sensor;
	}
}


void restore_session(void)
{
	AUTOLOCK(lock);

	_I("Trying to restore sensor client session for %s", get_client_name());

	command_channel *cmd_channel;
	int client_id;

	sensor_client_info::get_instance().close_command_channel();
	sensor_client_info::get_instance().set_client_id(CLIENT_ID_INVALID);

	sensor_id_vector sensors;

	sensor_client_info::get_instance().get_listening_sensors(sensors);

	bool first_connection = true;

	auto it_sensor = sensors.begin();

	while (it_sensor != sensors.end()) {
		cmd_channel = new(std::nothrow) command_channel();
		retm_if (!cmd_channel, "Failed to allocate memory");

		if (!cmd_channel->create_channel()) {
			_E("%s failed to create command channel for %s", get_client_name(), get_sensor_name(*it_sensor));
			delete cmd_channel;
			goto FAILED;
		}

		sensor_client_info::get_instance().add_command_channel(*it_sensor, cmd_channel);

		if (first_connection) {
			first_connection = false;
			if (!cmd_channel->cmd_get_id(client_id)) {
				_E("Failed to get client id");
				goto FAILED;
			}

			sensor_client_info::get_instance().set_client_id(client_id);
			sensor_event_listener::get_instance().start_event_listener();
		}

		cmd_channel->set_client_id(client_id);

		if (!cmd_channel->cmd_hello(*it_sensor)) {
			_E("Sending cmd_hello(%s, %d) failed for %s", get_sensor_name(*it_sensor), client_id, get_client_name());
			goto FAILED;
		}

		sensor_rep prev_rep, cur_rep;
		prev_rep.active = false;
		prev_rep.option = SENSOR_OPTION_DEFAULT;
		prev_rep.interval = 0;

		sensor_client_info::get_instance().get_sensor_rep(*it_sensor, cur_rep);
		if (!change_sensor_rep(*it_sensor, prev_rep, cur_rep)) {
			_E("Failed to change rep(%s) for %s", get_sensor_name(*it_sensor), get_client_name());
			goto FAILED;
		}

		++it_sensor;
	}

	_I("Succeeded to restore sensor client session for %s", get_client_name());

	return;

FAILED:
	sensor_event_listener::get_instance().clear();
	_E("Failed to restore session for %s", get_client_name());
}

static bool get_events_diff(event_type_vector &a_vec, event_type_vector &b_vec, event_type_vector &add_vec, event_type_vector &del_vec)
{
	sort(a_vec.begin(), a_vec.end());
	sort(b_vec.begin(), b_vec.end());

	set_difference(a_vec.begin(), a_vec.end(), b_vec.begin(), b_vec.end(), back_inserter(del_vec));
	set_difference(b_vec.begin(), b_vec.end(), a_vec.begin(), a_vec.end(), back_inserter(add_vec));

	return !(add_vec.empty() && del_vec.empty());
}


static bool change_sensor_rep(sensor_id_t sensor_id, sensor_rep &prev_rep, sensor_rep &cur_rep)
{
	int client_id;
	command_channel *cmd_channel;
	event_type_vector add_event_types, del_event_types;

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("client %s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false, "Invalid client id : %d, %s, %s", client_id, get_sensor_name(sensor_id), get_client_name());

	get_events_diff(prev_rep.event_types, cur_rep.event_types, add_event_types, del_event_types);

	if (cur_rep.active) {
		if (prev_rep.option != cur_rep.option) {
			if (!cmd_channel->cmd_set_option(cur_rep.option)) {
				_E("Sending cmd_set_option(%d, %s, %d) failed for %s", client_id, get_sensor_name(sensor_id), cur_rep.option, get_client_name());
				return false;
			}
		}

		if ( (prev_rep.interval != cur_rep.interval) || (prev_rep.latency != cur_rep.latency)) {
			if (!cmd_channel->cmd_set_batch(cur_rep.interval, cur_rep.latency)) {
				_E("Sending cmd_set_batch(%d, %s, %d, %d) failed for %s", client_id, get_sensor_name(sensor_id), cur_rep.interval, cur_rep.latency, get_client_name());
				return false;
			}
		}

		if (!add_event_types.empty()) {
			if (!cmd_channel->cmd_register_events(add_event_types)) {
				_E("Sending cmd_register_events(%d, add_event_types) failed for %s", client_id, get_client_name());
				return false;
			}
		}

	}

	if (prev_rep.active && !del_event_types.empty()) {
		if (!cmd_channel->cmd_unregister_events(del_event_types)) {
			_E("Sending cmd_unregister_events(%d, del_event_types) failed for %s", client_id, get_client_name());
			return false;
		}
	}

	if (prev_rep.active != cur_rep.active) {
		if (cur_rep.active) {
			if (!cmd_channel->cmd_start()) {
				_E("Sending cmd_start(%d, %s) failed for %s", client_id, get_sensor_name(sensor_id), get_client_name());
				return false;
			}
		} else {
			if (!cmd_channel->cmd_unset_batch()) {
				_E("Sending cmd_unset_interval(%d, %s) failed for %s", client_id, get_sensor_name(sensor_id), get_client_name());
				return false;
			}

			if (!cmd_channel->cmd_stop()) {
				_E("Sending cmd_stop(%d, %s) failed for %s", client_id, get_sensor_name(sensor_id), get_client_name());
				return false;
			}
		}
	}

	return true;
}

static bool get_sensor_list(void)
{
	static cmutex l;
	static bool init = false;

	AUTOLOCK(l);

	if (!init) {
		command_channel cmd_channel;

		if (!cmd_channel.create_channel()) {
			_E("%s failed to create command channel", get_client_name());
			return false;
		}

		if (!cmd_channel.cmd_get_sensor_list())
			return false;

		init = true;
	}

	return true;
}

API bool sensord_get_sensor_list(sensor_type_t type, sensor_t **list, int *sensor_count)
{
	retvm_if (!get_sensor_list(), false, "Fail to get sensor list from server");

	vector<sensor_info *> sensor_infos = sensor_info_manager::get_instance().get_infos(type);

	if (!sensor_infos.empty()) {
		*list = (sensor_t *) malloc(sizeof(sensor_info *) * sensor_infos.size());
		retvm_if(!*list, false, "Failed to allocate memory");
	}

	for (unsigned int i = 0; i < sensor_infos.size(); ++i)
		*(*list + i) = sensor_info_to_sensor(sensor_infos[i]);

	*sensor_count = sensor_infos.size();

	return true;
}

API sensor_t sensord_get_sensor(sensor_type_t type)
{
	retvm_if (!get_sensor_list(), NULL, "Fail to get sensor list from server");

	const sensor_info *info;

	info = sensor_info_manager::get_instance().get_info(type);

	return sensor_info_to_sensor(info);
}

API bool sensord_get_type(sensor_t sensor, sensor_type_t *type)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !type,
		NULL, "Invalid param: sensor (%p), type(%p)", sensor, type);

	*type = info->get_type();

	return true;
}

API const char* sensord_get_name(sensor_t sensor)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info),
		NULL, "Invalid param: sensor (%p)", sensor);

	return info->get_name();
}

API const char* sensord_get_vendor(sensor_t sensor)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info),
		NULL, "Invalid param: sensor (%p)", sensor);

	return info->get_vendor();
}

API bool sensord_get_privilege(sensor_t sensor, sensor_privilege_t *privilege)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !privilege,
		false, "Invalid param: sensor (%p), privilege(%p)", sensor, privilege);

	*privilege = info->get_privilege();

	return true;
}

API bool sensord_get_min_range(sensor_t sensor, float *min_range)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !min_range,
		false, "Invalid param: sensor (%p), min_range(%p)", sensor, min_range);

	*min_range = info->get_min_range();

	return true;
}

API bool sensord_get_max_range(sensor_t sensor, float *max_range)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !max_range,
		false, "Invalid param: sensor (%p), max_range(%p)", sensor, max_range);

	*max_range = info->get_max_range();

	return true;
}

API bool sensord_get_resolution(sensor_t sensor, float *resolution)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !resolution,
		false, "Invalid param: sensor (%p), resolution(%p)", sensor, resolution);

	*resolution = info->get_resolution();

	return true;
}

API bool sensord_get_min_interval(sensor_t sensor, int *min_interval)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !min_interval,
		false, "Invalid param: sensor (%p), min_interval(%p)", sensor, min_interval);

	*min_interval = info->get_min_interval();

	return true;
}

API bool sensord_get_fifo_count(sensor_t sensor, int *fifo_count)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !fifo_count,
		false, "Invalid param: sensor (%p), fifo_count(%p)", sensor, fifo_count);

	*fifo_count = info->get_fifo_count();

	return true;
}

API bool sensord_get_max_batch_count(sensor_t sensor, int *max_batch_count)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !max_batch_count,
		false, "Invalid param: sensor (%p), max_batch_count(%p)", sensor, max_batch_count);

	*max_batch_count = info->get_max_batch_count();

	return true;
}

API bool sensord_get_supported_event_types(sensor_t sensor, unsigned int **event_types, int *count)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !event_types || !count,
		false, "Invalid param: sensor (%p), event_types(%p), count(%p)", sensor, event_types, count);

	unsigned int event_type;
	event_type = info->get_supported_event();
	*event_types = (unsigned int *)malloc(sizeof(unsigned int));

	retvm_if(!*event_types, false, "Failed to allocate memory");

	(*event_types)[0] = event_type;
	*count = 1;

	return true;
}

API bool sensord_is_supported_event_type(sensor_t sensor, unsigned int event_type, bool *supported)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info) || !event_type || !supported,
		false, "Invalid param: sensor (%p), event_type(%p), supported(%p)", sensor, event_type, supported);

	*supported = info->is_supported_event(event_type);

	return true;
}

API bool sensord_is_wakeup_supported(sensor_t sensor)
{
	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info),
		false, "Invalid param: sensor (%p)", sensor);

	return info->is_wakeup_supported();
}

API int sensord_connect(sensor_t sensor)
{
	command_channel *cmd_channel = NULL;
	int handle;
	int client_id;
	bool sensor_registered;
	bool first_connection = false;

	sensor_info* info = sensor_to_sensor_info(sensor);

	retvm_if (!sensor_info_manager::get_instance().is_valid(info),
		OP_ERROR, "Invalid param: sensor (%p)", sensor);

	sensor_id_t sensor_id = info->get_id();

	AUTOLOCK(lock);

	sensor_registered = sensor_client_info::get_instance().is_sensor_registered(sensor_id);

	// lazy loading after creating static variables
	atexit(good_bye);

	handle = sensor_client_info::get_instance().create_handle(sensor_id);
	if (handle == MAX_HANDLE_REACHED) {
		_E("Maximum number of handles reached, sensor: %s in client %s", get_sensor_name(sensor_id), get_client_name());
		return OP_ERROR;
	}

	if (!sensor_registered) {
		cmd_channel = new(std::nothrow) command_channel();
		if (!cmd_channel) {
			_E("Failed to allocated memory");
			sensor_client_info::get_instance().delete_handle(handle);
			return OP_ERROR;
		}

		if (!cmd_channel->create_channel()) {
			_E("%s failed to create command channel for %s", get_client_name(), get_sensor_name(sensor_id));
			sensor_client_info::get_instance().delete_handle(handle);
			delete cmd_channel;
			return OP_ERROR;
		}

		sensor_client_info::get_instance().add_command_channel(sensor_id, cmd_channel);
	}

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("%s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		sensor_client_info::get_instance().delete_handle(handle);
		return OP_ERROR;
	}

	if (!sensor_client_info::get_instance().has_client_id()) {
		first_connection = true;
		if(!cmd_channel->cmd_get_id(client_id)) {
			_E("Sending cmd_get_id() failed for %s", get_sensor_name(sensor_id));
			sensor_client_info::get_instance().close_command_channel(sensor_id);
			sensor_client_info::get_instance().delete_handle(handle);
			return OP_ERROR;
		}

		sensor_client_info::get_instance().set_client_id(client_id);
		_I("%s gets client_id [%d]", get_client_name(), client_id);
		sensor_event_listener::get_instance().start_event_listener();
		_I("%s starts listening events with client_id [%d]", get_client_name(), client_id);
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	cmd_channel->set_client_id(client_id);

	_I("%s[%d] connects with %s[%d]", get_client_name(), client_id, get_sensor_name(sensor_id), handle);

	sensor_client_info::get_instance().set_sensor_params(handle, SENSOR_STATE_STOPPED, SENSOR_OPTION_DEFAULT);

	if (!sensor_registered) {
		if(!cmd_channel->cmd_hello(sensor_id)) {
			_E("Sending cmd_hello(%s, %d) failed for %s", get_sensor_name(sensor_id), client_id, get_client_name());
			sensor_client_info::get_instance().close_command_channel(sensor_id);
			sensor_client_info::get_instance().delete_handle(handle);
			if (first_connection) {
				sensor_client_info::get_instance().set_client_id(CLIENT_ID_INVALID);
				sensor_event_listener::get_instance().stop_event_listener();
			}
			return OP_ERROR;
		}
	}

	set_power_save_state_cb();
	return handle;
}

API bool sensord_disconnect(int handle)
{
	command_channel *cmd_channel;
	sensor_id_t sensor_id;
	int client_id;
	int sensor_state;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_state(handle, sensor_state)||
		!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("client %s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false, "Invalid client id : %d, handle: %d, %s, %s", client_id, handle, get_sensor_name(sensor_id), get_client_name());

	_I("%s disconnects with %s[%d]", get_client_name(), get_sensor_name(sensor_id), handle);

	if (sensor_state != SENSOR_STATE_STOPPED) {
		_W("%s[%d] for %s is not stopped before disconnecting.",
			get_sensor_name(sensor_id), handle, get_client_name());
		sensord_stop(handle);
	}

	if (!sensor_client_info::get_instance().delete_handle(handle))
		return false;

	if (!sensor_client_info::get_instance().is_active())
		sensor_client_info::get_instance().set_client_id(CLIENT_ID_INVALID);

	if (!sensor_client_info::get_instance().is_sensor_registered(sensor_id)) {
		if(!cmd_channel->cmd_byebye()) {
			_E("Sending cmd_byebye(%d, %s) failed for %s", client_id, get_sensor_name(sensor_id), get_client_name());
			return false;
		}
		sensor_client_info::get_instance().close_command_channel(sensor_id);
	}

	if (!sensor_client_info::get_instance().is_active()) {
		_I("Stop listening events for client %s with client id [%d]", get_client_name(), sensor_client_info::get_instance().get_client_id());
		sensor_event_listener::get_instance().stop_event_listener();
	}

	unset_power_save_state_cb();

	return true;
}


static bool register_event(int handle, unsigned int event_type, unsigned int interval, int max_batch_latency, int cb_type, void* cb, void *user_data)
{
	sensor_id_t sensor_id;
	sensor_rep prev_rep, cur_rep;
	bool ret;

	retvm_if (!cb, false, "callback is NULL");

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	if (interval == 0)
		interval = DEFAULT_INTERVAL;

	_I("%s registers event %s[0x%x] for sensor %s[%d] with interval: %d, latency: %d,  cb: 0x%x, user_data: 0x%x",
		get_client_name(), get_event_name(event_type), event_type, get_sensor_name(sensor_id),
		handle, interval, max_batch_latency, cb, user_data);

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);
	sensor_client_info::get_instance().register_event(handle, event_type, interval, max_batch_latency, cb_type, cb, user_data);
	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);
	ret = change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().unregister_event(handle, event_type);

	return ret;
}

API bool sensord_register_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensor_cb_t cb, void *user_data)
{
	return register_event(handle, event_type, interval, max_batch_latency, SENSOR_EVENT_CB, (void *)cb, user_data);
}

API bool sensord_register_hub_event(int handle, unsigned int event_type, unsigned int interval, unsigned int max_batch_latency, sensorhub_cb_t cb, void *user_data)
{
	return register_event(handle, event_type, interval, max_batch_latency, SENSORHUB_EVENT_CB, (void *)cb, user_data);
}


API bool sensord_unregister_event(int handle, unsigned int event_type)
{
	sensor_id_t sensor_id;
	sensor_rep prev_rep, cur_rep;
	bool ret;
	unsigned int prev_interval, prev_latency;
	int prev_cb_type;
	void *prev_cb;
	void *prev_user_data;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	_I("%s unregisters event %s[0x%x] for sensor %s[%d]", get_client_name(), get_event_name(event_type),
		event_type, get_sensor_name(sensor_id), handle);

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);
	sensor_client_info::get_instance().get_event_info(handle, event_type, prev_interval, prev_latency, prev_cb_type, prev_cb, prev_user_data);

	if (!sensor_client_info::get_instance().unregister_event(handle, event_type)) {
		_E("%s try to unregister non registered event %s[0x%x] for sensor %s[%d]",
			get_client_name(),get_event_name(event_type), event_type, get_sensor_name(sensor_id), handle);
		return false;
	}

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);
	ret =  change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().register_event(handle, event_type, prev_interval, prev_latency, prev_cb_type, prev_cb, prev_user_data);

	return ret;

}


API bool sensord_register_accuracy_cb(int handle, sensor_accuracy_changed_cb_t cb, void *user_data)
{
	sensor_id_t sensor_id;

	retvm_if (!cb, false, "callback is NULL");

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}


	_I("%s registers accuracy_changed_cb for sensor %s[%d] with cb: 0x%x, user_data: 0x%x",
		get_client_name(), get_sensor_name(sensor_id), handle, cb, user_data);

	sensor_client_info::get_instance().register_accuracy_cb(handle, cb , user_data);

	return true;

}

API bool sensord_unregister_accuracy_cb(int handle)
{
	sensor_id_t sensor_id;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}


	_I("%s unregisters accuracy_changed_cb for sensor %s[%d]",
		get_client_name(), get_sensor_name(sensor_id), handle);

	sensor_client_info::get_instance().unregister_accuracy_cb(handle);

	return true;
}


API bool sensord_start(int handle, int option)
{
	sensor_id_t sensor_id;
	sensor_rep prev_rep, cur_rep;
	bool ret;
	int prev_state, prev_option;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	retvm_if ((option < 0) || (option >= SENSOR_OPTION_END), false, "Invalid option value : %d, handle: %d, %s, %s",
		option, handle, get_sensor_name(sensor_id), get_client_name());

	_I("%s starts %s[%d], with option: %d, power save state: %d", get_client_name(), get_sensor_name(sensor_id),
		handle, option, g_power_save_state);

	if (g_power_save_state && !(g_power_save_state & option)) {
		sensor_client_info::get_instance().set_sensor_params(handle, SENSOR_STATE_PAUSED, option);
		return true;
	}

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);
	sensor_client_info::get_instance().get_sensor_params(handle, prev_state, prev_option);
	sensor_client_info::get_instance().set_sensor_params(handle, SENSOR_STATE_STARTED, option);
	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);

	ret = change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().set_sensor_params(handle, prev_state, prev_option);

	return ret;
}

API bool sensord_stop(int handle)
{
	sensor_id_t sensor_id;
	int sensor_state;
	bool ret;
	int prev_state, prev_option;

	sensor_rep prev_rep, cur_rep;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_state(handle, sensor_state)||
		!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	retvm_if ((sensor_state == SENSOR_STATE_STOPPED), true, "%s already stopped with %s[%d]",
		get_client_name(), get_sensor_name(sensor_id), handle);


	_I("%s stops sensor %s[%d]", get_client_name(), get_sensor_name(sensor_id), handle);

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);
	sensor_client_info::get_instance().get_sensor_params(handle, prev_state, prev_option);
	sensor_client_info::get_instance().set_sensor_state(handle, SENSOR_STATE_STOPPED);
	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);

	ret = change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().set_sensor_params(handle, prev_state, prev_option);

	return ret;
}


static bool change_event_batch(int handle, unsigned int event_type, unsigned int interval, unsigned int latency)
{
	sensor_id_t sensor_id;
	sensor_rep prev_rep, cur_rep;
	bool ret;
	unsigned int prev_interval, prev_latency;
	int prev_cb_type;
	void *prev_cb;
	void *prev_user_data;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	if (interval == 0)
		interval = DEFAULT_INTERVAL;

	_I("%s changes batch of event %s[0x%x] for %s[%d] to (%d, %d)", get_client_name(), get_event_name(event_type),
			event_type, get_sensor_name(sensor_id), handle, interval, latency);

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);

	sensor_client_info::get_instance().get_event_info(handle, event_type, prev_interval, prev_latency, prev_cb_type, prev_cb, prev_user_data);

	if (!sensor_client_info::get_instance().set_event_batch(handle, event_type, interval, latency))
		return false;

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);

	ret = change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().set_event_batch(handle, event_type, prev_interval, prev_latency);

	return ret;
}

API bool sensord_change_event_interval(int handle, unsigned int event_type, unsigned int interval)
{
	unsigned int prev_interval, prev_latency;
	int prev_cb_type;
	void *prev_cb;
	void *prev_user_data;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_event_info(handle, event_type, prev_interval, prev_latency, prev_cb_type, prev_cb, prev_user_data)) {
		_E("Failed to get event info with handle = %d, event_type = 0x%x", handle, event_type);
		return false;
	}

	_I("handle = %d, event_type = 0x%x, interval = %d, prev_latency = %d", handle, event_type, interval, prev_latency);
	return change_event_batch(handle, event_type, interval, prev_latency);
}

API bool sensord_change_event_max_batch_latency(int handle, unsigned int event_type, unsigned int max_batch_latency)
{
	unsigned int prev_interval, prev_latency;
	int prev_cb_type;
	void *prev_cb;
	void *prev_user_data;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_event_info(handle, event_type, prev_interval, prev_latency, prev_cb_type, prev_cb, prev_user_data)) {
		_E("Failed to get event info with handle = %d, event_type = 0x%x", handle, event_type);
		return false;
	}

	return change_event_batch(handle, event_type, prev_interval, max_batch_latency);
}

API bool sensord_change_event_maincontext(int handle, unsigned int event_type, GMainContext *maincontext)
{
	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().set_event_maincontext(handle, event_type, maincontext)) {
		_E("Failed to get event info with handle = %d, event_type = 0x%x, maincontext = 0x%x", handle, event_type, maincontext);
		return false;
	}

	_I("handle = %d, event_type = 0x%x, maincontext = 0x%x", handle, event_type, maincontext);
	return true;
}

API bool sensord_set_option(int handle, int option)
{
	sensor_id_t sensor_id;
	sensor_rep prev_rep, cur_rep;
	int sensor_state;
	bool ret;
	int prev_state, prev_option;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_state(handle, sensor_state)||
		!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	retvm_if ((option < 0) || (option >= SENSOR_OPTION_END), false, "Invalid option value : %d, handle: %d, %s, %s",
		option, handle, get_sensor_name(sensor_id), get_client_name());


	sensor_client_info::get_instance().get_sensor_rep(sensor_id, prev_rep);
	sensor_client_info::get_instance().get_sensor_params(handle, prev_state, prev_option);

	if (g_power_save_state) {
		if ((option & g_power_save_state) && (sensor_state == SENSOR_STATE_PAUSED))
			sensor_client_info::get_instance().set_sensor_state(handle, SENSOR_STATE_STARTED);
		else if (!(option & g_power_save_state) && (sensor_state == SENSOR_STATE_STARTED))
			sensor_client_info::get_instance().set_sensor_state(handle, SENSOR_STATE_PAUSED);
	}
	sensor_client_info::get_instance().set_sensor_option(handle, option);

	sensor_client_info::get_instance().get_sensor_rep(sensor_id, cur_rep);
	ret =  change_sensor_rep(sensor_id, prev_rep, cur_rep);

	if (!ret)
		sensor_client_info::get_instance().set_sensor_option(handle, prev_option);

	return ret;

}

API bool sensord_set_wakeup(int handle, int wakeup)
{
	sensor_id_t sensor_id;
	command_channel *cmd_channel;
	int client_id;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	retvm_if ((wakeup != SENSOR_WAKEUP_ON) && (wakeup != SENSOR_WAKEUP_OFF), false, "Invalid wakeup value : %d, handle: %d, %s, %s",
		wakeup, handle, get_sensor_name(sensor_id), get_client_name());

	sensor_client_info::get_instance().set_sensor_wakeup(handle, wakeup);

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("client %s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false, "Invalid client id : %d, handle: %d, %s, %s", client_id, handle, get_sensor_name(sensor_id), get_client_name());

	if (!cmd_channel->cmd_set_wakeup(wakeup)) {
		_E("Sending cmd_set_wakeup(%d, %s, %d) failed for %s", client_id, get_sensor_name(sensor_id), wakeup, get_client_name());
		return false;
	}

	return true;
}

API bool sensord_set_attribute_int(int handle, int attribute, int value)
{
	sensor_id_t sensor_id;
	command_channel *cmd_channel;
	int client_id;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("client %s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false,
			"Invalid client id : %d, handle: %d, %s, %s",
			client_id, handle, get_sensor_name(sensor_id), get_client_name());

	if (!cmd_channel->cmd_set_attribute_int(attribute, value)) {
		_E("Sending cmd_set_attribute_int(%d, %d) failed for %s",
			client_id, value, get_client_name);
		return false;
	}

	return true;
}

API bool sensord_set_attribute_str(int handle, int attribute, const char *value, int value_len)
{
	sensor_id_t sensor_id;
	command_channel *cmd_channel;
	int client_id;

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("Client %s failed to get handle information", get_client_name());
		return false;
	}

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("Client %s failed to get command channel for %s",
			get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	retvm_if((value_len < 0) || (value == NULL), false,
			"Invalid value_len: %d, value: 0x%x, handle: %d, %s, %s",
			value_len, value, handle, get_sensor_name(sensor_id), get_client_name());

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false,
			"Invalid client id : %d, handle: %d, %s, %s",
			client_id, handle, get_sensor_name(sensor_id), get_client_name());

	if (!cmd_channel->cmd_set_attribute_str(attribute, value, value_len)) {
		_E("Sending cmd_set_attribute_str(%d, %d, 0x%x) failed for %s",
			client_id, value_len, value, get_client_name);
		return false;
	}

	return true;
}

API bool sensord_send_sensorhub_data(int handle, const char *data, int data_len)
{
	return sensord_set_attribute_str(handle, 0, data, data_len);
}

API bool sensord_send_command(int handle, const char *command, int command_len)
{
	return sensord_set_attribute_str(handle, 0, command, command_len);
}

API bool sensord_get_data(int handle, unsigned int data_id, sensor_data_t* sensor_data)
{
	sensor_id_t sensor_id;
	command_channel *cmd_channel;
	int sensor_state;
	int client_id;

	retvm_if ((!sensor_data), false, "sensor_data is NULL");

	AUTOLOCK(lock);

	if (!sensor_client_info::get_instance().get_sensor_state(handle, sensor_state)||
		!sensor_client_info::get_instance().get_sensor_id(handle, sensor_id)) {
		_E("client %s failed to get handle information", get_client_name());
		return false;
	}

	if (!sensor_client_info::get_instance().get_command_channel(sensor_id, &cmd_channel)) {
		_E("client %s failed to get command channel for %s", get_client_name(), get_sensor_name(sensor_id));
		return false;
	}

	client_id = sensor_client_info::get_instance().get_client_id();
	retvm_if ((client_id < 0), false, "Invalid client id : %d, handle: %d, %s, %s", client_id, handle, get_sensor_name(sensor_id), get_client_name());

	if (sensor_state != SENSOR_STATE_STARTED) {
		_E("Sensor %s is not started for client %s with handle: %d, sensor_state: %d", get_sensor_name(sensor_id), get_client_name(), handle, sensor_state);
		return false;
	}

	if(!cmd_channel->cmd_get_data(data_id, sensor_data)) {
		_E("cmd_get_data(%d, %d, 0x%x) failed for %s", client_id, data_id, sensor_data, get_client_name());
		return false;
	}

	return true;

}
