/*
 ============================================================================
 Name        : 4220_lab4_module.c
 Author      : Chirstopher Smith
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
#define FIFO_SIZE 10



MODULE_LICENSE("GPL");

SEM sema;
RTIME baseP;
static RT_TASK task1;
unsigned long* portA;
unsigned long* pbdr;
unsigned long* pbddr;

void realtime_button_process(int);

void realtime_button_process(int a){
	struct timeval stamp;
	//int temp;
	while(1)
	{
		if((*pbdr &= 0x01) == 0){
			do_gettimeofday(&stamp);
			//printk("%d:%d:%d:%d:\n",buffer.hour,buffer.minute,buffer.second,buffer.milli);
			if(rtf_put(0,&stamp,sizeof(stamp)) != sizeof(stamp)){
				printk("FIFO write error\n");
				return;
			}
		}
		rt_task_wait_period();
	}
	return;
}
int init_module(void){
	portA = (unsigned long*)__ioremap(0x80840000,4096,0);
	pbdr = portA + 1;
	pbddr = portA + 5;
	*pbddr &= 0x00;
	*pbddr |= 0xE0;
	*pbdr &=0x00;
	struct timeval t;
	rt_set_periodic_mode();
	baseP = start_rt_timer(nano2count(1000000));

	int e1 = rt_task_init(&task1,(void*)realtime_button_process,0,256,0,0,0);
	if(e1 != 0)
	{
		printk("Could not initialize the real time task # 1, need to exit\n");
		return 0;
	}
	rt_task_make_periodic(&task1,rt_get_time()+0*baseP,75*baseP);
	rtf_create(0,1*sizeof(t));
	return 0;
}
int cleanup_module(void){
	*pbdr &= 0x00;
	rt_task_delete(&task1);
	stop_rt_timer();
	rtf_destroy(0);
	printk("end of cleanup\n");
	return 0;
}
