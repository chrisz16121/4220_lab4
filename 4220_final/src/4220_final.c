/*
 ============================================================================
 Name        : 4220_final.c
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#define MSG_SIZE 40

void* thread_function_gather_time(void*);
void* thread_function_data_out(void*);
void* thread_function_update(void*);

typedef struct info_time{
	unsigned int second,minute,hour;
	double total_time;
	int milli;
}time_log;
typedef struct info_log{
	int id_number;
	int status_sw_1,status_sw_2,status_sw3,status_LED_1,status_LED_2;
	double line_voltage;
}log;

time_log current_time;

int main(int argc,char* argv[]) {
	//struct timeval stamp;
	pthread_t data_out_thread,time_thread,RTU_1_thread,RTU_2_thread;

	int i,temp;

	if(argc<2){
		printf("No port for network communication provided, must exit\n");
		return 0;
	}
	pthread_create(&time_thread,NULL,thread_function_gather_time,NULL);
	pthread_create(&data_out_thread,NULL,thread_function_data_out,NULL);
	printf("Welcome to the operator/ historian of the final project\n"
			"please input data at any time format: @*Board_Number*:*Operation\n");

	//begin network communication block of code
	int socket_fd,n;
	unsigned int length;
	//socklen_t to_RTU_len;
	struct sockaddr_in controller;
	struct sockaddr_in RTU;
	struct hostent* hp;
	char buf[MSG_SIZE];

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0){
		printf("Error opening socket\n");
		return 0;
	}
	RTU.sin_family = AF_INET;
	hp = gethostbyname("128.206.16.15");
	if(hp == 0){
		printf("Error: unknown host\n");
		return 0;
	}
	bcopy((char *)hp->h_addr, (char *)&RTU.sin_addr, hp->h_length);
    RTU.sin_port = htons(atoi(argv[1]));	// port field
	length = sizeof(struct sockaddr_in);		// size of structure
	printf("Enter the test message\n");
	scanf("%s",buf);					//set the elements to zero
	//controller.sin_family = AF_INET;			// symbol constant for Internet domain
	//controller.sin_addr.s_addr = INADDR_ANY;	// IP address of the machine
	//controller.sin_port = htons(atoi(argv[1]));	// port number
	//if (bind(socket_fd, (struct sockaddr *)&controller, length) < 0){
		//printf("Error binding socket\n");
		//return 0;
	//}
	//length = sizeof(struct sockaddr_in);
	n = sendto(socket_fd,buf,strlen(buf),0,(const struct sockaddr *)&RTU,length);
	if(n < 0){
		printf("Error sending data\n");
		return 0;
	}
	while(1){
		for(i=0;i<10000;i++){

		}
	}
	return 0;
}
void* thread_function_update(void* parameters){
	pthread_exit(NULL);
}
void* thread_function_data_out(void* parameters){
	char* string_out;
	while(1){
		scanf("%s",string_out);
		printf("%d:%d:%d:%d\n",current_time.hour,current_time.minute,current_time.second,current_time.milli);
		printf("%s needs to be sent to a board\n",string_out);
	}
	pthread_exit(NULL);
}
void* thread_function_gather_time(void* parameters){
	int temp;
	struct timeval stamp;
	while(1){
		gettimeofday(&stamp,NULL);
		temp = stamp.tv_sec;
		current_time.second = temp%60;
		temp /= 60;
		current_time.minute = temp%60;
		temp /= 60;
		current_time.hour = temp%24;
		current_time.milli = (stamp.tv_usec)/1000 ;
		current_time.total_time = (double)(stamp.tv_usec/1000 + stamp.tv_sec * 1000);
	}
	pthread_exit(NULL);
}
