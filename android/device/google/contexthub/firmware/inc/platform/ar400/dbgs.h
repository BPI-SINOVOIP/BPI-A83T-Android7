/*
*********************************************************************************************************
*                                                AR100 System
*                                     AR100 Software System Develop Kits
*                                               Debugger Module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dbgs.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-25
* Descript: serial debugger public header.
* Update  : date                auther      ver     notes
*           2012-4-25 16:19:40  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DBGS_H__
#define __DBGS_H__

//debug level define,
//level 0 : dump debug information--none;
//level 1 : dump debug information--error;
//level 2 : dump debug information--error+warning;
//level 3 : dump debug information--error+warning+information;

#ifdef DEBUG_ON
/* debug levels */
#define DEBUG_LEVEL_INF    ((u32)1 << 0)
#define DEBUG_LEVEL_WRN    ((u32)1 << 1)
#define DEBUG_LEVEL_ERR    ((u32)1 << 2)
#define DEBUG_LEVEL_LOG    ((u32)1 << 3)

#if INF_USED
#define INF(...)    osLog(LOG_INFO, __VA_ARGS__)
#else
#define INF(...)    do{} while(0)
#endif

#if WRN_USED
#define WRN(...)    osLog(LOG_WARN, __VA_ARGS__)
#else
#define WRN(...)    do{} while(0)
#endif

#if ERR_USED
#define ERR(...)    osLog(LOG_ERROR, __VA_ARGS__)
#else
#define ERR(...)    do{} while(0)
#endif

#if LOG_USED
#define LOG(...)    osLog(LOG_DEBUG,__VA_ARGS__)
#else
#define LOG(...)    do{} while(0)
#endif

#else //DEBUG_ON
#define INF(...)    do{} while(0)
#define WRN(...)    do{} while(0)
#define ERR(...)    do{} while(0)
#define LOG(...)    do{} while(0)

#endif //DEBUG_ON

#define ASSERT(e)   (((void)0))
#define printk(...)    osLog(LOG_INFO,__VA_ARGS__)

#endif  //__DBGS_H__
