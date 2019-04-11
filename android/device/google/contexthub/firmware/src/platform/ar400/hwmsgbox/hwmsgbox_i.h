/**
 * driver\hwmsgbox\hwmsgbox_i.h
 *
 * Descript: hardware message-box internal header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __HWMSGBOX_I_H__
#define __HWMSGBOX_I_H__

#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>

#if HW_MESSAGE_USED
/* check message valid or not */
#define MESSAGE_VALID(msg) ((((u32)(msg)) >= MESSAGE_POOL_START) && \
                            (((u32)(msg)) < MESSAGE_POOL_END))

/* the number of hardware message queue. */
#define HWMSG_QUEUE_NUMBER (8)

//the user of hardware message queue.
typedef enum hwmsg_queue_user
{
	HWMSG_QUEUE_USER_ARISC, //arisc
	HWMSG_QUEUE_USER_AC327, //ac327
	HWMSG_QUEUE_USER_AC335, //ac335/a15
} hwmsg_queue_user_e;

/* hardware message-box register list */
#define MSGBOX_CTRL_REG(m)          (MSGBOX_REG_BASE + 0x0000 + (0x4 * (m>>2)))
#define MSGBOX_IRQ_EN_REG(u)        (MSGBOX_REG_BASE + 0x0040 + (0x20* u))
#define MSGBOX_IRQ_STATUS_REG(u)    (MSGBOX_REG_BASE + 0x0050 + (0x20* u))
#define MSGBOX_FIFO_STATUS_REG(m)   (MSGBOX_REG_BASE + 0x0100 + (0x4 * m))
#define MSGBOX_MSG_STATUS_REG(m)    (MSGBOX_REG_BASE + 0x0140 + (0x4 * m))
#define MSGBOX_MSG_REG(m)           (MSGBOX_REG_BASE + 0x0180 + (0x4 * m))
#define MSGBOX_DEBUG_REG            (MSGBOX_REG_BASE + 0x01c0)
#endif

#endif //__HWMSGBOX_I_H__
