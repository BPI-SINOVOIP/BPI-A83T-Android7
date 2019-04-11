/**
 * include\driver\hwspinlock.h
 *
 * Descript: include\driver\hwmsgbox.h
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __HWMSGBOX_H__
#define __HWMSGBOX_H__

#if HW_MESSAGE_USED

/*
*********************************************************************************************************
*                                           INITIALIZE HWMSGBOX
*
* Description:  initialize hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_init(void);

/*
*********************************************************************************************************
*                                           EXIT HWMSGBOX
*
* Description:  exit hwmsgbox.
*
* Arguments  :  none.
*
* Returns    :  OK if exit hwmsgbox succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_exit(void);

/*
*********************************************************************************************************
*                                       SEND MESSAGE BY HWMSGBOX
*
* Description:  send one message to another processor by hwmsgbox.
*
* Arguments  :  pmessage    : the pointer of sended message frame.
*               timeout     : the wait time limit when message fifo is full,
*                             it is valid only when parameter mode = SEND_MESSAGE_WAIT_TIMEOUT.
*
* Returns    :  OK if send message succeeded, other if failed.
*********************************************************************************************************
*/
s32 hwmsgbox_send_message(struct message *pmessage, u32 timeout);

/*
*********************************************************************************************************
*                                        QUERY MESSAGE
*
* Description:  query message of hwmsgbox by hand, mainly for.
*
* Arguments  :  none.
*
* Returns    :  the point of message, NULL if timeout.
*********************************************************************************************************
*/
struct message *hwmsgbox_query_message(void);

int hwmsgbox_feedback_message(struct message *pmessage, u32 timeout);

s32 hwmsgbox_super_standby_init(void);
s32 hwmsgbox_super_standby_exit(void);
void hwmsgbox_sh_wakeup_ap_set(void);
void hwmsgbox_sh_wakeup_ap_clear(void);

#else
static inline s32 hwmsgbox_init(void) { return OK; }
static inline s32 hwmsgbox_exit(void) { return OK; }
#endif /* HW_MESSAGE_USED */
#endif  //__HWMSGBOX_H__
