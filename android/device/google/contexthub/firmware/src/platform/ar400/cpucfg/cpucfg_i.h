/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                cpucfg module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : cpucfg_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: cpu config module internal header.
* Update  : date                auther      ver     notes
*           2012-5-7 18:55:57   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __CPUCFG_I_H__
#define __CPUCFG_I_H__

#include <plat/inc/include.h>

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || (defined CONFIG_ARCH_SUN8IW5P1)
#include "cpucfg_regs-sun8iw1w3w5.h"
#elif (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
#include "cpucfg_regs-sun8iw6w9.h"
#elif (defined CONFIG_ARCH_SUN50IW1P1)
#include "cpucfg_regs-sun50iw1.h"
#elif (defined CONFIG_ARCH_SUN50IW2P1)
#include "cpucfg_regs-sun50iw2.h"
#elif (defined CONFIG_ARCH_SUN50IW3P1)
#include "cpucfg_regs-sun50iw3.h"
#elif (defined CONFIG_ARCH_SUN50IW6P1)
#include "cpucfg_regs-sun50iw6.h"
#endif

#endif  //__CPUCFG_I_H__
