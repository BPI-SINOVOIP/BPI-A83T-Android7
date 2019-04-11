/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 cpu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : time_ticks.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-7-9
* Descript: cpu time ticks.
* Update  : date                auther      ver     notes
*           2012-7-9 14:20:22   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "cpu_i.h"

static HANDLE htimer = NULL;
volatile static u32 time_ticks = 0;
struct softtimer *softtimer[SOFTTIMER_NUMBER];

static s32 timer_tick_server(void *parg)
{
	u32 i;

	time_ticks++;

	for (i = 0; (i < SOFTTIMER_NUMBER) && softtimer[i]; i++) {
		if (softtimer[i]->expires > 0) {
			softtimer[i]->expires--;
		} else if (softtimer[i]->expires == 0) {
			softtimer[i]->expires = softtimer[i]->cycle;
			if(softtimer[i]->cb)
				(softtimer[i]->cb)(softtimer[i]->arg);
		}
	}

	return OK;
}

u32 current_time_tick(void)
{
	return time_ticks;
}

u32 mecs_to_ticks(u32 ms)
{
	return (TICK_PER_SEC * ms) / 1000;
}

s32 time_ticks_init(void)
{
	//startup period timer for system timer tick
	htimer = timer_request(timer_tick_server, NULL);
	if (htimer)
	{
		u32 period;

		//start sysem time tick. period base on ms.
		time_ticks = 0;
		period = 1000 / TICK_PER_SEC;
		timer_start(htimer, period, TIMER_MODE_PERIOD);

		INF("setup timer server succeeded\n");
		return OK;
	}
	//request timer failed
	return -EFAIL;
}
