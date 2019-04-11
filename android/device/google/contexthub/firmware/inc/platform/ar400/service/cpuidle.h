/**
 * include\service\cpuidle.h
 *
 * Descript: cpuidle public head.
 * Copyright (C) 2014-2016 AllWinnertech Ltd.
 * Author: Xiafeng <xiafeng@allwinnertech.com>
 *
 */

#ifndef __CPUIDLE_H__
#define __CPUIDLE_H__

#include <plat/inc/cfgs.h>

#if CPUIDLE_USED
extern int cpuidle_init(void);
extern int cpuidle_exit(void);
int cpuidle_enter(u32 entrypoint, u32 mpidr, u32 level);
#else
static inline int cpuidle_init(void) { return 0; }
static inline int cpuidle_exit(void) { return 0; }
static inline int cpuidle_enter(u32 entrypoint, u32 mpidr, u32 level) { return 0; }
#endif

#endif /* __CPUIDLE_H__ */
