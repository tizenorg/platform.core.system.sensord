/*
 * multi-sensor.c
 *
 *  Created on: Sep 4, 2015
 *      Author: adarsh.sr1
 */
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <sensor_internal.h>
#include <stdbool.h>
#include <sensor_common.h>
#include <unistd.h>
#include <string.h>
#include<pthread.h>
#include "check-sensor.h"
void usage()
{
	printf("Usage : ./multi-sensor -pthread n{number of sensors}\n\n");

	printf("arg[i].sensor_type: ");
	printf("[accelerometer] ");
	printf("[auto_rotation]\n");
	printf("[gyroscope] ");
	printf("[pressure] ");
	printf("[temperature] ");
	printf("[geomagnetic] ");
	printf("[orientation] ");
	printf("[tilt] ");
	printf("[gravity] ");
	printf("[linear_accel] ");
	printf("[rotation_vector] ");
	printf("[geomagnetic_rv] ");
	printf("[gaming_rv] ");
	printf("[ultraviolet] ");
	printf("[light]\n");
	printf("[uncal_gyro]");
	printf("interval:\n");
	printf("The time interval should be entered based on the sampling frequency supported by accelerometer driver on the device in ms.If no value for sensor is entered default value by the driver will be used.\n");
}
int main(int argc , char **argv)
{
	if (argc != 3) {
			printf("Wrong number of arguments\n");
			usage();
			return 0;
		}
	int n= atoi(argv[2]);
	struct arguments arg[n];
 //printf("%d number of thread ",n);
	int i=0,j=0,k=0;

	for(i=0;i<n;i++)
	{
		char temp[20];
			int tmp_int;
		printf("Enter the sensor name :\n");
		scanf("%s",temp);
		    if (strcmp(temp, "accelerometer") == 0) {
				 arg[i].sensor_type = ACCELEROMETER_SENSOR;
				 arg[i].event = ACCELEROMETER_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "auto_rotation") == 0) {
				 arg[i].sensor_type = AUTO_ROTATION_SENSOR;
				 arg[i].event= AUTO_ROTATION_CHANGE_STATE_EVENT;
			}
			else if (strcmp(temp, "gyroscope") == 0) {
				 arg[i].sensor_type = GYROSCOPE_SENSOR;
				 arg[i].event = GYROSCOPE_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "pressure") == 0) {
				 arg[i].sensor_type = PRESSURE_SENSOR;
				 arg[i].event = PRESSURE_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "temperature") == 0) {
				 arg[i].sensor_type = TEMPERATURE_SENSOR;
				 arg[i].event = TEMPERATURE_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "geomagnetic") == 0) {
				 arg[i].sensor_type = GEOMAGNETIC_SENSOR;
				 arg[i].event = GEOMAGNETIC_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "orientation") == 0) {
				 arg[i].sensor_type = ORIENTATION_SENSOR;
				 arg[i].event = ORIENTATION_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "tilt") == 0) {
				 arg[i].sensor_type = TILT_SENSOR;
				 arg[i].event = TILT_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "gravity") == 0) {
				 arg[i].sensor_type = GRAVITY_SENSOR;
				 arg[i].event = GRAVITY_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "linear_accel") == 0) {
				 arg[i].sensor_type = LINEAR_ACCEL_SENSOR;
				 arg[i].event = LINEAR_ACCEL_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "rotation_vector") == 0) {
				 arg[i].sensor_type = ROTATION_VECTOR_SENSOR;
				 arg[i].event = ROTATION_VECTOR_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "geomagnetic_rv") == 0) {
				 arg[i].sensor_type = GEOMAGNETIC_RV_SENSOR;
				 arg[i].event = GEOMAGNETIC_RV_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "gaming_rv") == 0) {
				 arg[i].sensor_type = GAMING_RV_SENSOR;
				 arg[i].event = GAMING_RV_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "light") == 0) {
				 arg[i].sensor_type = LIGHT_SENSOR;
				 arg[i].event = LIGHT_LUX_DATA_EVENT;
			}
			else if (strcmp(temp, "proximity") == 0) {
				 arg[i].sensor_type = PROXIMITY_SENSOR;
				 arg[i].event = PROXIMITY_CHANGE_STATE_EVENT;
			}
			else if (strcmp(temp, "ultraviolet") == 0) {
				 arg[i].sensor_type = ULTRAVIOLET_SENSOR;
				 arg[i].event = ULTRAVIOLET_RAW_DATA_EVENT;
			}
			else if (strcmp(temp, "uncal_gyro") == 0) {
				 arg[i].sensor_type = UNCAL_GYROSCOPE_SENSOR;
				 arg[i].event = UNCAL_GYRO_RAW_DATA_EVENT;
			}


		/*strcpy(arg[i].arg[i].sensor_type,temp);
		printf("Enter the event type :\n")
		scanf("%s",temp);
		arg[i].event = get_event(arg[i].arg[i].sensor_type,temp);*/
		//printf("%d\n",arg[i].sensor_type);
		printf("Enter the interval :\n");
	    scanf("%d",&tmp_int);
		arg[i].interval =  tmp_int ;

		//printf("%d\n",arg[i].interval);
	}
	pthread_t thread_id[n];

	for(j=0;j<n;j++)
	{
		pthread_create(&thread_id[j],NULL,check_sensor,(void*)&arg[j]);
	}
	for(k=0;k<n;k++)
	{ pthread_join(thread_id[k],NULL);}


	}



