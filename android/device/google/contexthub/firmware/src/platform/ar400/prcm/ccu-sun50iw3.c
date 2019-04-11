/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                         clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : ccu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: clock control unit module.
* Update  : date                auther      ver     notes
*           2012-5-7 8:43:10    Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "ccu_i-sun50iw3.h"

#if (defined CONFIG_ARCH_SUN50IW3P1)

//ccu module registers base address
struct ccu_reg_list *ccu_reg_addr;
struct ccu_pll_c0_cpux_reg0000 *ccu_pll_c0_cpux_reg_addr;
struct ccu_pll_periph0_reg0020 *ccu_pll_periph0_reg_addr;

//apb clock change notifier list
struct notifier *apbs2_notifier_head;
u32 iosc_freq = 16000000;
u32 losc_freq = 31250;
volatile static u32 already_init_osc_freq = 0;

void osc_freq_init(void)
{
       u64 cnt1, cnt2;
       u32 xt, ht, xf;
       u32 cpus_src;

	if (already_init_osc_freq == 0) {
		cpus_src = ccu_get_mclk_src(CCU_MOD_CLK_CPUS);
		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_HOSC);
		time_cdelay(1600);
		//cpucfg_counter_clear();
		cnt1 = cpucfg_counter_read();
		time_cdelay(1000000);
		cnt2 = cpucfg_counter_read() - cnt1;
		ht = ((u32)(cnt2 & 0xffffffff))/24000;

		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_IOSC);
		time_cdelay(1600);
		//cpucfg_counter_clear();
		cnt1 = cpucfg_counter_read();
		time_cdelay(1000000);
		cnt2 = cpucfg_counter_read() - cnt1;
		xt = ((u32)(cnt2 & 0xffffffff))/24000;
		xf = (24000 * ht)/xt;
		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, cpus_src);

		iosc_freq = xf * 1000;
		losc_freq = iosc_freq/512;

		already_init_osc_freq = 1;
	}
}

/*
*********************************************************************************************************
*                                       INITIALIZE CCU
*
* Description:  initialize clock control unit.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize ccu succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_init(void)
{

	//initialize ccu register address
	ccu_reg_addr = (struct ccu_reg_list *)R_PRCM_REG_BASE;
	ccu_pll_c0_cpux_reg_addr = (struct ccu_pll_c0_cpux_reg0000 *)CCU_PLL_C0_REG;
	ccu_pll_periph0_reg_addr = (struct ccu_pll_periph0_reg0020 *)CCU_PLL_PERIPH0_REG;
#ifndef FPGA_PLATFORM
	//set systick clk src to osc24M
	ccu_set_mclk_src(CCU_MOD_CLK_SYSTICK, CCU_SYS_CLK_HOSC);
	//setup cpus post div source to 200M(CCU_CPUS_POST_DIV)
	u32 value;
	value = (ccu_get_sclk_freq(CCU_SYS_CLK_PLL5)) / CCU_CPUS_POST_DIV;
	if (value < 1) {
		//to avoid PLL5 freq less than CCU_CPUS_POST_DIV
		value = 1;
	}
	ccu_reg_addr->cpus_clk_cfg.factor_m = value - 1;
	//set ar100 clock source to PLL5
	ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_PLL5);
	//ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_HOSC);
#endif

	//initialize apb notifier list
	apbs2_notifier_head = NULL;

	if (((u32)(&(ccu_reg_addr->prcm_version)) == (R_PRCM_REG_BASE + 0x3f0)) && (ccu_reg_addr->prcm_version == 0x00010002))
		//ccu initialize succeeded
		return OK;
	else
		//while(1);
		return FAIL;
}

/*
*********************************************************************************************************
*                                       EXIT CCU
*
* Description:  exit clock control unit.
*
* Arguments  :  none.
*
* Returns    :  OK if exit ccu succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_exit(void)
{
	ccu_pll_c0_cpux_reg_addr = NULL;
	ccu_reg_addr      = NULL;
	ccu_pll_periph0_reg_addr = NULL;

	return OK;
}

void save_state_flag(u32 value)
{
	writel(value, RTC_RECORD_REG);
}

u32 read_state_flag(void)
{
	return readl(RTC_RECORD_REG);
}

void write_rtc_domain_reg(u32 reg, u32 value)
{
	writel(value, reg);
}

u32 read_rtc_domain_reg(u32 reg)
{
	return readl(reg);
}
#endif // sun50iw3
