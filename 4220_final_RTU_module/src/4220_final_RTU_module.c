/*
 ============================================================================
 Name        : 4220_final_RTU_module.c
 Author      : Christopher Smith
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#ifndef MODULE
#define MODULE
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_sem.h>
#include <asm/io.h>
#include <rtai_fifos.h>
#include <asm/io.h>

//#include <linux/semaphore.h>
#include <linux/proc_fs.h>
MODULE_LICENSE("GPL");

RTIME baseP;
static RT_TASK task1;
unsigned long* portA;
unsigned long* pbdr;
unsigned long* pbddr;
typedef struct info_log{
	int id_number;
	int status_sw_1,status_sw_2,status_sw_3,status_LED_1,status_LED_2;
	double line_voltage;
}log;
typedef struct status_log{
	log status_info;
	struct timeval t;
};
struct status_log state;
void realtime_status_process(int);

void realtime_status_process(int a){
	while(1){
		if((*pbdr &= 0x01) == 0x00){
			state.status_info.status_sw_1 = 1;
			printk("Button 1 pushed: %d\n",state.status_info.status_sw_1);
		}
		else state.status_info.status_sw_1 = 0;
		if((*pbdr &= 0x02) == 0x00){
			state.status_info.status_sw_2 = 1;
			printk("Button 2 pushed: %d\n",state.status_info.status_sw_2);
		}
		else state.status_info.status_sw_2 = 0;
		if((*pbdr &= 0x04) == 0x00){
			state.status_info.status_sw_3 = 1;
			printk("Button 3 pushed : %d\n",state.status_info.status_sw_3);
		}
		else state.status_info.status_sw_3 = 0;
		do_gettimeofday(&state.t);
		rtf_put(0,&state,sizeof(state));
		rt_task_wait_period();
		//printk("test\n");
	}
	//NOTE: need to implement voltage level status as well

}

int init_module(void){
	//struct timeval t;
	portA = (unsigned long*)__ioremap(0x80840000,4096,0);
	pbdr = portA + 1;
	pbddr = portA + 5;
	*pbddr &= 0x00;
	*pbddr |= 0xE0;
	*pbdr &=0x00;
	rt_set_periodic_mode();
	baseP = start_rt_timer(nano2count(1000000));
	printk("the initializer\n");
	int e1 = rt_task_init(&task1,(void*)realtime_status_process,0,256,0,0,0);
	if(e1 != 0)
	{
		printk("Could not initialize the real time task, need to exit\n");
		return 0;
	}
	rt_task_make_periodic(&task1,rt_get_time()+0*baseP,1000*baseP);
	rtf_create(0,1*sizeof(state));
	return 0;
}
void cleanup_module(void){
	rt_task_delete(&task1);
	stop_rt_timer();
}
