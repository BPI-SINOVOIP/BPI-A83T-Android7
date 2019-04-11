/**
 * driver\hwspinlock\hwspinlock.c
 *
 * Descript: hardware spinlock module.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#include "hwspinlock_i.h"
#if HW_MESSAGE_USED

//#define HWSPINLOCK_TEST

struct hwspinlock hwspinlocks[HW_SPINLOCK_NUM];

static volatile u32 hwspinlock_suspend;

/*
 * hwspinlock_init() - initialize hwspinlock.
 *
 * @return: OK if initialize hwspinlock succeeded, others if failed.
 */
s32 hwspinlock_init(void)
{
	s32 index;

	/* enable SPINLOCK clock and set reset as de-assert state.
	 */
	ccu_set_mclk_onoff(CCU_MOD_CLK_SPINLOCK, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_SPINLOCK, CCU_CLK_NRESET);

	for (index = 0; index < HW_SPINLOCK_NUM; index++)
		hwspinlocks[index].locked = SPINLOCK_FREE;

	hwspinlock_suspend = 0;

	return OK;
}

/*
 * hwspinlock_exit() - exit hwspinlock.
 *
 * @return: OK if exit hwspinlock succeeded, others if failed.
 */
s32 hwspinlock_exit(void)
{
	return OK;
}

/*
 * hwspin_lock_timeout() - lock an hwspinlock with timeout limit.
 *
 * @hwid: an hwspinlock id which we want to lock.
 * @return: OK if lock hwspinlock succeeded, other if failed.
 */
s32 hwspin_lock_timeout(u32 hwid, u32 timeout)
{
	/* convert to cpu clcyes */
	u32 expire = timeout * 2000000;

	ASSERT(hwid < HW_SPINLOCK_NUM);

	/* disable cpu interrupt, save cpsr to cpsr-table. */
	hwspinlocks[hwid].cpsr = cpuIntsOff();

	if (hwspinlock_suspend) {
		return -EACCES;
	}

	/* try to take spinlock */
	while (readl(SPINLOCK_LOCK_REG(hwid)) == SPINLOCK_TAKEN) {
		/* spinlock is busy */
		if (expire == 0) {
			ERR("ts%d to\n", hwid);
			cpuIntsRestore(hwspinlocks[hwid].cpsr);
			return -ETIMEOUT;
		}
		expire--;
	}

	/* spinlock is been taken */
	hwspinlocks[hwid].locked = SPINLOCK_USED;

	return OK;
}

/*
 * hwspin_unlock() - unlock a specific hwspinlock.
 *
 * @hwid: an hwspinlock id which we want to unlock.
 * @return: OK if unlock hwspinlock succeeded, other if failed.
 */
s32 hwspin_unlock(u32 hwid)
{
	ASSERT(hwid < HW_SPINLOCK_NUM);

	if (hwspinlock_suspend) {
		cpuIntsRestore(hwspinlocks[hwid].cpsr);
		return -EACCES;
	}

	/* untaken the spinlock */
	writel(0x0, SPINLOCK_LOCK_REG(hwid));

	/* spinlock is free */
	hwspinlocks[hwid].locked = SPINLOCK_FREE;

	cpuIntsRestore(hwspinlocks[hwid].cpsr);

	return OK;
}

s32 hwspinlock_super_standby_init(void)
{
	hwspinlock_suspend = 1;

	return OK;
}

s32 hwspinlock_super_standby_exit(void)
{
	/* enable hwspinlock clock gatings */
	ccu_set_mclk_onoff(CCU_MOD_CLK_SPINLOCK, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_SPINLOCK, CCU_CLK_NRESET);

	hwspinlock_suspend = 0;

	return OK;
}

#ifdef HWSPINLOCK_TEST
#define HWSPINLOCK_TEST_ID 4

void hwspinlock_test(u32 *add_protect)
{
	if (hwspin_lock_timeout(HWSPINLOCK_TEST_ID, SPINLOCK_TIMEOUT)) {
		ERR("lto!\n");
		return;
	}
	if (*add_protect)
		ERR("%s,%d,%x, %x locked!\n", __func__, __LINE__, add_protect, *add_protect);
	*add_protect = 0x015f;
	time_udelay(5000);
	*add_protect = 0;
	hwspin_unlock(HWSPINLOCK_TEST_ID);
}

static struct softtimer timer_hwspl;
int hwspinlock_test_init(u32 add_protect)
{
	timer_hwspl.cycle = mecs_to_ticks(100);
	timer_hwspl.expires = 0;
	timer_hwspl.cb = (ar100_cb_t)hwspinlock_test;
	timer_hwspl.arg = (void *)add_protect;
	/* at the first time, we should valify the logic of hwspinlock, so
	 * the cancled code
	 */
	/* step1: logic test */
	/*
	if (hwspin_lock_timeout(HWSPINLOCK_TEST_ID, SPINLOCK_TIMEOUT)) {
		ERR("lock time out!\n");
		return 0;
	}
	*/
	/* step2: press test */
	add_softtimer(&timer_hwspl);
	LOG("hwspinlock_test init ok,add:%p,%x\n", add_protect, *(u32 *)add_protect);
	return 0;
}
#endif
#endif /* HW_MESSAGE_USED */
