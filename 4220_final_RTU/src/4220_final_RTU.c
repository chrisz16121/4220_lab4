/*
 ============================================================================
 Name        : 4220_final_RTU.c
 Author      : Christopher Smith
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MSG_SIZE 40

void* thread_function_receive_message(void*);

typedef struct info_log{
	int id_number;
	int status_sw_1,status_sw_2,status_sw_3,status_LED_1,status_LED_2;
	double line_voltage;
}log;
typedef struct status_log{
	log status_info;
	struct timeval t;
};
typedef struct info_time{
	unsigned int second,minute,hour;
	double total_time;
	int milli;
}time_log;
struct status_log state;

int main(int argc,char* argv[]){
	log update;
	int temp;
	time_log timestamp;
	struct timeval stamp;
	pthread_t thread_receive_message;
	int fd_fifo_in = open("/dev/rtf/0",O_RDONLY);
	if(argc < 2){
		printf("No port number provided, must exit\n");
		return 0;
	}

	pthread_create(&thread_receive_message,NULL,thread_function_receive_message,(void*)argv[1]);
	while(1){
		if(read(fd_fifo_in,&state,sizeof(state)) != sizeof(state)){
			printf("error in reading from status FIFO\n");
			return 1;
		}
		gettimeofday(&stamp,NULL);
		temp = stamp.tv_sec;
		timestamp.second = temp%60;
		temp /= 60;
		timestamp.minute = temp%60;
		temp /= 60;
		timestamp.hour = temp%24;
		timestamp.milli = (stamp.tv_usec)/1000 ;
		timestamp.total_time = (double)(stamp.tv_usec/1000 + stamp.tv_sec * 1000);
		update = state.status_info;
		printf("%d:%d:%d:%d\nSwitch 1: %d\tSwitch 2: %d\tSwitch 3: %d\nLED 1: %d\tLED 2: %d\n",
				timestamp.hour,timestamp.minute,timestamp.second,timestamp.milli,
				update.status_sw_1,update.status_sw_2,update.status_sw_3,update.status_LED_1,update.status_LED_2);

	}
	return 0;
}
void* thread_function_receive_message(void* parameters){
	char* portNum = (char*)parameters;
	int socket_fd, length, n;
	socklen_t from_controller_len;
	struct sockaddr_in controller;
	struct sockaddr_in RTU;
	char buf[MSG_SIZE];
	printf("the port number is %s\n",portNum);

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0){
		printf("Error opening socket\n");
		return 0;
	}
	length = sizeof(RTU);				// length of structure
	bzero(&RTU,length);					//set the elements to zero
	RTU.sin_family = AF_INET;			// symbol constant for Internet domain
	RTU.sin_addr.s_addr = INADDR_ANY;	// IP address of the machine
	RTU.sin_port = htons(*portNum);	// port number
	if (bind(socket_fd, (struct sockaddr *)&RTU, length) < 0){
		printf("Error binding socket\n");
		return 0;
	}
	from_controller_len = sizeof(struct sockaddr_in);
	while(1){
		bzero(buf,MSG_SIZE);
		n = recvfrom(socket_fd, buf, MSG_SIZE, 0, (struct sockaddr *)&controller, &from_controller_len);
		if(n < 0){
			printf("Error receiving data from controller\n");
			return 0;
		}
		printf("received a message, it says: %s\n",buf);
	}
	pthread_exit(NULL);
}
