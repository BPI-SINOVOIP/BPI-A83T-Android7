/*
 * include/cfgs.h
 *
 * Descript: system configure header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */

#ifndef __CFGS_H__
#define __CFGS_H__

//platform defines
#include "autoconf.h"
#include "platform.h"

#define HOMELET_USED            (1)     /* NOTE:should defined by autoconf.h!!  */

#define ARISC_SRAM_A2_PBASE             (0x0)
#define ARISC_SRAM_A2_SIZE              (SUNXI_SRAM_A2_SIZE)

/* NOTE: for big-endia use, if the code plased in DRAM, we should define 1
 * for twi or some other devices when translate data from u32 to u8 */
#define CODE_IN_DRAM           (1)

/* test mode control */
#define AR100_TEST_EN   (0) //enable control of ar100 self test

/* cpus jtag debug enable */
#define CPUS_JTAG_DEBUG_EN              (0)

/* debugger system */
#define DEBUG_ON
#define DEBUG_LEVEL                     (2)             /* debug level */

/* dram config */
#define DRAM_BASE_ADDR                  (0x40000000)    /* the base address of dram */
#define DRAM_TRANING_SIZE               (128)           /* the byte size of dram traning area */

/* system time tick, 100 ticks/second */
#define TICK_PER_SEC                    (100)

/* the max number of notifier */
#define NOTIFIER_MAX                    (8)

#define DAEMON_ONCE_TICKS               (500)

/*
 * system define
 */
#define UCOS_USED               (0)     /* config here */

/*
 * devices & services define, should not depend other defined
 */
#if defined CONFIG_ARCH_SUN8IW1P1
#include "cfgs-sun8iw1.h"
#elif defined CONFIG_ARCH_SUN8IW3P1
#include "cfgs-sun8iw3.h"
#elif defined CONFIG_ARCH_SUN8IW5P1
#include "cfgs-sun8iw5.h"
#elif defined CONFIG_ARCH_SUN8IW6P1
#include "cfgs-sun8iw6.h"
#elif defined CONFIG_ARCH_SUN8IW7P1
#include "cfgs-sun8iw7.h"
#elif defined CONFIG_ARCH_SUN8IW9P1
#include "cfgs-sun8iw9.h"
#elif CONFIG_ARCH_SUN50IW1P1
#include "cfgs-sun50iw1.h"
#elif CONFIG_ARCH_SUN50IW2P1
#include "cfgs-sun50iw2.h"
#elif CONFIG_ARCH_SUN50IW3P1
#include "cfgs-sun50iw3.h"
#elif CONFIG_ARCH_SUN50IW6P1
#include "cfgs-sun50iw6.h"
#else /* other platforms */
#err "not support platform!!"
#endif

#if RSB_USED
#define RSB_SCK_FREQ                    (1000000)       /* the rsb SCK freq(3M) */
#define RSB_SDAODLY                     (1)             /* SDA output delay */
#define RSB_TRANS_BYTE_MAX              (4)             /* the max number of RSB transfer byte */
#endif

#if PMU_USED
#define AXP_TRANS_BYTE_MAX              (8)             /* the max number of pmu transfer byte */
#define PMU_DCDC3_STEP_DELAY_US         (40)            /* reference from pmu spec */
#else
#define PMU_DCDC3_STEP_DELAY_US         (40)            /* reference from sy8106a spec */
#endif

#if TWI_USED || RSB_USED
#define TWI_CLOCK_FREQ                  (400 * 1000)    /* the twi source clock freq */
#define TWI_OPS_USED            (1)     /* config here */
#endif

#if HW_MESSAGE_USED
#define MESSAGE_CACHED_MAX              (4)             /* the max number of cached message frame */
#define MESSAGE_POOL_START              (0x13000)       /* the address of message pool, message pool size 4KByte.*/
#define MESSAGE_POOL_END                (0x14000)
#define SPINLOCK_TIMEOUT                (100)           /* spinlock max timeout, base on ms */
#define SEND_MSG_TIMEOUT                (4000)          /* send message max timeout, base on ms */
#define HWMSGBOX_AR100_ASYN_TX_CH       (0)             /* hwmsgbox channels configure */
#define HWMSGBOX_AR100_ASYN_RX_CH       (1)
#define HWMSGBOX_AR100_SYN_TX_CH        (2)
#define HWMSGBOX_AR100_SYN_RX_CH        (3)
#define HWMSGBOX_AC327_SYN_TX_CH        (4)
#define HWMSGBOX_AC327_SYN_RX_CH        (5)
#define HWMSGBOX_SH_WAKEUP_AP_CH        (6)
#define HWMSGBOX_AP_WAKEUP_SH_CH        (7)
#endif

/*
 * function define
 */
#ifdef BOOT_USED
#define SYSCONFIG_USED          (1)     /* in BOX MODE use sysconfig */
#define SYS_CONFIG_START                (0x44000000)    /* sysconfig.bin at 0x44000000, in all platform */
#define SYS_CONFIG_END                  (0x44010000)
#else
#define SYSCONFIG_USED          (0)     /* config here */
#endif

#if (PMU_USED && (defined KERNEL_USED))
#define PMU_CONFIG_VOL_USED     (1)
#define SWITCH_CPUS_CLK_SRC_EN  (1)
#define PIN_INIT_BY_CPUS        (0)
#elif (PMU_USED && (defined TF_USED))
#define PMU_CONFIG_VOL_USED     (1)
#define SWITCH_CPUS_CLK_SRC_EN  (1)
#define PIN_INIT_BY_CPUS        (1)
#else
#define PMU_CONFIG_VOL_USED     (0)
#define SWITCH_CPUS_CLK_SRC_EN  (0)
#define PIN_INIT_BY_CPUS        (1)
#endif


#if PMU_CHRCUR_CRTL_USED
#define CHRCUR_CRTL_PER_SECONDS     (10)     /* config here */
#endif

//dvfs config
#if DVFS_USED
#define DVFS_VF_TABLE_MAX               (16)
#endif

/*
 * param depend on SYSTEM
 */
#if (defined KERNEL_USED)
#define IR_NUM_KEY_SUP                  (1)             /* the number of IR code support */
#elif (defined TF_USED)
#define IR_NUM_KEY_SUP                  (64)            /* the number of IR code support */
#else
#define IR_NUM_KEY_SUP                  (16)            /* the number of IR code support */
#endif
#define IR_CHECK_ADDR_CODE

#if SST_USED || EST_USED || FAKE_POWOFF_USED
#define STANDBY_USED            (1)
#else
#define STANDBY_USED            (0)
#endif

#if PMU_USED && (defined KERNEL_USED)
#define PMU_BAT_USED            (1)     /* config here */ /* battery is not used */
#else
#define PMU_BAT_USED            (0)     /* config here */ /* in TV box proposal, battery is not used */
#endif

#if RSB_USED || TWI_USED
#define I2C_USED                (1)
#else
#define I2C_USED                (0)
#endif

#endif //__CFGS_H__
