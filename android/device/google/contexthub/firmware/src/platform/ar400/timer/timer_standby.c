/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                timer  module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : timer_standby.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-3
* Descript: timer standby service.
* Update  : date                auther      ver     notes
*           2012-5-3 14:12:17   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "timer_i.h"
#if STANDBY_USED

int timer_standby_init(void)
{
	return OK;
}

int timer_standby_exit(void)
{
	return OK;
}

bool timer_standby_expired(void)
{
	return FALSE;
}

#endif /* STANDBY_USED */
