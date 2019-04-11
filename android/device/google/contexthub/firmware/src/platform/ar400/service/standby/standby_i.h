
/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                            super-standby module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : standby_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-14
* Descript: standby module internal header.
* Update  : date                auther      ver     notes
*           2012-5-14 8:56:44   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __STANDBY_I_H__
#define __STANDBY_I_H__

#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>

#if STANDBY_USED
//test mode control
#define NSTANDBY_DISABLE_24M 0

#define GPU_PWR_CTRL 1

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1)
#define PLLCTRL_PLL1_EN    (0x1 << 0)
#define PLLCTRL_PLL2_EN    (0x1 << 1)
#define PLLCTRL_PLL3_EN    (0x1 << 2)
#define PLLCTRL_PLL4_EN    (0x1 << 3)
#define PLLCTRL_PLL5_EN    (0x1 << 4)
#define PLLCTRL_PLL6_EN    (0x1 << 5)
#define PLLCTRL_PLL7_EN    (0x1 << 6)
#define PLLCTRL_PLL8_EN    (0x1 << 7)
#define PLLCTRL_PLLMIPI_EN (0x1 << 8)
#define PLLCTRL_PLL9_EN    (0x1 << 9)
#define PLLCTRL_PLL10_EN   (0x1 << 10)
#elif (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
#define PLLCTRL_PLL1_EN    (0x1 << 0)
#define PLLCTRL_PLL2_EN    (0x1 << 1)
#define PLLCTRL_PLL3_EN    (0x1 << 2)
#define PLLCTRL_PLL4_EN    (0x1 << 3)
#define PLLCTRL_PLL5_EN    (0x1 << 4)
#define PLLCTRL_PLL6_EN    (0x1 << 5)
#define PLLCTRL_PLL7_EN    (0x1 << 6)
#define PLLCTRL_PLL8_EN    (0x1 << 7)
#define PLLCTRL_PLL9_EN    (0x1 << 8)
#define PLLCTRL_PLL10_EN   (0x1 << 9)
#define PLLCTRL_PLL11_EN   (0x1 << 10)
#elif (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
#define PLLCTRL_PLL1_EN    (0x1 << 0)
#define PLLCTRL_PLL2_EN    (0x1 << 1)
#define PLLCTRL_PLL3_EN    (0x1 << 2)
#define PLLCTRL_PLL4_EN    (0x1 << 3)
#define PLLCTRL_PLL5_EN    (0x1 << 4)
#define PLLCTRL_PLL6_EN    (0x1 << 5)
#define PLLCTRL_PLL7_EN    (0x1 << 6)
#define PLLCTRL_PLL8_EN    (0x1 << 7)
#define PLLCTRL_PLL9_EN    (0x1 << 8)
#define PLLCTRL_PLL10_EN   (0x1 << 9)
#define PLLCTRL_PLL11_EN   (0x1 << 10)
#define PLLCTRL_PLL12_EN   (0x1 << 11)
#define PLLCTRL_PLL13_EN   (0x1 << 12)
#endif

#define PWR_CTRL_CPUS      (0x1 << 0)
#define PWR_CTRL_IO        (0x1 << 1)
#define PWR_CTRL_GPU       (0x1 << 2)
#define PWR_CTRL_CPU0      (0x1 << 3)
#define PWR_CTRL_SYS       (0x1 << 4)
#define PWR_CTRL_DRAM      (0x1 << 5)
#define PWR_CTRL_WIFI      (0x1 << 6)
#define PWR_CTRL_AVCC      (0x1 << 7)
#define PWR_CTRL_EXDEV     (0x1 << 8)
#define PWR_CTRL_DLL       (0x1 << 9)

typedef enum POWER_SCENE_FLAGS
{
	TALKING_STANDBY_FLAG           = (1<<0x0),
	USB_STANDBY_FLAG               = (1<<0x1),
	MP3_STANDBY_FLAG               = (1<<0x2),
	SUPER_STANDBY_FLAG             = (1<<0x3),
	NORMAL_STANDBY_FLAG            = (1<<0x4),
	GPIO_STANDBY_FLAG              = (1<<0x5),
	MISC_STANDBY_FLAG              = (1<<0x6),
} power_scene_flags;

typedef enum {
	arisc_power_on = 0,
	arisc_power_retention = 1,
	arisc_power_off = 3,
} arisc_power_state_t;

typedef enum {
	arisc_system_shutdown = 0,
	arisc_system_reboot = 1,
	arisc_system_reset = 2
} arisc_system_state_t;

/*
//macro for axp wakeup source event
#define CPUS_WAKEUP_NMI     (CPUS_WAKEUP_LOWBATT   | \
							 CPUS_WAKEUP_USB       | \
							 CPUS_WAKEUP_AC        | \
							 CPUS_WAKEUP_ASCEND    | \
							 CPUS_WAKEUP_DESCEND   | \
							 CPUS_WAKEUP_SHORT_KEY | \
							 CPUS_WAKEUP_LONG_KEY)
*/
s32 fake_power_off_process(void);

s32 standby_dram_crc_enable(void);
u32 standby_dram_crc(void);

s32 pll_regs_backup(struct pll_reg *plls);
s32 pll_regs_restore(struct pll_reg *plls);

//dram traning area backup
#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
extern u8 dram_traning_area_back[DRAM_TRANING_SIZE];
#endif

//dram crc
extern u32 before_crc;
extern u32 after_crc;
extern struct standby_info_para sst_info;
extern struct sst_power_info_para power_chk_back;

#endif
#endif  //__STANDBY_I_H__
