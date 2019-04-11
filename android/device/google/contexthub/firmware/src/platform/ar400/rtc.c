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

#include <cpu/inc/barrier.h>
#include <cpu/inc/cpuMath.h>
#include <plat/inc/rtc.h>
#include <plat/inc/pwr.h>
#include <inc/timer.h>
#include <inc/platform.h>
#include <plat/inc/exti.h>
#include <plat/inc/cmsis.h>
#include <variant/inc/variant.h>
#include <plat/inc/include.h>

#ifndef NS_PER_S
#define NS_PER_S                    1000000000ULL
#endif


struct Ar400Rtc
{
    volatile uint32_t LOSC_CTRL;      /* 0x00 */
    volatile uint32_t LOSC_STA;       /* 0x04 */
    volatile uint32_t LOSC_CLK;       /* 0x08 */
    volatile uint32_t LOSC_CALIBR;    /* 0x0C */
    volatile uint32_t YYMMDD;         /* 0x10 */
    volatile uint32_t HHMMSS;         /* 0x14 */
    volatile uint32_t unused0[2];     /* 0x18 */
    volatile uint32_t ALARM0_CNT;     /* 0x20*/
    volatile uint32_t ALARM0_CUR;     /* 0x24 */
    volatile uint32_t ALARM0_EN;      /* 0x28 */
    volatile uint32_t ALARM0_IRQEN;   /* 0x2C */
    volatile uint32_t ALARM0_IRQST;   /* 0x30 */
    volatile uint32_t unused1[3];     /* 0x34 */
    volatile uint32_t ALARM1_HMS;     /* 0x40 */
    volatile uint32_t ALARM1_EN;      /* 0x44 */
    volatile uint32_t ALARM1_IRQEN;   /* 0x48 */
    volatile uint32_t ALARM1_IRQST;   /* 0x4C */
    volatile uint32_t ALARM0_CFG;     /* 0x50 */
    volatile uint32_t unused2[3];     /* 0x54 */
    volatile uint32_t LOSC_GATE;      /* 0x60 */
    volatile uint32_t unused3[3];     /* 0x64 */
    volatile uint32_t VERSION;        /* 0x70 */
    volatile uint32_t unused4[23];    /* 0x74 */
    volatile uint32_t GPR[8];         /* 0x100 */
};

#define RTC ((struct Ar400Rtc*)RTC_REG_BASE)

static int RTC_WKUP_IRQHandler(void *arg)
{
    RTC->ALARM0_IRQST = 1;
    return timIntHandler();
}

static void rtcSetDefaultDateTimeAndPrescalar(void)
{
    install_isr(R_ALARM0_IRQn, RTC_WKUP_IRQHandler, NULL);
    interrupt_enable(R_ALARM0_IRQn);
}

void rtcInit(void)
{
    rtcSetDefaultDateTimeAndPrescalar();
}

/* Set calendar alarm to go off after delay has expired. uint64_t delay must
 * be in valid uint64_t format */
int rtcSetWakeupTimer(uint64_t delay)
{
    uint64_t intState;
    uint64_t periodNsRecip;
    uint32_t periodNs;

    /* Minimum wakeup interrupt period is 1s */
    if (delay < NS_PER_S) {
        return RTC_ERR_TOO_SMALL;
    }

    periodNs = NS_PER_S;
    periodNsRecip = U64_RECIPROCAL_CALCULATE(NS_PER_S);

    intState = cpuIntsOff();

    /* Disable wakeup timer */
    RTC->ALARM0_IRQEN = 0;
    RTC->ALARM0_EN = 0;

    /* Downcounter value for wakeup clock.  Wakeup flag is set every
        * RTC->WUTR[15:0] + 1 cycles of the WUT clock. */
    RTC->ALARM0_CNT = cpuMathRecipAssistedUdiv64by32(delay, periodNs, periodNsRecip) - 1;

    /* Enable wakeup interrupts */
    RTC->ALARM0_EN = 1;
    RTC->ALARM0_IRQEN = 1;
    RTC->ALARM0_IRQST = 1;

    cpuIntsRestore(intState);

    return 0;
}

#if 0
uint64_t rtcGetTime(void)
{
    int32_t time_s;
    uint32_t yymmdd, hhmmss;
    // cumulative adjustments from 32 day months (year 2000)
    //   31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    //    1,  3,  1,  2,  1,  2,  1,  1,  2,  1,  2,  1
    //  0   1,  4,  5,  7,  8, 10, 11, 12, 14, 15, 17
    static const uint8_t adjust[] = { 0, 1, 4, 5, 7, 8, 10, 11, 12, 14, 15, 17 };
    uint8_t month;

    // need to loop incase an interrupt occurs in the middle or ssr
    // decrements (which can propagate changes to tr and dr)
    do {
        yymmdd = RTC->YYMMDD;
        hhmmss = RTC->HHMMSS;
    } while (yymmdd != RTC->YYMMDD);

    month =  ((yymmdd >> 8) & 0xf) - 1;
    time_s = (((yymmdd & 0x1F) - 1) + (month << 5) - adjust[month]) * 86400ULL;
    time_s += ((((hhmmss >> 16) & 0x1F) * 3600ULL) +
             (((hhmmss >> 6) & 0x3F) * 60ULL) +
             (((hhmmss) & 0x3F)));

    return (time_s * NS_PER_S);
}
#endif

/* 1000 / 24 = 41.6 ~= 42  */
uint64_t rtcGetTime(void)
{
    uint64_t cnt;

    cnt = cpucfg_counter_read();

    //return (cnt * 42);
    return cpuMathU64DivByU16((cnt * 125), 3);
}

uint32_t* rtcGetBackupStorage(void)
{
    return (uint32_t*)RTC->GPR;
}
