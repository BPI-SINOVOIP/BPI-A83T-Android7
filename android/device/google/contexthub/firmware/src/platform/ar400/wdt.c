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

#include <plat/inc/pwr.h>
#include <plat/inc/wdt.h>
#include <plat/inc/cmsis.h>
#include <plat/inc/include.h>

static s32 wdt_handler(void *parg)
{
    asm volatile(
        "mov    r0, #2                    \n"
        "b      cpuCommonFaultCode        \n"
    );

    return 0;
}

void wdtEnableClk()
{
}

void wdtEnableIrq()
{
    NVIC_EnableIRQ(R_WDG_IRQn);
}

void wdtDisableClk()
{
}

void wdtDisableIrq()
{
    NVIC_DisableIRQ(R_WDG_IRQn);
}

void wdtInit()
{
    watchdog_init();
    install_isr(R_WDG_IRQn, wdt_handler, NULL);
    watchdog_enable();
}

void wdtPing()
{
    watchdog_ping();
}
