/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 pin module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pin_int.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-8
* Descript: interrupt pin managment module.
* Update  : date                auther      ver     notes
*           2012-5-8 14:25:55   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "pin_i.h"

static int pin_pin2eint(u32 pin_grp, u32 pin_num, u32 *eint_grp, u32 *eint_num)
{
	*eint_grp = pin_grp;
	*eint_num = pin_num;
	return OK;
}

/*
*********************************************************************************************************
*                                      SET PIN INT TRIGGER MODE
*
* Description:  set the trigger mode of external interrupt pin.
*
* Arguments  :  pin_grp : the group number of the specific pin.
*               pin_num : the pin number of the specific pin.
*               mode    : the trigger mode which we want to set.
*
* Returns    :  OK if set trigger mode succeeded, others if failed.
*********************************************************************************************************
*/
int pin_set_int_trigger_mode(u32 pin_grp, u32 pin_num, u32 mode)
{
	u32            eint_grp = 0;
	u32            eint_num = 0;
	volatile u32  *addr;
	volatile u32   value;

	//convert pin number to eint number
	pin_pin2eint(pin_grp, pin_num, &eint_grp, &eint_num);

	//set eint pin trigger mode
	addr = PIN_REG_INT_CFG(eint_grp, eint_num);
	value = *addr;
	value &= ~(0xf  << ((eint_num & 0x7) * 4));
	value |=  (mode << ((eint_num & 0x7) * 4));
	*addr = value;

	return OK;
}

/*
*********************************************************************************************************
*                                      ENABLE PIN INT
*
* Description:  enable the interrupt of external interrupt pin.
*
* Arguments  :  pin_grp : the group number of the specific pin.
*               pin_num : the pin number of the specific pin.
*
* Returns    :  OK if enable pin interrupt succeeded, others if failed.
*********************************************************************************************************
*/
int pin_enable_int(u32 pin_grp, u32 pin_num)
{
	u32            eint_grp = 0;
	u32            eint_num = 0;
	volatile u32  *addr;
	volatile u32   value;

	//convert pin number to eint number
	pin_pin2eint(pin_grp, pin_num, &eint_grp, &eint_num);

	//enable pin interrupt
	addr   = PIN_REG_INT_CTL(eint_grp);
	value  = *addr;
	value |= (0x1 << (eint_num));
	*addr  = value;

	return OK;
}

/*
*********************************************************************************************************
*                                      DISABLE PIN INT
*
* Description:  disable the interrupt of external interrupt pin.
*
* Arguments  :  pin_grp : the group number of the specific pin.
*               pin_num : the pin number of the specific pin.
*
* Returns    :  OK if disable pin interrupt succeeded, others if failed.
*********************************************************************************************************
*/
int pin_disable_int(u32 pin_grp, u32 pin_num)
{
	u32            eint_grp = 0;
	u32            eint_num = 0;
	volatile u32  *addr;
	volatile u32   value;

	//convert pin number to eint number
	pin_pin2eint(pin_grp, pin_num, &eint_grp, &eint_num);

	//disable pin interrupt
	addr   = PIN_REG_INT_CTL(eint_grp);
	value  = *addr;
	value &= ~(0x1 << (eint_num));
	*addr  = value;

	return OK;
}

/*
*********************************************************************************************************
*                                      QUERY PENDING
*
* Description:  query the pending of interrupt of external interrupt pin.
*
* Arguments  :  pin_grp : the group number of the specific pin.
*               pin_num : the pin number of the specific pin.
*
* Returns    :  the pending status of external interrupt.
*********************************************************************************************************
*/
u32 pin_query_pending(u32 pin_grp, u32 pin_num)
{
	u32            eint_grp = 0;
	u32            eint_num = 0;
	volatile u32  *addr;
	volatile u32   value;

	//convert pin number to eint number
	pin_pin2eint(pin_grp, pin_num, &eint_grp, &eint_num);

	//query pending
	addr   = PIN_REG_INT_STAT(eint_grp);
	value  = *addr;

	return (value & (0x1 << (eint_num)));
}

/*
*********************************************************************************************************
*                                      CLEAR PENDING
*
* Description:  clear the pending of interrupt of external interrupt pin.
*
* Arguments  :  pin_grp : the group number of the specific pin.
*               pin_num : the pin number of the specific pin.
*
* Returns    :  OK if clear pending succeeded, others if failed.
*********************************************************************************************************
*/
int pin_clear_pending(u32 pin_grp, u32 pin_num)
{
	u32            eint_grp = 0;
	u32            eint_num = 0;
	volatile u32  *addr;
	volatile u32   value;

	//convert pin number to eint number
	pin_pin2eint(pin_grp, pin_num, &eint_grp, &eint_num);

	//clear pending
	addr   = PIN_REG_INT_STAT(eint_grp);
	value  = (0x1 << (eint_num));
	*addr  = value;

	return OK;
}

#if STANDBY_USED
static u32 pl_int_en, pm_int_en;

int pin_standby_init(u32 gpio_enable_bitmap)
{
	u32 i;

	pl_int_en = readl(PIN_REG_INT_CTL(PIN_GRP_PL));
	pm_int_en = readl(PIN_REG_INT_CTL(PIN_GRP_PM));

	writel(0x0, PIN_REG_INT_CTL(PIN_GRP_PL));
	writel(0x0, PIN_REG_INT_CTL(PIN_GRP_PM));

	writel(0xffffffff, PIN_REG_INT_STAT(PIN_GRP_PL));
	writel(0xffffffff, PIN_REG_INT_STAT(PIN_GRP_PM));

	for (i = 0; i < PL_NUM; i++)
	{
		if (gpio_enable_bitmap & CPUS_GPIO_PL(i))
		{
			pin_enable_int(PIN_GRP_PL, i);
		}
	}
	for (i = 0; i < PM_NUM; i++)
	{
		if (gpio_enable_bitmap & CPUS_GPIO_PM(i))
		{
			pin_enable_int(PIN_GRP_PM, i);
		}
	}

	return OK;
}

int pin_standby_exit(void)
{
	writel(pl_int_en, PIN_REG_INT_CTL(PIN_GRP_PL));
	writel(pm_int_en, PIN_REG_INT_CTL(PIN_GRP_PM));

	return OK;
}

#endif
