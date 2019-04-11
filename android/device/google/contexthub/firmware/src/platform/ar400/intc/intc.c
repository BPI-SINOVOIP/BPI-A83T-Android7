/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                              interrupt  module
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : intc.c
* By      : Superm
* Version : v1.0
* Date    : 2012-5-3
* Descript: interrupt controller module.
* Update  : date                auther      ver     notes
*           2012-5-3 13:25:40   Superm       1.0     Create this file.
*********************************************************************************************************
*/

#include "intc_i.h"

struct intc_regs *pintc_regs;

/*
*********************************************************************************************************
*                                           INTERRUPT INIT
*
* Description:  initialize interrupt.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
int intc_init(void)
{
	uint32_t i;

	/* set ints up for a sane state
	 * 3 bits preemptPriority, 1 bit subPriority
	 */
	NVIC_SetPriorityGrouping(4);
	for (i = 0; i < NUM_INTERRUPTS; i++) {
		NVIC_SetPriority(i, NVIC_EncodePriority(4, 2, 1));
		NVIC_DisableIRQ(i);
		NVIC_ClearPendingIRQ(i);
	}

	return OK;
}

/*
*********************************************************************************************************
*                                         INTERRUPT EXIT
*
* Description:  exit interrupt.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
int intc_exit(void)
{
	return OK;
}

/*
*********************************************************************************************************
*                                           ENABLE INTERRUPT
*
* Description:  enable a specific interrupt.
*
* Arguments  :  intno   : the source number of interrupt to which we want to enable.
*
* Returns    :  OK if enable interrupt succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
int intc_enable_interrupt(uint32_t intno)
{
	//intno can't beyond then IRQ_SOURCE_MAX
	ASSERT(intno < NUM_INTERRUPTS);

	if (intno < NUM_INTERRUPTS) {
		NVIC_EnableIRQ(intno);
	}
	return OK;
}

/*
*********************************************************************************************************
*                                           DISABLE INTERRUPT
*
* Description:  disable a specific interrupt.
*
* Arguments  :  intno  : the source number of interrupt which we want to disable.
*
* Returns    :  OK if disable interrupt succeeded, others if failed.
*
* Note       :
*********************************************************************************************************
*/
int intc_disable_interrupt(uint32_t intno)
{
	//intno can't beyond then IRQ_SOURCE_MAX
	ASSERT(intno < NUM_INTERRUPTS);

	if (intno < NUM_INTERRUPTS) {
		NVIC_DisableIRQ(intno);
	}

	return OK;
}

int intc_disable_all(void)
{
	cpuIntsOff();

	return OK;
}

int intc_enable_all(void)
{
	cpuIntsOn();

	return OK;
}

/*
*********************************************************************************************************
*                                   GET CURRENT INTERRUPT
*
* Description: get the source number of current interrupt, exclude NO.0-NO.15 exception.
*
* Arguments  : none.
*
* Returns    : the source number of current interrupt.
*
* Note       :
*********************************************************************************************************
*/
uint32_t intc_get_current_interrupt(void)
{
	if ((SCB->ICSR & 0x3ff) >= 16)
	{
		return (SCB->ICSR & 0x3ff) - 16;
	}
	else
	{
		WRN("the int nr < 0, pls use intc_get_current_exception()\n");
		WRN("exception nr:%lu\n", intc_get_current_exception());
		return 0xff;
	}

}

/*
*********************************************************************************************************
*                                   GET CURRENT EXCETOPM
*
* Description: get the source number of current exception, include all exceptions.
*
* Arguments  : none.
*
* Returns    : the source number of current exception.
*
* Note       :
*********************************************************************************************************
*/
uint32_t intc_get_current_exception(void)
{
	return (SCB->ICSR & 0x3ff);
}

