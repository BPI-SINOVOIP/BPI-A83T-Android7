/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                                timer  module
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : timer.h
* By      : Superm
* Version : v1.0
* Date    : 2012-4-27
* Descript: timer controller public header.
* Update  : date                auther      ver     notes
*           2012-4-27 17:03:52  Superm       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __TIMER_H__
#define __TIMER_H__

/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})

/*
 *	These inlines deal with timer wrapping correctly. You are
 *	strongly encouraged to use them
 *	1. Because people otherwise forget
 *	2. Because if the timer wrap changes in future you won't have to
 *	   alter your driver code.
 *
 * time_after(a,b) returns true if the time a is after time b.
 *
 * Do this with "<0" and ">=0" to only test the sign of the result. A
 * good compiler would generate better code (and a really good compiler
 * wouldn't care). Gcc is currently neither.
 */
#define time_after(a,b)		\
	(typecheck(unsigned int, a) && \
	 typecheck(unsigned int, b) && \
	 ((int)(b) - (int)(a) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned int, a) && \
	 typecheck(unsigned int, b) && \
	 ((int)(a) - (int)(b) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)

/*
 * These four macros compare jiffies and 'a' for convenience.
 */

/* time_is_before_jiffies(a) return true if a is before jiffies */
#define time_is_before_jiffies(a) time_after(OSTime, a)

/* time_is_after_jiffies(a) return true if a is after jiffies */
#define time_is_after_jiffies(a) time_before(OSTime, a)

/* time_is_before_eq_jiffies(a) return true if a is before or equal to jiffies*/
#define time_is_before_eq_jiffies(a) time_after_eq(OSTime, a)

/* time_is_after_eq_jiffies(a) return true if a is after or equal to jiffies*/
#define time_is_after_eq_jiffies(a) time_before_eq(OSTime, a)

//the total number of hardware timer
#define TIMERC_TIMERS_NUMBER (2)

//timer work mode
typedef enum timer_mode
{
	TIMER_MODE_PERIOD       = 0x0,  //period trigger mode
	TIMER_MODE_ONE_SHOOT    = 0x1,  //one shoot trigger mode
} timer_mode_e;

/*
*********************************************************************************************************
*                                       INIT TIMER
*
* Description:  initialize timer.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize timer succeeded, others if failed.
*********************************************************************************************************
*/
int timer_init(void);

/*
*********************************************************************************************************
*                                       EXIT TIMER
*
* Description:  exit timer.
*
* Arguments  :  none.
*
* Returns    :  OK if exit timer succeeded, others if failed.
*********************************************************************************************************
*/
int timer_exit(void);

/*
*********************************************************************************************************
*                                       REQUEST TIMER
*
* Description:  request a hardware timer.
*
* Arguments  :  phdle   : the callback when the requested timer tick reached.
*               parg    : the argument for the callback.
*
* Returns    :  the handler if request hardware timer succeeded, NULL if failed.
*
* Note       :  the callback execute entironment : CPU disable interrupt response.
*********************************************************************************************************
*/
HANDLE timer_request(__pCBK_t phdle, void *parg);

/*
*********************************************************************************************************
*                                       RELEASE TIMER
*
* Description:  release a hardware timer.
*
* Arguments  :  htimer  : the handler of the released timer.
*
* Returns    :  OK if release hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
int timer_release(HANDLE htimer);

/*
*********************************************************************************************************
*                                       START TIMER
*
* Description:  start a hardware timer.
*
* Arguments  :  htimer  : the timer handler which we want to start.
*               period  : the period of the timer trigger, base on us.
*               mode    : the mode the timer trigger, details please
*                         refer to timer trigger mode.
*
* Returns    :  OK if start hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
int timer_start(HANDLE htimer, uint32_t period, uint32_t mode);


/*
*********************************************************************************************************
*                                       STOP TIMER
*
* Description:  stop a hardware timer.
*
* Arguments  :  htimer  : the timer handler which we want ot stop.
*
* Returns    :  OK if stop hardware timer succeeded, others if failed.
*********************************************************************************************************
*/
int timer_stop(HANDLE htimer);

//timer standby services
extern int timer_standby_init(uint32_t ms, timer_mode_e mode);
int  timer_standby_exit(void);
bool timer_standby_expired(void);
extern void timer_standby_isr(void);

//timer delay services
void time_mdelay(uint32_t ms);     //base on hardware timer
void time_cdelay(uint32_t cycles); //base on cpu cycle
void time_udelay(uint32_t us);     //base on hardware counter

//NOTE:everytimes change the cpus freq should recall
//this function to init the systick ticks per seconds
void  timer_systick_init(void);
#if !UCOS_USED
extern uint32_t tick_get(void);
#endif
int time_ticks_init(void);
uint64_t platSetTimerAlarm(uint64_t delay);
bool platSleepClockRequest(uint64_t wakeupTime, uint32_t maxJitterPpm, uint32_t maxDriftPpm, uint32_t maxErrTotalPpm);
bool sleepClockTmrPrepare(uint64_t delay, uint32_t acceptableJitter, uint32_t acceptableDrift, uint32_t maxAcceptableError, void *userData, uint64_t *savedData);
void sleepClockTmrWake(void *userData, uint64_t *savedData);

#endif  //__TIMER_H__
