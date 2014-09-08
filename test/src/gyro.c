#include <time.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor.h>
#include <stdbool.h>

static GMainLoop *mainloop;

void callback(unsigned int event_type, sensor_event_data_t *event, void *user_data)
{
	sensor_data_t *data = (sensor_data_t *)event->event_data;
	printf("Gyroscope [%6.6f] [%6.6f] [%6.6f] [%lld]\n\n", data->values[0], data->values[1], data->values[2], data->timestamp);
}

void printformat()
{
	printf("Usage : ./gyroscope <event> <interval>(optional)\n\n");
	printf("event:\n");
	printf("RAW_DATA_REPORT_ON_TIME\n");
	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by gyroscope driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}

int main(int argc,char **argv)
{
	int result, handle;
	unsigned int event;
	bool error_state = FALSE;

	mainloop = g_main_loop_new(NULL, FALSE);
	sensor_type_t type = GYROSCOPE_SENSOR;
	event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
	event_condition->cond_op = CONDITION_EQUAL;
	event_condition->cond_value1=10.52;

	if (argc != 2 && argc != 3) {
		printformat();
		error_state = TRUE;
	}
	else {
		 if (strcmp(argv[1], "RAW_DATA_REPORT_ON_TIME") == 0)
			event = GYROSCOPE_EVENT_RAW_DATA_REPORT_ON_TIME;

		 else {
			printformat();
			error_state = TRUE;
		}

		if(argc == 3)
			event_condition->cond_value1 = atof(argv[2]);
	}

	if (!error_state) {
		handle = sf_connect(type);
		result = sf_register_event(handle, event, event_condition, callback, NULL);

		if (result < 0)
			printf("Can't register gyroscope\n");

		if (!(sf_start(handle,0) < 0)) {
			printf("Success start \n");
		}
		else {
			printf("Error\n\n\n\n");
			sf_unregister_event(handle, event);
			sf_disconnect(handle);
			return -1;
		}

		g_main_loop_run(mainloop);
		g_main_loop_unref(mainloop);

		sf_unregister_event(handle, event);

		if (!(sf_stop(handle) < 0))
			printf("Success stop \n");

		sf_disconnect(handle);
	}

	free(event_condition);

	return 0;
}

