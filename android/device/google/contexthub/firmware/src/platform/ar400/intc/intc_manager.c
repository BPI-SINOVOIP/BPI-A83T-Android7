/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                               interrupt manager
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : interrupt_manager.c
* By      : Superm
* Version : v1.0
* Date    : 2012-5-3
* Descript: the manager of interrupt.
* Update  : date                auther      ver     notes
*           2012-5-3 10:45:15   Superm       1.0     Create this file.
*********************************************************************************************************
*/

#include "intc_i.h"
#include "../prcm/ccu_i-sun50iw3.h"

extern struct ccu_reg_list *ccu_reg_addr;

struct isr_node isr_table[NUM_INTERRUPTS];

/*
*********************************************************************************************************
*                                       INIT INTERRUPT MANAGER
*
* Description:  initialize interrupt manager.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize interrupt manager succeeded, others if failed.
*********************************************************************************************************
*/
s32 interrupt_init(void)
{
	int index;

	//initialize interrupt controller
	intc_init();

	//initialize ISR table
	for (index = 0; index < NUM_INTERRUPTS; index++)
	{
		isr_table[index].pisr = isr_default;
		isr_table[index].parg = NULL;
	}

	//interrupt manager initialize succeeded
	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT INTERRUPT MANAGER
*
* Description:  exit interrupt manager.
*
* Arguments  :  none.
*
* Returns    :  OK if exit interrupt manager succeeded, others if failed.
*********************************************************************************************************
*/
s32 interrupt_exit(void)
{
	intc_exit();

	return OK;
}

/*
*********************************************************************************************************
*                                               ENABLE INTERRUPT
*
* Description:  enable a specific interrupt.
*
* Arguments  :  intno : the number of interrupt which we want to enable.
*
* Returns    :  OK if enable interrupt succeeded, others if failed.
*********************************************************************************************************
*/
s32 interrupt_enable(u32 intno)
{
	if (intno == EXT_NMI_IRQn)
	{
		ccu_reg_addr->nmi_int_en.nmi_irq_en = 1;
	}

	return intc_enable_interrupt(intno);
}

/*
*********************************************************************************************************
*                                               DISABLE INTERRUPT
*
* Description:  disable a specific interrupt.
*
* Arguments  :  intno : the number of interrupt which we want to disable.
*
* Returns    :  OK if disable interrupt succeeded, others if failed.
*********************************************************************************************************
*/
s32 interrupt_disable(u32 intno)
{
	if (intno == EXT_NMI_IRQn)
	{
		ccu_reg_addr->nmi_int_en.nmi_irq_en = 0;
	}

	return intc_disable_interrupt(intno);
}

/*
*********************************************************************************************************
*                                           SET NMI TRIGGER
*
* Description:  set nmi trigger.
*
* Arguments  :  type : the trigger type.
*
* Returns    :  OK if set trigger type succeeded, others if failed.
*********************************************************************************************************
*/
s32 interrupt_set_nmi_trigger(u32 type)
{
	ccu_reg_addr->nmi_int_ctrl.nmi_src_type = type;

	return OK;
}

/*
*********************************************************************************************************
*                                               INSTALL ISR
*
* Description:  install ISR for a specific interrupt.
*
* Arguments  :  intno   : the number of interrupt which we want to install ISR.
*               pisr    : the ISR which to been install.
*               parg    : the argument for the ISR.
*
* Returns    :  OK if install ISR succeeded, others if failed.
*
* Note       :  the ISR execute entironment : CPU disable interrupt response.
*********************************************************************************************************
*/
s32 install_isr(u32 intno, __pISR_t pisr, void *parg)
{
	//intno can't beyond then IRQ_SOURCE_MAX
	ASSERT(intno < NUM_INTERRUPTS);

	//default isr, install directly
	INF("install isr %d\n", intno);
	isr_table[intno].pisr = pisr;
	isr_table[intno].parg = parg;

	return OK;
}

/*
*********************************************************************************************************
*                                               UNINSTALL ISR
*
* Description:  uninstall ISR for a specific interrupt.
*
* Arguments  :  intno   : the number of interrupt which we want to uninstall ISR.
*                               pisr    : the ISR which to been uninstall.
*
* Returns    :  OK if uninstall ISR succeeded, others if failed.
*********************************************************************************************************
*/
int uninstall_isr(u32 intno, __pISR_t pisr)
{
	//intno can't beyond then IRQ_SOURCE_MAX
	ASSERT(intno < NUM_INTERRUPTS);

	if (isr_table[intno].pisr == pisr)
	{
		//uninstall isr
		isr_table[intno].pisr = isr_default;
		isr_table[intno].parg = NULL;
	}
	else
	{
		//don't support shared interrupt now,
		//by superm at 2012-5-3 11:20:28.
		ERR("ISR not installed!\n");
		return -ENODEV;
	}

	return OK;
}

/*
*********************************************************************************************************
*                                               INTERRUPT ENTRY
*
* Description:  the entry of CPU IRQ, mainly for CPU IRQ exception.
*
* Arguments  :  none.
*
* Returns    :  OK if process CPU IRQ succeeded, others if failed.
*********************************************************************************************************
*/
int interrupt_entry(void)
{
	u32 intno;

	intno = intc_get_current_interrupt();

	//intno can't beyond then IRQ_SOURCE_MAX
	//ASSERT(intno < NUM_INTERRUPTS);

	//process interrupt by call isr,
	//not support shared intterrupt.
	//INF("interrupt entry, intno:%x\n", intno);
	if (intno < NUM_INTERRUPTS)
		(isr_table[intno].pisr)(isr_table[intno].parg);

	//clear pending
	interrupt_clear_pending(intno);

	return OK;
}

/*
*********************************************************************************************************
*                                               EXCEPTION ENTRY
*
* Description:  the entry of CPU exception, mainly for CPU nmi, hard fault,
* memmanger fault, bus fault, usage fault, SVCall, debugmonitor exception.
*
* register list in stack:
* ---------LOW ADDR--------
* R11           <-PSP(pstack)
* R10
* R9
* R8
* R7
* R6
* R5
* R4            <-PSP+4*7
* R0            <-PSP+4*8
* R1
* R2
* R3
* R12           <-PSP+4*12
* R14(LR)       <-PSP+4*13
* R15(PC)       <-PSP+4*14
* xPSR          <-PSP+4*15
* --------HIGH ADDR--------
*
* Arguments  :  pstack:the pointer of stack, PSP before the exception happen.
*
* Returns    :  OK if process CPU exception succeeded, others if failed.
*********************************************************************************************************
*/
int exception_entry(u32 *pstack)
{
	u32 i;
	u32 exceptionno = intc_get_current_exception();

	//intno can't beyond then IRQ_SOURCE_MAX
	ASSERT(exceptionno < (NUM_INTERRUPTS + 16));
	ERR("pstack:%p\n", pstack);
	ERR("exception:%d happen\n", exceptionno);
	//print R0-R3
	for (i = 0; i <=3; i++)
	{
		ERR("R%d:%p:0x%x\n", i, (pstack + (8 + i)), *(pstack + (8 + i)));
	}
	//print R4-R11
	for (i = 0; i <=7; i++)
	{
		ERR("R%d:%p:0x%x\n", i + 4, (pstack + (7 - i)), *(pstack + (7 - i)));
	}
	//print R12, R14(LR), R15(PC), xPSR
	ERR("R12:%p:0x%x\n", (pstack + 12), *(pstack + 12));
	ERR("R14(LR):%p:0x%x\n", (pstack + 13), *(pstack + 13));
	ERR("R15(PC):%p:0x%x\n", (pstack + 14), *(pstack + 14));
	ERR("xPSR:%p:0x%x\n", (pstack + 15), *(pstack + 15));

	switch (exceptionno)
	{
	case 2:
	{
		ERR("NMI happen\n");
		break;
	}
	case 3:
	{
		ERR("hard fault happen, HFSR:0x%x\n", (u32)SCB->HFSR);
		ERR("memm fault maybe happen, MFSR:0x%x, MMFAR:0x%x\n", readb(0xE000ED28), (u32)SCB->MMFAR);
		ERR("bus fault maybe happen, BFSR:0x%x, BFAR:0x%x\n", readb(0xE000ED29), (u32)SCB->BFAR);
		ERR("usage fault maybe happen, UFSR:0x%x\n", readw(0xE000ED2A));
		break;
	}
	case 4:
	{
		ERR("memm fault happen, MFSR:0x%x, MMFAR:0x%x\n", readb(0xE000ED28), (u32)SCB->MMFAR);
		break;
	}
	case 5:
	{
		ERR("bus fault happen, BFSR:0x%x, BFAR:0x%x\n", readb(0xE000ED29), (u32)SCB->BFAR);
		break;
	}
	case 6:
	{
		ERR("usage fault happen, UFSR:0x%x\n", readw(0xE000ED2A));
		break;
	}
	case 11:
	{
		ERR("SVCall fault happen\n");
		break;
	}
	case 12:
	{
		ERR("SVCall fault happen, DFSR:0x%x\n", (u32)SCB->DFSR);
		break;
	}
	default:
	{
		//invalid exception nr
		ERR("invalid exception nr\n");
	}
	}
	// if happen fault, print important information and drop-dead halt
	while(1);

	return OK;
}

int interrupt_query_pending(u32 intno)
{
	return NVIC_GetPendingIRQ(intno);
}

int interrupt_clear_pending(u32 intno)
{
	if (intno == EXT_NMI_IRQn)
	{
		ccu_reg_addr->nmi_int_st.nmi_irq_pend = 1;
	}
	NVIC_ClearPendingIRQ(intno);

	return OK;
}

int isr_default(void *arg)
{
	u32 irq = intc_get_current_interrupt();
	ERR("irq [%x] enable before ISR install\n", irq);

	return OK;
}


//the backup of enable and mask register
static u32 intc_enable[8];
static u32 intc_pend[8];
static u32 prcm_nmi_irq_en;

int interrupt_standby_enter(void)
{
	u8 i;

	prcm_nmi_irq_en = ccu_reg_addr->nmi_int_en.nmi_irq_en;

	//backup SETEN and SETPEND registers
	for (i = 0; i < 8; i++)
	{
		intc_enable[i] = NVIC->ISER[i];
		intc_pend[i] = NVIC->ISPR[i];
	}

	//disable all interrupt and clear all pend registers
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xffffffff;
		NVIC->ICPR[i] = 0xffffffff;
	}

	return OK;
}

int interrupt_standby_exit(void)
{
	u8 i;

	ccu_reg_addr->nmi_int_st.nmi_irq_pend = 1;
	ccu_reg_addr->nmi_int_en.nmi_irq_en = prcm_nmi_irq_en;

	//disable all interrupt and clear all pend registers
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xffffffff;
		NVIC->ICPR[i] = 0xffffffff;
	}

	//restore registers
	for (i = 0; i < 8; i++)
	{
		NVIC->ISER[i] = intc_enable[i];
		NVIC->ISPR[i] = intc_pend[i];
	}

	return OK;
}
