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

#include <errno.h>
#include <isr.h>

#include <plat/inc/cmsis.h>
#include <plat/inc/exti.h>
#include <plat/inc/pwr.h>
#include <plat/inc/include.h>

void extiEnableIntLine(const uint32_t line, enum ExtiTrigger trigger)
{
    pin_set_int_trigger_mode(line >> GPIO_PORT_SHIFT, line & GPIO_PIN_MASK, trigger);

    /* Clear pending interrupt */
    extiClearPendingLine(line);

    /* Enable hardware interrupt */
    pin_enable_int(line >> GPIO_PORT_SHIFT, line & GPIO_PIN_MASK);
}

void extiDisableIntLine(const uint32_t line)
{
    pin_disable_int(line >> GPIO_PORT_SHIFT, line & GPIO_PIN_MASK);
}

void extiClearPendingLine(const uint32_t line)
{
    pin_clear_pending(line >> GPIO_PORT_SHIFT, line & GPIO_PIN_MASK);
}

bool extiIsPendingLine(const uint32_t line)
{
    return pin_query_pending(line >> GPIO_PORT_SHIFT, line & GPIO_PIN_MASK) ? true : false;
}

struct ExtiInterrupt
{
    struct ChainedInterrupt base;
    IRQn_Type irq;
};

static void extiInterruptEnable(struct ChainedInterrupt *irq)
{
    struct ExtiInterrupt *exti = container_of(irq, struct ExtiInterrupt, base);
    NVIC_EnableIRQ(exti->irq);
}

static void extiInterruptDisable(struct ChainedInterrupt *irq)
{
    struct ExtiInterrupt *exti = container_of(irq, struct ExtiInterrupt, base);
    NVIC_DisableIRQ(exti->irq);
}

#define DECLARE_SHARED_EXTI(i) {            \
    .base = {                               \
        .enable = extiInterruptEnable,      \
        .disable = extiInterruptDisable,    \
    },                                      \
    .irq = i,                               \
}

static struct ExtiInterrupt gInterrupts[] = {
    DECLARE_SHARED_EXTI(EXT_NMI_IRQn),
    DECLARE_SHARED_EXTI(R_GPIOL_IRQn),
    DECLARE_SHARED_EXTI(R_GPIOM_IRQn),
};

static inline struct ExtiInterrupt *extiForIrq(IRQn_Type n)
{
    if (n == EXT_NMI_IRQn)
        return &gInterrupts[0];
    if (n == R_GPIOL_IRQn)
        return &gInterrupts[1];
    if (n == R_GPIOM_IRQn)
        return &gInterrupts[2];
    return NULL;
}

static void extiIrqHandler(IRQn_Type n)
{
    struct ExtiInterrupt *exti = extiForIrq(n);
    dispatchIsr(&exti->base);
}

static int EXTI_NMI_IRQHandler(void *parg)
{
        extiIrqHandler(EXT_NMI_IRQn);

	return 0;
}

static int EXTI_PL_IRQHandler(void *parg)
{
        extiIrqHandler(R_GPIOL_IRQn);

	return 0;
}

static int EXTI_PM_IRQHandler(void *parg)
{
        extiIrqHandler(R_GPIOM_IRQn);

	return 0;
}

int extiChainIsr(IRQn_Type n, struct ChainedIsr *isr)
{
    struct ExtiInterrupt *exti = extiForIrq(n);
    if (!exti)
        return -EINVAL;

    chainIsr(&exti->base, isr);
    return 0;
}

int extiUnchainIsr(IRQn_Type n, struct ChainedIsr *isr)
{
    struct ExtiInterrupt *exti = extiForIrq(n);
    if (!exti)
        return -EINVAL;

    unchainIsr(&exti->base, isr);
    return 0;
}

int extiUnchainAll(uint32_t tid)
{
    int i, count = 0;
    struct ExtiInterrupt *exti = gInterrupts;

    for (i = 0; i < ARRAY_SIZE(gInterrupts); ++i, ++exti)
        count += unchainIsrAll(&exti->base, tid);

    return count;
}

void extiInit(void)
{
    install_isr(EXT_NMI_IRQn, EXTI_NMI_IRQHandler, NULL);
    install_isr(R_GPIOL_IRQn, EXTI_PL_IRQHandler, NULL);
    install_isr(R_GPIOM_IRQn, EXTI_PM_IRQHandler, NULL);
}
