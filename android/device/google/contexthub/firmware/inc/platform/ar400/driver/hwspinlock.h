/**
 * include\driver\hwspinlock.h
 *
 * Descript: hardware spinlock public header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __HW_SPINLOCK_H__
#define __HW_SPINLOCK_H__

#define	MSG_HWSPINLOCK      (0)
#define	AUDIO_HWSPINLOCK    (1)
#define	RTC_REG_HWSPINLOCK  (2)
#define HWSPINLOCK_CPUIDLE  (3)
#define INTC_HWSPINLOCK     (4)
//the max number of hardware spinlock
#define HW_SPINLOCK_NUM     (8)

#if HW_MESSAGE_USED

/*
*********************************************************************************************************
*                                       INITIALIZE HWSPINLOCK
*
* Description:  initialize hwspinlock.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize hwspinlock succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwspinlock_init(void);

/*
*********************************************************************************************************
*                                           EXIT HWSPINLOCK
*
* Description:  exit hwspinlock.
*
* Arguments  :  none.
*
* Returns    :  OK if exit hwspinlock succeeded, others if failed.
*********************************************************************************************************
*/
s32 hwspinlock_exit(void);

/*
*********************************************************************************************************
*                                           LOCK HWSPINLOCK WITH TIMEOUT
*
* Description:  lock an hwspinlock with timeout limit.
*
* Arguments  :  hwid : an hwspinlock id which we want to lock.
*
* Returns    :  OK if lock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
s32 hwspin_lock_timeout(u32 hwid, u32 timeout);

/*
*********************************************************************************************************
*                                           UNLOCK HWSPINLOCK
*
* Description:  unlock a specific hwspinlock.
*
* Arguments  :  hwid : an hwspinlock id which we want to unlock.
*
* Returns    :  OK if unlock hwspinlock succeeded, other if failed.
*********************************************************************************************************
*/
s32 hwspin_unlock(u32 hwid);

s32 hwspinlock_super_standby_init(void);
s32 hwspinlock_super_standby_exit(void);
#else
static inline s32 hwspinlock_init(void) { return 0; }
static inline s32 hwspinlock_exit(void) { return 0; }
static inline s32 hwspin_lock_timeout(u32 hwid, u32 timeout) { return OK; }
static inline s32 hwspin_unlock(u32 hwid) { return OK; }
#endif
#endif  //__HW_SPINLOCK_H__
