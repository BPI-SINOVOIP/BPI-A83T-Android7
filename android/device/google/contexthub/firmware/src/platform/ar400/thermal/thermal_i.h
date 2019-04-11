/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                thermal  module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : thermal_i.h
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: thermal controller public interfaces.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __THERMAL_I_H__
#define __THERMAL_I_H__

#include <plat/inc/include.h>

/* the base address of thermal register */
#define THS_REG_BASE     (0x01f04000)
#define THS_STAT         (THS_REG_BASE + 0x48)

#endif  //__THERMAL_I_H__
