#if 0
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sensorctl_log.h>
#include "dbus_util.h"
#include "injector_manager.h"

#define SENSOR_EVENT_INJECT "event"
#define DEV_NODE "/dev/input/event9"

class injector_sensor: public injector {
public:
	bool inject(int option_count, char *options[]);
};

bool injector_sensor::inject(int option_count, char *options[])
{
	struct input_event event, event_sync;

	int fd = open(DEV_NODE, O_RDWR);
	if (fd < 0) {
		_E("Errro open device node:%s\n", strerror(errno));
		return false;
	}

	int fd_data = open("/dev/ssp_sensorhub", O_RDWR);
	if (fd < 0) {
		_E("Errro open device node:%s\n", strerror(errno));
		return false;
	}

	memset(&event, 0, sizeof(event));
	memset(&event_sync, 0, sizeof(event_sync));
	gettimeofday(&event.time, NULL);
	event.type = EV_REL;
	event.code = REL_RY;
	event.value = REL_RY;
	gettimeofday(&event_sync.time, NULL);
	event_sync.type = EV_SYN;
	event_sync.code = SYN_REPORT;
	event_sync.value = 100;
	
	static const char enable_wrist_up[] = {(char)-79, 19, 0, 0};
	static const char disable_wrist_up[] = {(char)-78, 19, 0, 0};
	static const char wrist_up[] = {1, 1, 19, 1};

	int ret = write(fd_data, enable_wrist_up, sizeof(enable_wrist_up));
	if (ret < 0) _E("failed to write to %d\n", fd_data);
	else 		 _E("pass1 %d\n", ret);
	sleep(1);

	ret = write(fd_data, wrist_up, sizeof(wrist_up));
	if (ret < 0) _E("failed to write to %d\n", fd_data);
	else 		 _E("pass2 %d\n", ret);

	ret = write(fd, &event, sizeof(event));
	if (ret < 0) _E("failed to write to %d\n", fd_data);
	else 		 _E("pass3 %d\n", ret);

	ret = write(fd, &event_sync, sizeof(event_sync));
	if (ret < 0) _E("failed to write to %d\n", fd_data);
	else 		 _E("pass4 %d\n", ret);

	sleep(1);
	ret = write(fd_data, disable_wrist_up, sizeof(disable_wrist_up));
	if (ret < 0) _E("failed to write to %d\n", fd_data);
	else 		 _E("pass5 %d\n", ret);

	close(fd);
	close(fd_data);
	return true;
}

REGISTER_INJECTOR(GESTURE_WRIST_UP_SENSOR, SENSOR_EVENT_INJECT, injector_sensor)
#endif
