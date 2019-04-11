/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                	  	  clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : ccu_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: clock control unit internal header.
* Update  : date                auther      ver     notes
*           2012-5-7 8:40:42	Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef	__CCU_I_H__
#define	__CCU_I_H__

#include <plat/inc/include.h>
#include "ccu_regs-sun50iw3.h"

typedef struct ccu_pll1_factor
{
    u8 factor_n;
    u8 factor_k;
    u8 factor_m;
    u8 factor_p;
} ccu_pll1_factor_t;

//local functions
s32 ccu_calc_pll1_factor(struct ccu_pll1_factor *factor, u32 rate);
s32 ccu_set_cpus_src(u32 sclk);

//ccu module registers address
extern struct ccu_reg_list *ccu_reg_addr;
extern struct ccu_pll_c0_cpux_reg0000 *ccu_pll_c0_cpux_reg_addr;
extern struct ccu_pll_periph0_reg0020 *ccu_pll_periph0_reg_addr;

//apb notifier list
extern struct notifier *apbs2_notifier_head;

#endif	//__CCU_I_H__
