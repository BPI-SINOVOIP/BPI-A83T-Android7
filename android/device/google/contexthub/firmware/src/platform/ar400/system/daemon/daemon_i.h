/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                daemon module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : daemon_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: daemon module internal header.
* Update  : date                auther      ver     notes
*           2012-5-13 15:09:44  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DAEMON_I_H__
#define __DAEMON_I_H__

#include <plat/inc/include.h>
#include <plat/inc/exti.h>
#include <cpu.h>

extern char ld_dbuf_beg;
extern char ld_dbuf_end;
#define DATA_BUF_BEG            (&ld_dbuf_beg)
#define DATA_BUF_END            (&ld_dbuf_end)

#endif  //__DAEMON_I_H__
