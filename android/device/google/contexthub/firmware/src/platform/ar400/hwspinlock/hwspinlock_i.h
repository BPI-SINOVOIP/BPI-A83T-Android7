/**
 * driver\hwspinlock\hwspinlock_i.h
 *
 * Descript: hardware spinlock internal header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __HW_SPINLOCK_I_H__
#define __HW_SPINLOCK_I_H__

#include <plat/inc/include.h>
#include <cpu.h>
#if HW_MESSAGE_USED

/* the taken or not state of spinlock */
#define SPINLOCK_NOTTAKEN   (0)
#define SPINLOCK_TAKEN      (1)

/* the used state of spinlock */
#define SPINLOCK_FREE       (0)
#define SPINLOCK_USED       (1)

/* hardware spinlock register list */
#define SPINLOCK_SYS_STATUS_REG     (SPINLOCK_REG_BASE + 0x0000)
#define SPINLOCK_STATUS_REG         (SPINLOCK_REG_BASE + 0x0010)
#ifdef CONFIG_ARCH_SUN8IW1P1 //only 1633 defined
#define SPINLOCK_IRQ_EN_REG         (SPINLOCK_REG_BASE + 0x0020)
#define SPINLOCK_IRQ_PEND_REG       (SPINLOCK_REG_BASE + 0x0040)
#endif
#define SPINLOCK_LOCK_REG(id)       (SPINLOCK_REG_BASE + 0x0100 + id * 4)

typedef struct hwspinlock
{
	u32 cpsr;
	u32 locked;
} hwspinlock_t;
#endif

#endif  //__HW_SPINLOCK_I_H__
