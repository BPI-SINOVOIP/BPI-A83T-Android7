/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                              include master
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : include.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-2
* Descript: the include mater header.
* Update  : date                auther      ver     notes
*           2012-5-2 14:48:33   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __INCLUDE_H__
#define __INCLUDE_H__

//general header
#include <stdarg.h>     //use gcc stardard header file
#include <stddef.h>     //use gcc stardard define keyword, like size_t, ptrdiff_t
#include <stdint.h>

#include "./types.h"
#include "./cfgs.h"
#include "./error.h"

//messages define
#include "./messages.h"

/* power headers */
#include "./power/power.h"

//system headers
#include "./system/cpu.h"
#include "./system/daemon.h"
#include "./system/debugger.h"
#include "./system/notifier.h"
#include "./system/message_manager.h"
#include "./system/para.h"
#include "./system/trace_time.h"
#include "./system/platform.h"

//driver headers
#include "./driver/io.h"
#include "./driver/prcm.h"
#include "./driver/cpucfg.h"
#include "./driver/dma.h"
#include "./driver/dram.h"
#include "./driver/hwmsgbox.h"
#include "./driver/hwspinlock.h"
#include "./driver/intc.h"
#include "./driver/ir.h"
#include "./driver/rsb.h"
#include "./driver/p2wi.h"
#include "./driver/twi.h"
#include "./driver/pin.h"
#include "./driver/pmu.h"
#include "./driver/timer.h"
#include "./driver/uart.h"
#include "./driver/watchdog.h"
#include "./driver/thermal.h"
#include "./driver/led.h"

//bare test headers
#include "./test/bare_test.h"

//sevices
#include "./service/dvfs.h"
#include "./service/standby.h"
#include "./service/cpuidle.h"
#include "./service/mem_include.h"

//debugger
#include "./dbgs.h"
#if SYSCONFIG_USED
#include "./system/sys_config.h"
#endif

//libary
#include "./library.h"

#endif //__INCLUDE_H__
