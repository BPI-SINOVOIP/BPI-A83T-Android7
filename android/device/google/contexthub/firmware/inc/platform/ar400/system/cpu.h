/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                cpu  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : cpu.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-2
* Descript: cpu operations public header.
* Update  : date                auther      ver     notes
*           2012-5-2 13:29:32   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __CPU_H__
#define __CPU_H__

//cache operation fucntion
void cpu_flush_cache(void);

//enable and disable interrupt
s32  cpu_disable_int(void);
void cpu_enable_int(int cpsr);

//cpu initialize
void cpu_init(void);

//system exit
void exit(int i);

//system time ticks
s32 time_ticks_init(void);
u32 mecs_to_ticks(u32 ms);
u32 current_time_tick(void);

//build icache coherent
void icache_coherent(void);

#endif  //__CPU_H__
