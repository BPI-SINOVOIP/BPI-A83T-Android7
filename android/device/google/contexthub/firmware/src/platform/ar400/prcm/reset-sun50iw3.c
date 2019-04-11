/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                         clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : reset.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-11-22
* Descript: reset control of a module.
* Update  : date                auther      ver     notes
*           2012-11-22 16:55:22 Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#include "ccu_i-sun50iw3.h"

#if (defined CONFIG_ARCH_SUN50IW3P1)

/*
*********************************************************************************************************
*                                    SET RESET STATUS OF MODULE CLOCK
*
* Description:  set the reset status of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to set reset status.
*       reset   : the reset status which we want to set, the detail please
*             refer to the clock status of reset.
*
* Returns    :  OK if set module clock reset status succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_mclk_reset(u32 mclk, s32 reset)
{
	switch (mclk)
	{
		case CCU_MOD_CLK_R_TWI0:
		{
			ccu_reg_addr->r_twi.twi0_reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_TWI1:
		{
			ccu_reg_addr->r_twi.twi1_reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_TWI2:
		{
			ccu_reg_addr->r_twi.twi2_reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_UART:
		{
			ccu_reg_addr->r_uart.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_TIMER0_1:
		{
			ccu_reg_addr->r_timer.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_TWD:
		{
			ccu_reg_addr->r_twd.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_PWM:
		{
			ccu_reg_addr->r_pwm.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_ONEWIRE:
		{
			ccu_reg_addr->r_owc.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_RTC:
		{
			ccu_reg_addr->r_owc.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_RSB:
		{
			ccu_reg_addr->r_rsb.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_CIR:
		{
			ccu_reg_addr->r_ir.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_R_SPI:
		{
			ccu_reg_addr->r_spi.reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_VDD_SYS:
		{
			ccu_reg_addr->sys_pwr_rst.module_reset = reset;
			return OK;
		}
		case CCU_MOD_CLK_SPINLOCK:
		{
			writel(((readl(CCU_SPINLOCK_BGR_REG) & (~(0x1 << 16)))
									 | (reset << 16)), CCU_SPINLOCK_BGR_REG);
			return OK;
		}
		case CCU_MOD_CLK_MSGBOX:
		{
			writel(((readl(CCU_MSGBOX_BGR_REG) & (~(0x1 << 16)))
									 | (reset << 16)), CCU_MSGBOX_BGR_REG);
			return OK;
		}
		default:
		{
			WRN("invaid module clock id (%d) when set reset\n", mclk);
			return -EINVAL;
		}
	}
	//un-reached
}

s32 ccu_reset_module(u32 mclk)
{
	//module reset method: set as reset valid->set as reset invalid
	ccu_set_mclk_reset(mclk, CCU_CLK_RESET);
	ccu_set_mclk_reset(mclk, CCU_CLK_NRESET);

	return OK;
}
#endif // sun50iw3
