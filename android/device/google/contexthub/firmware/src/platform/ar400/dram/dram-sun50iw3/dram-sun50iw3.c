/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                dram module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dram.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: dram driver.
* Update  : date                auther      ver     notes
*           2012-5-22 18:24:02  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include <plat/inc/include.h>

#if (defined CONFIG_ARCH_SUN50IW3P1)
dram_para_t *pdram_para = &arisc_para.dram_para;

#endif // sun50iw3
