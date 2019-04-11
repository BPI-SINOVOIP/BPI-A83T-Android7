/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                thermal  module
*
*                                    (c) Copyright 2012-2016, Superm Wu China
*                                             All Rights Reserved
*
* File    : thermal.c
* By      : Superm Wu
* Version : v1.0
* Date    : 2012-9-18
* Descript: thermal controller public interfaces.
* Update  : date                auther      ver     notes
*           2012-9-18 19:08:23  Superm Wu   1.0     Create this file.
*********************************************************************************************************
*/
#include "thermal_i.h"
#if THERMAL_USED

/*
*********************************************************************************************************
*                                       THERMAL INTERRUPT HANDLER
*
* Description:  thermal interrupt handler.
*
* Arguments  :  none.
*
* Returns    :  OK if enable watchdog succeeded, others if failed.
*********************************************************************************************************
*/
static s32 thermal_int_handler(void *p_arg)
{
	volatile u32 value;

	//disable axp interrupt
	interrupt_disable(INTC_R_TH_IRQ);

	value = readl(THS_STAT);
	ERR("THS_STAT:0x%x\n", value);

	if (value & 0x70)
	{
		ERR("SHUTDOWN SOC\n");
		save_state_flag(REC_SHUTDOWN | ((value & 0x70) >> 4));
		pmu_shutdown();
	}

	//cpus should not clean the THS_STAT reg, because the cpux will response these interrupts
	//clear interrupt flag first
	interrupt_clear_pending(INTC_R_TH_IRQ);
	interrupt_enable(INTC_R_TH_IRQ);

	return OK;
}

/*
*********************************************************************************************************
*                                       INIT THERMAL
*
* Description:  initialize thermal.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize thermal succeeded, others if failed.
*********************************************************************************************************
*/
s32 thermal_init(void)
{
	//register thermal interrupt handler.
	install_isr(INTC_R_TH_IRQ, thermal_int_handler, NULL);
	interrupt_enable(INTC_R_TH_IRQ);

	return OK;
}

#endif /* THERMAL_USED */
