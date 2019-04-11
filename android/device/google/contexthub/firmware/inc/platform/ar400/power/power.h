/*
 * sunxi virtual power layer for compatible all power solutions
 *
 * Copyright (C) 2014 AllWinnertech Ltd.
 * Author: xiafeng <xiafeng@allwinnertech.com>
 *
 */
#ifndef __POWER_H__
#define __POWER_H__

#if PMU_USED
#define power_set_sys_lowcons() pmu_set_lowpcons()
#define power_off_system() pmu_poweroff_system()
#define power_set_pkeytime(t) pmu_set_pok_time(t)
#define power_reset_system() pmu_reset_system()

#else
#define power_set_sys_lowcons()
#define power_off_system()
#define power_set_pkeytime(v)
#define power_reset_system()
#endif

#endif  /* __POWER_H__ */
