/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : intc_i.h
* By      : Superm
* Version : v1.0
* Date    : 2012-5-3
* Descript: interrupt controller internal header.
* Update  : date                auther      ver     notes
*           2012-5-3 13:27:40   Superm       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __INTC_I_H__
#define __INTC_I_H__

#include <cpu.h>
#include <plat/inc/cmsis.h>
#include <plat/inc/include.h>

//local functions
int intc_init(void);
int intc_exit(void);
int intc_set_fiq_triggermode(uint32_t triggermode);
int intc_enable_interrupt(uint32_t intno);
int intc_disable_interrupt(uint32_t intno);
uint32_t intc_get_current_interrupt(void);
uint32_t intc_get_current_exception(void);

int isr_default(void *arg);

#endif  //__INTC_I_H__
