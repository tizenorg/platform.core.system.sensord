
#include <time.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor.h>

#define INTER 100.00

static GMainLoop *mainloop;

void callback( unsigned int event_type ,sensor_event_data_t *event, void *user_data)
{
        sensor_data_t *data = (sensor_data_t *)event->event_data;
        printf("Accelerometer [%6.6f] [%6.6f] [%6.6f] [%lld]\n\n", data->values[0],data->values[1],data->values[2],data->timestamp);
}


int main()
{
       	int result;
        int handle;

       	mainloop = g_main_loop_new(NULL, FALSE);             

       	sensor_type_t type = ACCELEROMETER_SENSOR;
        event_condition_t *event_condition = (event_condition_t*) malloc(sizeof(event_condition_t));
        event_condition->cond_op = CONDITION_EQUAL;
        event_condition->cond_value1 = INTER;

        handle= sf_connect(type);	
        result=sf_register_event(handle,ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME,event_condition, callback, NULL); 
        if(result<0)
        printf("Can't register accelerometer\n");       

        if(!(sf_start(handle,0) <0))
        {
	printf("Success start \n");
        }
        else
        {
	printf("Error\n\n\n\n");
	sf_unregister_event(handle,ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);
	sf_disconnect(handle);
	return -1;
        }		

        g_main_loop_run(mainloop);
        g_main_loop_unref(mainloop);
	
        sf_unregister_event(handle,ACCELEROMETER_EVENT_RAW_DATA_REPORT_ON_TIME);      
        if(!(sf_stop(handle)<0))
	printf("Success stop \n");

        sf_disconnect(handle);
        return 0;
}
