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
#include <plat/inc/cmsis.h>
#include <plat/inc/pwr.h>
#include <plat/inc/rtc.h>
#include <reset.h>
#include <stddef.h>

static uint32_t mResetReason;                                                     \

void pwrUnitClock(uint32_t bus, uint32_t unit, bool on)
{
}

void pwrUnitReset(uint32_t bus, uint32_t unit, bool on)
{
}

uint32_t pwrGetBusSpeed(uint32_t bus)
{
    return 0;
}

void pwrEnableAndClockRtc(enum RtcClock rtcClock)
{
}

void pwrEnableWriteBackupDomainRegs(void)
{
}

void pwrSetSleepType(enum Stm32F4xxSleepType sleepType)
{
    switch (sleepType) {
    case stm32f411SleepModeSleep:
        SCB->SCR &=~ SCB_SCR_SLEEPDEEP_Msk;
        break;
    case stm32f144SleepModeStopMR:
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        break;
    case stm32f144SleepModeStopMRFPD:
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        break;
    case stm32f411SleepModeStopLPFD:
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        break;
    case stm32f411SleepModeStopLPLV:
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        break;
    }

}

void pwrSystemInit(void)
{
}

uint32_t pwrResetReason(void)
{
    return mResetReason;
}
