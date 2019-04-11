/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cpu/inc/cpuMath.h>
#include <plat/inc/gpio.h>
#include <plat/inc/usart.h>
#include <plat/inc/cmsis.h>
#include <plat/inc/pwr.h>
#include <plat/inc/rtc.h>
#include <plat/inc/plat.h>
#include <plat/inc/exti.h>
#include <plat/inc/syscfg.h>
#include <plat/inc/wdt.h>
#include <plat/inc/dma.h>
#include <plat/inc/include.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <platform.h>
#include <seos.h>
#include <heap.h>
#include <timer.h>
#include <usart.h>
#include <gpio.h>
#include <mpu.h>
#include <cpu.h>
#include <hostIntf.h>
#include <atomic.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <sensType.h>
#include <variant/inc/variant.h>
#include "timer/timer_i.h"

/* RTC bit defintions */
#define TIM_EGR_UG          0x0001


#ifdef DEBUG_UART_UNITNO
static struct usart mDbgUart;
#endif

#ifdef DEBUG_LOG_EVT
#define EARLY_LOG_BUF_SIZE      1024
#define HOSTINTF_HEADER_SIZE    4
uint8_t *mEarlyLogBuffer;
uint16_t mEarlyLogBufferCnt;
uint16_t mEarlyLogBufferOffset;
bool mLateBoot;
#endif

static uint64_t mTimeAccumulated = 0;
static uint32_t mMaxJitterPpm = 0, mMaxDriftPpm = 0, mMaxErrTotalPpm = 0;
static uint32_t mSleepDevsToKeepAlive = 0;
static uint64_t mWakeupTime = 0;
static uint32_t mDevsMaxWakeTime[PLAT_MAX_SLEEP_DEVS] = {0,};
//static struct Gpio *mShWakeupGpio;
//static struct ChainedIsr mShWakeupIsr;

int systimer_init(void);
s32 TIM2_IRQHandler(void *p_arg);

void platUninitialize(void)
{
#ifdef DEBUG_UART_UNITNO
    usartClose(&mDbgUart);
#endif
}

void *platLogAllocUserData()
{
#if defined(DEBUG_LOG_EVT)
    struct HostIntfDataBuffer *userData = NULL;

    if (mLateBoot) {
        userData = heapAlloc(sizeof(struct HostIntfDataBuffer));
    } else if (mEarlyLogBufferOffset < EARLY_LOG_BUF_SIZE - HOSTINTF_HEADER_SIZE) {
        userData = (struct HostIntfDataBuffer *)(mEarlyLogBuffer + mEarlyLogBufferOffset);
        mEarlyLogBufferOffset += HOSTINTF_HEADER_SIZE;
    }
    if (userData) {
        userData->sensType = SENS_TYPE_INVALID;
        userData->length = 0;
        userData->dataType = HOSTINTF_DATA_TYPE_LOG;
        userData->interrupt = NANOHUB_INT_NONWAKEUP;
    }
    return userData;
#else
    return NULL;
#endif
}

#if defined(DEBUG_LOG_EVT)
static void platEarlyLogFree(void *buf)
{
    struct HostIntfDataBuffer *userData = (struct HostIntfDataBuffer *)buf;
    mEarlyLogBufferCnt += userData->length + HOSTINTF_HEADER_SIZE;
    if (mEarlyLogBufferCnt >= mEarlyLogBufferOffset) {
        heapFree(mEarlyLogBuffer);
    }
}
#endif

void platEarlyLogFlush(void)
{
#if defined(DEBUG_LOG_EVT)
    uint16_t i = 0;
    struct HostIntfDataBuffer *userData;

    mLateBoot = true;

    while (i < mEarlyLogBufferOffset) {
        userData = (struct HostIntfDataBuffer *)(mEarlyLogBuffer + i);
        osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | EVT_DEBUG_LOG, userData, platEarlyLogFree);
        i += HOSTINTF_HEADER_SIZE + userData->length;
    }
#endif
}

void platLogFlush(void *userData)
{
#if defined(DEBUG_LOG_EVT)
    if (userData && mLateBoot) {
        if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | EVT_DEBUG_LOG, userData, heapFree))
            heapFree(userData);
    }
#endif
}

bool platLogPutcharF(void *userData, char ch)
{
#if defined(DEBUG) && defined(DEBUG_UART_PIN)
    if (ch == '\n')
        gpioBitbangedUartOut('\r');
    gpioBitbangedUartOut(ch);
#endif
#ifdef DEBUG_UART_UNITNO
    //usartPutchat(&mDbgUart, ch);
    /* if current character is '\n', insert output with '\r' */
    if (ch == '\n')
        uart_putc('\r');
    uart_putc(ch);
#elif defined(DEBUG_LOG_EVT)
    struct HostIntfDataBuffer *buffer;

    if (userData) {
        buffer = userData;
        if (buffer->length == sizeof(uint64_t) + HOSTINTF_SENSOR_DATA_MAX) {
            buffer->buffer[buffer->length - 1] = '\n';
        } else if (!mLateBoot) {
            if (mEarlyLogBufferOffset == EARLY_LOG_BUF_SIZE) {
                buffer->buffer[buffer->length - 1] = '\n';
            } else {
                buffer->buffer[buffer->length++] = ch;
                mEarlyLogBufferOffset++;
            }
        } else {
            buffer->buffer[buffer->length++] = ch;
        }
    }
#endif
    return true;
}

#if 0
static bool platWakeupIsr(struct ChainedIsr *isr)
{
    if (!extiIsPendingGpio(mShWakeupGpio))
        return false;

    extiClearPendingGpio(mShWakeupGpio);

    hostIntfRxPacket(!gpioGet(mShWakeupGpio));

    return true;
}
#endif

bool platWakeupIsr(bool wakeup)
{
    hostIntfRxPacket(!wakeup);

    return true;
}

void platInitialize(void)
{
    //prepare for sleep mode(s)
    SCB->SCR &=~ SCB_SCR_SLEEPONEXIT_Msk;

#if 0
	{
        	volatile u32 flag = 0xff;
		while(flag == 0xff);
   	}
#endif
    startup_entry();

    //ccu_init();

    //pin_init();

#ifdef DEBUG_UART_UNITNO
    //uart_init();
    printk("date:%s,%s\n", __DATE__, __TIME__);
    //printk("uart init ok\n");
#endif

    //cpucfg_init();
    //printk("cpucfg init ok\n");

    //interrupt_init();
    //printk("int init ok\n");

    /* enable MPU */
    mpuStart();
    printk("mpu init ok\n");

    //timer_init();
    //printk("timer init ok\n");
    systimer_init();
    printk("systimer init ok\n");

    /* set up RTC */
    //rtcInit();

    /* bring up systick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->VAL = 0;
    //SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    //mShWakeupGpio = gpioRequest(SH_INT_WAKEUP);
    //gpioConfigInput(mShWakeupGpio, GPIO_SPEED_LOW, GPIO_PULL_NONE);
    //syscfgSetExtiPort(mShWakeupGpio);
    //extiEnableIntGpio(mShWakeupGpio, EXTI_TRIGGER_BOTH);
    //mShWakeupIsr.func = platWakeupIsr;
    //extiChainIsr(SH_EXTI_WAKEUP_IRQ, &mShWakeupIsr);

#ifdef DEBUG_LOG_EVT
    /* allocate buffer for early boot log message*/
    mEarlyLogBuffer = heapAlloc(EARLY_LOG_BUF_SIZE);
#endif

}

//fpga the cpus clk is 24MHz, so the systick clk src is 24MHz
//24M / 10^9 = 3 / 125 = 0.024
//1000 / 24 = 41.6666 ~= 42
static uint64_t platsystickTicksToNs(uint32_t systickTicks)
{
    //return cpuMathMul(systickTicks, 125) / 3;
    //return systickTicks * 42;
    return cpuMathU64DivByU16((systickTicks * 125), 3);
}

uint64_t platGetTicks(void)
{
    uint64_t ret;
    uint32_t val;

    do {
        mem_reorder_barrier(); //mTimeAccumulated may change since it was read in condition check

        ret = mTimeAccumulated;
        val = SysTick->VAL;

        mem_reorder_barrier(); //mTimeAccumulated may change since it was read above

    } while (mTimeAccumulated != ret || SysTick->VAL > val);

    return platsystickTicksToNs(0x01000000 - val) + ret;
}

int systimer_init(void)
{
	//startup period timer for system timer tick
	systimer = timer_request(TIM2_IRQHandler, NULL);
	if (systimer)
	{
		timer_start(systimer, 65000, TIMER_MODE_PERIOD);

		INF("setup timer server succeeded\n");
		return OK;
	}
	//request timer failed
	return -EFAIL;
}

/* Timer interrupt handler */
s32 TIM2_IRQHandler(void *p_arg)
{
	timer_stop(systimer);

	/* call timer handler since it might need to reschedule an interrupt (eg: in case where initial delay was too far off & we were limited by timer length) */
	timIntHandler();

	return 0;
}

/* SysTick interrupt handler */
void SysTick_Handler(void);
void SysTick_Handler(void)
{
    //INF("systick\n");
    mTimeAccumulated += platsystickTicksToNs(SysTick->LOAD + 1); //todo - incremenet by actual elapsed nanoseconds and not just "1"
}

bool platRequestDevInSleepMode(uint32_t sleepDevID, uint32_t maxWakeupTime)
{
    if (sleepDevID >= PLAT_MAX_SLEEP_DEVS || sleepDevID >= Stm32sleepDevNum)
        return false;

    mDevsMaxWakeTime[sleepDevID] = maxWakeupTime;
    while (!atomicCmpXchg32bits(&mSleepDevsToKeepAlive, mSleepDevsToKeepAlive, mSleepDevsToKeepAlive | (1UL << sleepDevID)));

    return true;
}

bool platReleaseDevInSleepMode(uint32_t sleepDevID)
{
    if (sleepDevID >= PLAT_MAX_SLEEP_DEVS || sleepDevID >= Stm32sleepDevNum)
        return false;

    while (!atomicCmpXchg32bits(&mSleepDevsToKeepAlive, mSleepDevsToKeepAlive, mSleepDevsToKeepAlive &~ (1UL << sleepDevID)));

    return true;
}

uint64_t platSetTimerAlarm(uint64_t delay) //delay at most that many nsec
{
    uint32_t delayInUsecs;

    //turn off timer to prevent interrupts now
    timer_stop(systimer);

    if (delay >= (1000ULL << 32)) //it is only a 32-bit counter - we cannot set delays bigger than that
        delayInUsecs = 0xffffffff;
    else
        delayInUsecs = cpuMathUint44Div1000ToUint32(delay);

    //timer_start(systimer, cpuMathUint44Div1000ToUint32(delayInUsecs), TIMER_MODE_ONE_SHOOT);
    timer_start(systimer, delayInUsecs, TIMER_MODE_ONE_SHOOT);

    return delayInUsecs;
}

bool platSleepClockRequest(uint64_t wakeupTime, uint32_t maxJitterPpm, uint32_t maxDriftPpm, uint32_t maxErrTotalPpm)
{
    uint64_t intState, curTime = timGetTime();

    if (wakeupTime && curTime >= wakeupTime)
        return false;

    intState = cpuIntsOff();

    mMaxJitterPpm = maxJitterPpm;
    mMaxDriftPpm = maxDriftPpm;
    mMaxErrTotalPpm = maxErrTotalPpm;
    mWakeupTime = wakeupTime;

    //LOG("wakeupTime:%llu, curTime:%llu\n", wakeupTime, curTime);

    //TODO: set an actual alarm here so that if we keep running and do not sleep till this is due, we still fire an interrupt for it!
    if (wakeupTime)
        platSetTimerAlarm(wakeupTime - curTime);

    cpuIntsRestore(intState);

    return true;
}

#if 0
static bool sleepClockRtcPrepare(uint64_t delay, uint32_t acceptableJitter, uint32_t acceptableDrift, uint32_t maxAcceptableError, void *userData, uint64_t *savedData)
{

    pwrSetSleepType((uint32_t)userData);
    *savedData = rtcGetTime();

    if (delay && rtcSetWakeupTimer(delay) < 0)
        return false;

    //sleep with systick off (for timing) and interrupts off (for power due to HWR errata)
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

    return true;
}

static void sleepClockRtcWake(void *userData, uint64_t *savedData)
{
    //re-enable Systic and its interrupt
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    mTimeAccumulated += rtcGetTime() - *savedData;
}
#endif

bool sleepClockTmrPrepare(uint64_t delay, uint32_t acceptableJitter, uint32_t acceptableDrift, uint32_t maxAcceptableError, void *userData, uint64_t *savedData)
{
    pwrSetSleepType(stm32f411SleepModeSleep);
    platRequestDevInSleepMode(Stm32sleepDevTim2, 0);

    *savedData = platSetTimerAlarm(delay ?: ~0ull);

    //sleep with systick off (for timing) and interrupts off (for power due to HWR errata)
    SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);
    return true;
}

void sleepClockTmrWake(void *userData, uint64_t *savedData)
{
    uint32_t cnt;
    uint32_t overflow;
    uint64_t leftTicks;
    struct timer *ptimer = (struct timer *)systimer;

    //re-enable Systic and its interrupt
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

    //stop the timer counting;
    ptimer->pregs->control &= ~(0x1);

    //If we are within one time tick of overflow, it is possible for SR to
    //not indicate a pending overflow, but CNT contain 0xFFFFFFFF or vice versa,
    //depending on the read order of SR and CNT
    //read both values until they are stable
    do {
        overflow = ptimerc_regs->irq_status & (1 << ptimer->timer_no);
        cnt = ptimer->pregs->count;
    } while (overflow != (ptimerc_regs->irq_status & (1 << ptimer->timer_no)) || cnt != ptimer->pregs->count);

    leftTicks = cnt; //if we wake NOT from timer, only count the ticks that actually ticked as "time passed"
    if (overflow & 1) //if there was an overflow, account for it
        leftTicks -= 0x100000000ull;

    mTimeAccumulated += cpuMathMul((*savedData - leftTicks), 1000); //this clock runs at 1MHz

    platReleaseDevInSleepMode(Stm32sleepDevTim2);
}

static bool sleepClockJustWfiPrepare(uint64_t delay, uint32_t acceptableJitter, uint32_t acceptableDrift, uint32_t maxAcceptableError, void *userData, uint64_t *savedData)
{
    pwrSetSleepType(stm32f411SleepModeSleep);

    return true;
}

struct PlatSleepAndClockInfo {
    uint64_t resolution;
    uint64_t resolutionReciprocal; // speed up runtime by using 48 more code bytes? yes please!
    uint32_t maxCounter;
    uint32_t jitterPpm;
    uint32_t driftPpm;
    uint32_t maxWakeupTime;
    uint32_t devsAvail; //what is available in sleep mode?
    bool (*prepare)(uint64_t delay, uint32_t acceptableJitter, uint32_t acceptableDrift, uint32_t maxAcceptableError, void *userData, uint64_t *savedData);
    void (*wake)(void *userData, uint64_t *savedData);
    void *userData;
} static const platSleepClocks[] = {
#if 0
#ifndef STM32F4xx_DISABLE_LPLV_SLEEP
    { /* RTC + LPLV STOP MODE */
        .resolution = 1000000000ull/32768,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(1000000000ull/32768),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 50,
        .maxWakeupTime = 407000ull,
        .prepare = sleepClockRtcPrepare,
        .wake = sleepClockRtcWake,
        .userData = (void*)stm32f411SleepModeStopLPLV,
    },
#endif
#ifndef STM32F4xx_DISABLE_LPFD_SLEEP
    { /* RTC + LPFD STOP MODE */
        .resolution = 1000000000ull/32768,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(1000000000ull/32768),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 50,
        .maxWakeupTime = 130000ull,
        .prepare = sleepClockRtcPrepare,
        .wake = sleepClockRtcWake,
        .userData = (void*)stm32f411SleepModeStopLPFD,
    },
#endif
#ifndef STM32F4xx_DISABLE_MRFPD_SLEEP
    { /* RTC + MRFPD STOP MODE */
        .resolution = 1000000000ull/32768,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(1000000000ull/32768),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 50,
        .maxWakeupTime = 111000ull,
        .prepare = sleepClockRtcPrepare,
        .wake = sleepClockRtcWake,
        .userData = (void*)stm32f144SleepModeStopMRFPD,
    },
#endif
#ifndef STM32F4xx_DISABLE_MR_SLEEP
    { /* RTC + MR STOP MODE */
        .resolution = 1000000000ull/32768,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(1000000000ull/32768),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 50,
        .maxWakeupTime = 14500ull,
        .prepare = sleepClockRtcPrepare,
        .wake = sleepClockRtcWake,
        .userData = (void*)stm32f144SleepModeStopMR,
    },
#endif

#ifndef STM32F4xx_DISABLE_TIM2_SLEEP
    { /* TIM2 + SLEEP MODE */
        .resolution = 1000000000ull/1000000,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(1000000000ull/1000000),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 30,
        .maxWakeupTime = 12ull,
        .devsAvail = (1 << Stm32sleepDevTim2) | (1 << Stm32sleepDevTim4) | (1 << Stm32sleepDevTim5) | (1 << Stm32sleepDevTim9) | (1 << Stm32sleepWakeup) | (1 << Stm32sleepDevSpi2) | (1 << Stm32sleepDevSpi3) | (1 << Stm32sleepDevI2c1),
        .prepare = sleepClockTmrPrepare,
        .wake = sleepClockTmrWake,
    },
#endif
#endif
    { /* just WFI */
        .resolution = 16000000000ull/1000000,
        .resolutionReciprocal = U64_RECIPROCAL_CALCULATE(16000000000ull/1000000),
        .maxCounter = 0xffffffff,
        .jitterPpm = 0,
        .driftPpm = 0,
        .maxWakeupTime = 0,
        .devsAvail = (1 << Stm32sleepDevTim2) | (1 << Stm32sleepDevTim4) | (1 << Stm32sleepDevTim5) | (1 << Stm32sleepDevTim9) | (1 << Stm32sleepWakeup) | (1 << Stm32sleepDevSpi2) | (1 << Stm32sleepDevSpi3) | (1 << Stm32sleepDevI2c1),
        .prepare = sleepClockJustWfiPrepare,
    },

    /* terminator */
    {0},
};

void platSleep(void)
{
    uint64_t predecrement = 0, curTime = timGetTime(), length = mWakeupTime - curTime, intState;
    const struct PlatSleepAndClockInfo *sleepClock, *leastBadOption = NULL;
    uint64_t savedData;
    uint32_t i;

    //shortcut the sleep if it is time to wake up already
    if (mWakeupTime && mWakeupTime < curTime)
        return;

    for (sleepClock = platSleepClocks; sleepClock->maxCounter; sleepClock++) {

        bool potentialLeastBadOption = false;

        //if we have timers, consider them
        if (mWakeupTime) {

            //calculate how much we WOULD predecerement by
            predecrement = sleepClock->resolution + sleepClock->maxWakeupTime;

            //skip options with too much jitter (after accounting for error
            if (sleepClock->jitterPpm > mMaxJitterPpm)
                continue;

            //skip options that will take too long to wake up to be of use
            if (predecrement > length)
                continue;

            //skip options with too much  drift
            if (sleepClock->driftPpm > mMaxDriftPpm)
                continue;

            //skip options that do not let us sleep enough, but save them for later if we simply must pick something
            if (cpuMathRecipAssistedUdiv64by64(length, sleepClock->resolution, sleepClock->resolutionReciprocal) > sleepClock->maxCounter && !leastBadOption)
                potentialLeastBadOption = true;
        }

        //skip all options that do not keep enough deviceas awake
        if ((sleepClock->devsAvail & mSleepDevsToKeepAlive) != mSleepDevsToKeepAlive)
            continue;

        //skip all options that wake up too slowly
        for (i = 0; i < Stm32sleepDevNum; i++) {
            if (!(mSleepDevsToKeepAlive & (1 << i)))
                continue;
            if (mDevsMaxWakeTime[i] < sleepClock->maxWakeupTime)
                break;
        }
        if (i != Stm32sleepDevNum)
            continue;

        //if it will not let us sleep long enough save it as a possibility and go on
        if (potentialLeastBadOption && !leastBadOption)
            leastBadOption = sleepClock;
        else //if it fits us perfectly, pick it
            break;
    }
    if (!sleepClock->maxCounter)
        sleepClock = leastBadOption;

    if (!sleepClock) {
        //should never happen - this will spin the CPU and be bad, but it WILL work in all cases
        return;
    }

    //turn ints off in prep for sleep
    intState = cpuIntsOff();

    //options? config it
    if (sleepClock->prepare && !sleepClock->prepare(mWakeupTime ? length - sleepClock->maxWakeupTime : 0, mMaxJitterPpm, mMaxDriftPpm, mMaxErrTotalPpm, sleepClock->userData, &savedData))
        return;

    asm volatile ("wfi\n"
        "nop" :::"memory");

    //wakeup
    if (sleepClock->wake)
        sleepClock->wake(sleepClock->userData, &savedData);

    //re-enable interrupts and let the handlers run
    cpuIntsRestore(intState);
}

static uint32_t rtcbakeupstorage[RTC_NUM_BACKUP_REGS];
void* platGetPersistentRamStore(uint32_t *bytes)
{
#if 0
    *bytes = sizeof(uint32_t[RTC_NUM_BACKUP_REGS]);
    return rtcGetBackupStorage();
#endif
    *bytes = sizeof(uint32_t[RTC_NUM_BACKUP_REGS]);
    return rtcbakeupstorage;
}

uint32_t platFreeResources(uint32_t tid)
{
#if 0
    uint32_t dmaCount = dmaStopAll(tid);
    uint32_t irqCount = extiUnchainAll(tid);

    return (dmaCount << 8) | irqCount;
#endif
    return 0;
}

void platPeriodic()
{
    wdtPing();
}
