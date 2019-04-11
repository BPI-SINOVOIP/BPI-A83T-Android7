/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                thermal  module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : Watchdog.h
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: thermal controller public interfaces.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __THERMAL_H__
#define __THERMAL_H__

/*
*********************************************************************************************************
*                                       INIT THERMAL
*
* Description:  initialize thermal.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize thermal succeeded, others if failed.
*********************************************************************************************************
*/
s32 thermal_init(void);

#endif  //__THERMAL_H__
