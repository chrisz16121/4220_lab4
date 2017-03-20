/*
 ============================================================================
 Name        : 4220_lab4.c
 Author      : Chirstopher Smith
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "serial_ece4220.h"
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>

typedef struct data_buffer{
	unsigned int second,minute,hour;
	double total_time;
	int milli;
	int bytes;
	double data;
	int truth;
	int thread_id;
}buffer_arg;

sem_t sem,print_sem;
buffer_arg common_buffer;
void* thread_operation(void*);
void* thread_operation_dynamic(void*);
void* thread_operation_print(void*);

void* thread_operation_print(void* parameters){
	int pipe_id;
	pipe_id = open("tmp/myPIPE",O_RDONLY);
	buffer_arg argument;
	read(pipe_id,&argument,sizeof(argument));
	printf("%d:%d:%d\t%.0lf\n",argument.minute,argument.second,argument.milli,argument.data);
	read(pipe_id,&argument,sizeof(argument));
	printf("%d:%d:%d\t%.0lf\n",argument.minute,argument.second,argument.milli,argument.data);
	read(pipe_id,&argument,sizeof(argument));
	printf("%d:%d:%d\t%.0lf\n",argument.minute,argument.second,argument.milli,argument.data);
	sem_post(&print_sem);
	pthread_exit(NULL);
}

void* thread_operation_dynamic(void* parameters){
	int pipe_id;
	pipe_id = open("/tmp/myPIPE",O_WRONLY);
	buffer_arg previous_event = common_buffer;
	buffer_arg* gps_buffer = (buffer_arg*)parameters;
	buffer_arg gps_event = *gps_buffer;
	//buffer_arg print_info[3];
	while(previous_event.data == common_buffer.data){
		//printf("%d %d\n",previous_event.data,common_buffer.data);
		usleep(100);
	}
	buffer_arg next_event = common_buffer;
	gps_event.data = previous_event.data +
				(((gps_event.total_time - previous_event.total_time)/(next_event.total_time-previous_event.total_time))
				*(next_event.data-previous_event.data));
	sem_wait(&sem);
	pthread_t thread[3];

	printf("Thread : %d\n",gps_event.thread_id);
	sem_wait(&print_sem);
	pthread_create(&thread[0],NULL,thread_operation_print,NULL);
	printf("Previous TS:");//%d:%d:%d\t%.0lf\n",previous_event.minute,previous_event.second,previous_event.milli,previous_event.data);
	write(pipe_id,&previous_event,sizeof(previous_event));
	sem_wait(&print_sem);
	pthread_create(&thread[1],NULL,thread_operation_print,NULL);
	printf("Realtime TS:"); //%d:%d:%d\t%.3lf\n",gps_event.minute,gps_event.second,gps_event.milli,gps_event.data);
	write(pipe_id,&gps_event,sizeof(gps_event));
	sem_wait(&print_sem);
	pthread_create(&thread[2],NULL,thread_operation_print,NULL);
	printf("NextEvent TS:"); //%d:%d:%d\t%.0lf\n",next_event.minute,next_event.second,next_event.milli,next_event.data);
	write(pipe_id,&next_event,sizeof(next_event));
	sem_post(&sem);
	pthread_exit(NULL);

}
void* thread_operation(void* parameters){
	//int read_count;
	int fd_fifo_in = open("/dev/rtf/0",O_RDONLY);
	//buffer_arg* buffer = (buffer_arg*)parameters;
	buffer_arg realtime_buffer;
	int i;
	int temp_realtime;
	struct timeval stamp_realtime;

	//printf("test 0");
	while(1){
		for(i=0;i<4;i++){
			pthread_t threads[4];
			if(read(fd_fifo_in,&stamp_realtime,sizeof(stamp_realtime)) != sizeof(stamp_realtime) ){
				printf("module2userspace read error\n");
				fflush(stdout);
				exit(-1);
			}
			else{
				temp_realtime = stamp_realtime.tv_sec;
				realtime_buffer.second = temp_realtime%60;
				temp_realtime /= 60;
				realtime_buffer.minute = temp_realtime%60;
				temp_realtime /= 60;
				realtime_buffer.hour = temp_realtime%24;
				realtime_buffer.milli = (stamp_realtime.tv_usec)/1000;
				realtime_buffer.thread_id = i;
				realtime_buffer.total_time = (double)(stamp_realtime.tv_usec/1000 + stamp_realtime.tv_sec *1000);
				//printf("\t%d:%d:%d:%d:\t %d\n",
					//	realtime_buffer.hour,realtime_buffer.minute,realtime_buffer.second,realtime_buffer.milli,common_buffer.data);
				pthread_create(&threads[i],NULL,thread_operation_dynamic,(void*)&realtime_buffer);
				//printf("%d:%d:%d:%d:\n",previous.hour,previous.minute,previous.second,previous.milli);
				fflush(stdout);
			}
		}
	}

	pthread_exit(NULL);

}
int main(void)
{
	// Open the serial port. The first argument denotes the serial port number,
	// while the second and third denote the baud rate (of 115200 in this case)
	/*given code*/
	int prt_id = serial_open(0,5,5);
	unsigned char x;
	ssize_t num_bytes;
	unsigned int ReadBuff;

	struct timeval stamp;

	pthread_t thread1;
	int temp;
	sem_init(&sem,0,1);
	sem_init(&print_sem,0,1);
	pthread_create(&thread1,NULL,thread_operation,(void*)&common_buffer);

	while(1)
	{
		//Read data from the serial port. Blocking function
		num_bytes = read(prt_id, &x, 1);
		ReadBuff = (unsigned int)x;
		gettimeofday(&stamp,NULL);
		temp = stamp.tv_sec;
		common_buffer.second = temp%60;
		temp /= 60;
		common_buffer.minute = temp%60;
		temp /= 60;
		common_buffer.hour = temp%24;
		common_buffer.milli = (stamp.tv_usec)/1000 ;
		common_buffer.data = ReadBuff;
		common_buffer.total_time = (double)(stamp.tv_usec/1000 + stamp.tv_sec * 1000);
		//printf("%d:%d:%d:%d: [%d,%u]\n",common_buffer.hour,common_buffer.minute,common_buffer.second,common_buffer.milli,num_bytes,ReadBuff);
		//printf("[%d,%u]\t", num_bytes, x);
		fflush(stdout);
	}
}
