/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit module internal header.
* Update  : date                auther      ver     notes
*           2012-5-22 13:27:09  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PMU_I_H__
#define __PMU_I_H__

#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>
#include "../prcm/ccu_i-sun50iw3.h"

#define RTC_ALARM_INT_EN_REG (0xd0)
#define RTC_ALARM_INT_ST_REG (0xd1)

//keep the struct word align
//by superm at 2014-2-13 15:34:09
typedef struct voltage_info
{
	u16 devaddr;
	u16 regaddr;
	u16 min1_mV;
	u16 max1_mV;
	u16 step1_mV;
	u16 step1_num;
	u16 min2_mV;
	u16 max2_mV;
	u16 step2_mV;
	u16 step2_num;
	u32 mask;
	u8 mode_reg;
	u8 mode_offset;
} voltage_info_t;

//local functions
s32 nmi_int_handler(void *parg);
s32 pmu_clear_pendings(void);

#endif  //__PMU_I_H__
