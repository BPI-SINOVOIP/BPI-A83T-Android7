/**
 * system\message_manager\message_manager_i.h
 *
 * Descript: message manager internal header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __MESSAGE_MANAGER_I_H__
#define __MESSAGE_MANAGER_I_H__

#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>
#include <sharemem.h>
#include <cpu.h>
#if HW_MESSAGE_USED

//the strcuture of message cache,
//main for messages cache management.
typedef struct message_cache
{
	volatile unsigned int number;                    //valid message number
	struct message       *cache[MESSAGE_CACHED_MAX]; //message cache table
} message_cache_t;

enum textx_section_e
{
	SECTION_NONE            = 0,
	SECTION_SSTANDBY        = 1,
	SECTION_NSTANDBY        = 2,
	SECTION_ESSTANDBY       = 3,
	SECTION_FAKEPOWEROFF    = 4,
	SECTION_TSTANDBY        = 5,
	SECTION_MAX             = 6
};

#endif

#endif  //__MESSAGE_MANAGER_I_H__
