/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                pmu module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pmu.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: power management unit.
* Update  : date                auther      ver     notes
*           2012-5-22 13:33:03  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "pmu_i-sun50iw3.h"

#if (defined CONFIG_ARCH_SUN50IW3P1)
#if PMU_USED

#if ((defined KERNEL_USED) || (defined TF_USED))
extern struct ccu_reg_list *ccu_reg_addr;
static u32 pmu_discharge_ltf = 0; //pmu discharge ltf
static u32 pmu_discharge_htf = 0; //pmu discharge htf
static u32 bat_charge_temp = 0; /* the max temperature of PMU IC */
static u32 pmu_max_chg_cur = 0; /* the max charge current of battery */
#endif

#if (defined CONFIG_ARCH_SUN50IW3P1)
extern u32 rsb_audio_used;
#endif

//voltages info table,
//the index of table is voltage type.
#if PMU_CONFIG_VOL_USED
voltage_info_t voltages_table[] =
{
	//dev_addr               //reg_addr             //min1_mV //max1_mV //step1_mV //step1_num //min2_mV //max2_mV  //step2_mV //step2_num //mask //mode_reg //mode_offset
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC1_VOL_CTRL  ,    1600,    3400,    100 ,    19  ,    3400,    3400,    0   ,    0 ,    0x1f,  0x80,     0x0},//AW1660_POWER_DCDC1
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC2_VOL_CTRL  ,    500 ,    1200,    10  ,    71  ,    1220,    1300,    20  ,    5 ,    0x7f,  0x80,     0x1},//AW1660_POWER_DCDC2
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC3_VOL_CTRL  ,    500 ,    1200,    10  ,    71  ,    1220,    1300,    20  ,    5 ,    0x7f,  0x80,     0x2},//AW1660_POWER_DCDC3
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC4_VOL_CTRL  ,    500 ,    1200,    10  ,    71  ,    1220,    1300,    20  ,    5 ,    0x7f,  0x80,     0x3},//AW1660_POWER_DCDC4
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC5_VOL_CTRL  ,    800 ,    1120,    10  ,    33  ,    1140,    1184,    20  ,    37,    0x7f,  0x80,     0x4},//AW1660_POWER_DCDC5
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC6_VOL_CTRL  ,    600 ,    1100,    10  ,    51  ,    1120,    1520,    20  ,    21,    0x7f,  0x80,     0x5},//AW1660_POWER_DCDC6
	{RSB_RTSADDR_AW1660 ,    AW1660_DCDC7_VOL_CTRL  ,    600 ,    1100,    10  ,    51  ,    1120,    1520,    20  ,    21,    0x7f,  0x80,     0x6},//AW1660_POWER_DCDC7
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO1_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ALDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO2_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ALDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO3_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ALDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_DLDO1_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_DLDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_DLDO2_VOL_CTRL  ,    700 ,    3400,    100 ,    28  ,    3400,    4200,    200 ,    4 ,    0x1f,  0x00,     0x0},//AW1660_POWER_DLDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_DLDO3_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_DLDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_DLDO4_VOL_CTRL  ,    700 ,    3300,    100 ,    27  ,    3300,    3300,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_DLDO4
	{RSB_RTSADDR_AW1660 ,    AW1660_ELDO1_VOL_CTRL  ,    700 ,    1900,    50  ,    25  ,    1900,    1900,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ELDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_ELDO2_VOL_CTRL  ,    700 ,    1900,    50  ,    25  ,    1900,    1900,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ELDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_ELDO3_VOL_CTRL  ,    700 ,    1900,    50  ,    25  ,    1900,    1900,    0   ,    0 ,    0x1f,  0x00,     0x0},//AW1660_POWER_ELDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_FLDO1_VOL_CTRL  ,    700 ,    1450,    50  ,    16  ,    1450,    1450,    0   ,    0 ,    0x0f,  0x00,     0x0},//AW1660_POWER_FLDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_FLDO23_VOL_CTRL ,    700 ,    1450,    50  ,    16  ,    1450,    1450,    0   ,    0 ,    0x0f,  0x00,     0x0},//AW1660_POWER_FLDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_FLDO23_VOL_CTRL ,    700 ,    1450,    50  ,    16  ,    1450,    1450,    0   ,    0 ,    0x0f,  0x00,     0x0},//AW1660_POWER_FLDO3
	{RSB_INVALID_RTSADDR,    AW1660_INVALID_ADDR    ,    0   ,    0   ,    0   ,    27  ,    0   ,    0   ,    0   ,    0 ,    0x00,  0x00,     0x0},//AW1660_POWER_MAX
};
#endif

pmu_onoff_reg_bitmap_t pmu_onoff_reg_bitmap[] =
{
	//dev_addr               //reg_addr             //offset //state //dvm_en
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    0,    1,    0},//AW1660_POWER_DCDC1
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    1,    1,    0},//AW1660_POWER_DCDC2
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    2,    1,    0},//AW1660_POWER_DCDC3
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    3,    1,    0},//AW1660_POWER_DCDC4
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    4,    1,    0},//AW1660_POWER_DCDC5
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    5,    1,    0},//AW1660_POWER_DCDC6
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL0,    6,    1,    0},//AW1660_POWER_DCDC7
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    5,    0,    0},//AW1660_POWER_ALDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    6,    0,    0},//AW1660_POWER_ALDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    7,    0,    0},//AW1660_POWER_ALDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    3,    1,    0},//AW1660_POWER_DLDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    4,    0,    0},//AW1660_POWER_DLDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    5,    1,    0},//AW1660_POWER_DLDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    6,    1,    0},//AW1660_POWER_DLDO4
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    0 ,   0,    0},//AW1660_POWER_ELDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    1,    0,    0},//AW1660_POWER_ELDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    2,    0,    0},//AW1660_POWER_ELDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    2,    0,    0},//AW1660_POWER_FLDO1
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    3,    0,    0},//AW1660_POWER_FLDO2
	{RSB_RTSADDR_AW1660 ,    AW1660_ALDO123_OP_MODE ,    4,    0,    0},//AW1660_POWER_FLDO3
	{RSB_RTSADDR_AW1660 ,    AW1660_OUTPUT_PWR_CTRL1,    7,    0,    0},//AW1660_POWER_DC1SW
	{RSB_INVALID_RTSADDR,    AW1660_INVALID_ADDR    ,    0,    0,    0},//POWER_ONOFF_MAX
};

s32 pmu_init(void)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;

	//check IC version number match or not
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_IC_NO_REG;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	INF("AW1660 IC_NO_REG value = %x\n", data);

	//enable CPUA(DCDC2) and CPUB(DCDC3) DVM
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_DCDC_DVM_CTRL;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data |= 1 << 2;
	data |= 1 << 3;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	pmu_onoff_reg_bitmap[AW1660_POWER_DCDC2].dvm_st = 1;
	pmu_onoff_reg_bitmap[AW1660_POWER_DCDC3].dvm_st = 1;

	//FIXME:remove when linux ready, just test gpio exti
	//power on vcc-sensor, switch pmu gpio1/ldo
	regaddr = AW1660_GPIOLDO1_CTRL;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ~0x7;
	data |= 0x3;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	//FIXME: NMI response by linux pmu driver
	//clear pendings
	//pmu_clear_pendings();

	//register pmu interrupt handler.
	//install_isr(INTC_R_NMI_IRQ, nmi_int_handler, NULL);

	//interrupt_clear_pending(INTC_R_NMI_IRQ);
	//pmu irq should be enable after linux pmu driver ready.
	//by superm at 2013-1-10 8:58:04.
	//interrupt_enable(INTC_R_NMI_IRQ);

	return OK;
}

s32 pmu_exit(void)
{
	return OK;
}

#if (THERMAL_USED | CHECK_USED | SYSOP_USED)
void pmu_shutdown(void)
{
	u8 devaddr = RSB_RTSADDR_AW1660;
	u8 regaddr = AW1660_PWR_OFF_CTRL;
	u8 data = 1 << 7;

	//power off system
	//disable DCDC & LDO
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
}
#endif

#if SYSOP_USED
void pmu_reset(void)
{
	writel(0x0, CPUX_TIMER_REG_BASE + 0xA0); //disable watchdog int
	writel(0x1, CPUX_TIMER_REG_BASE + 0xB4); //reset whole system
	writel((0x3 << 4), CPUX_TIMER_REG_BASE + 0xB8); //set reset after 3s
	writel((readl(CPUX_TIMER_REG_BASE + 0xB8) | 0x1), CPUX_TIMER_REG_BASE + 0xB8); //enable watchdog
	while (1);
}
#endif

#if PMU_CONFIG_VOL_USED
u32 pmu_output_is_stable(u32 type)
{
	u8 devaddr = voltages_table[type].devaddr;
	u8 regaddr = voltages_table[type].regaddr;
	u8 data = 0;

	pmu_reg_read(&devaddr, &regaddr, &data, 1);

	if (data & (0x1 << 7))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//the voltage base on mV
s32 pmu_set_voltage(u32 type, u32 voltage)
{
	u8 devaddr;
	u8 regaddr;
	u8 step;
	u8 ret = OK;
	voltage_info_t *pvoltage_info;

	if (type >= AW1660_POWER_MAX)
	{
		//invalid power voltage type
		WRN("invalid power voltage type[%u]\n", type);
		return -EINVAL;
	}

	//get voltage info by type
	pvoltage_info = &(voltages_table[type]);

	//check voltage region valid or not
	//min1_mV < max1_mV <= min2_mV <= max2_mV
	if ((voltage < pvoltage_info->min1_mV) ||
		(voltage > pvoltage_info->max2_mV))
	{
		WRN("invalid voltage value[%u] to set, type[%u], the reasonable range[%u-%u]\n",
			voltage, type, pvoltage_info->min1_mV, pvoltage_info->max2_mV);
		return -EINVAL;
	}

	if (voltage <= pvoltage_info->max1_mV)
	{
		//calc step value
		step = ((voltage - pvoltage_info->min1_mV + pvoltage_info->step1_mV - 1) / (pvoltage_info->step1_mV)) & 0x7f;
	}
	else if ((voltage > pvoltage_info->max1_mV) && (voltage <= pvoltage_info->min2_mV))
	{
		step = pvoltage_info->step1_num & 0x7f;
	}
	else
	{
		//calc step value
		step = ((voltage - pvoltage_info->min2_mV + pvoltage_info->step2_mV - 1) / (pvoltage_info->step2_mV) + pvoltage_info->step1_num) & 0x7f;
	}

	//write voltage control register
	devaddr = pvoltage_info->devaddr;
	regaddr = pvoltage_info->regaddr;
	step &= pvoltage_info->mask;
	ret = pmu_reg_write(&devaddr, &regaddr, &step, 1);

	if (pmu_onoff_reg_bitmap[type].dvm_st)
	{
		//delay 100us for pmu hardware update
		time_udelay(100);
		while(pmu_output_is_stable(type) == FALSE);
	}

	return ret;
}

s32 pmu_get_voltage(u32 type)
{
	u8 devaddr;
	u8 regaddr;
	u8 step = 0;
	voltage_info_t *pvoltage_info;

	if (type >= AW1660_POWER_MAX)
	{
		//invalid power voltage type
		WRN("invalid power voltage type[%u]\n", type);
		return -EINVAL;
	}

	//get voltage info by type
	pvoltage_info = &(voltages_table[type]);

	//read voltage control register
	devaddr = pvoltage_info->devaddr;
	regaddr = pvoltage_info->regaddr;
	pmu_reg_read(&devaddr, &regaddr, &step, 1);

	step &= pvoltage_info->mask;

	if (step <= (pvoltage_info->step1_num - 1))
	{
		return ((step * (pvoltage_info->step1_mV)) + pvoltage_info->min1_mV);
	}
	else if (step == pvoltage_info->step1_num)
	{
		return pvoltage_info->min2_mV;
	}
	else
	{
		return (((step - pvoltage_info->step1_num) * (pvoltage_info->step2_mV)) + pvoltage_info->min2_mV);
	}
}

s32 pmu_set_voltage_state(u32 type, u32 state)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;
	u32 offset;

	devaddr = pmu_onoff_reg_bitmap[type].devaddr;
	regaddr = pmu_onoff_reg_bitmap[type].regaddr;
	offset  = pmu_onoff_reg_bitmap[type].offset;
	pmu_onoff_reg_bitmap[type].state = state;

	//read-modify-write
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ( ~(1 << offset));
	data |= (state << offset);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	if (state == POWER_VOL_ON)
	{
		//delay 1ms for open PMU output
		time_mdelay(1);
	}

	return OK;
}

s32 pmu_get_voltage_state(u32 type)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;
	u32 offset;

	devaddr = pmu_onoff_reg_bitmap[type].devaddr;
	regaddr = pmu_onoff_reg_bitmap[type].regaddr;
	offset  = pmu_onoff_reg_bitmap[type].offset;

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	if (data & (1 << offset))
	{
		pmu_onoff_reg_bitmap[type].state = POWER_VOL_ON;
		return POWER_VOL_ON;
	}
	else
	{
		pmu_onoff_reg_bitmap[type].state = POWER_VOL_OFF;
		return POWER_VOL_OFF;
	}
}
#endif /* PMU_CONFIG_VOL_USED */

s32 pmu_set_mode(u32 type, regulator_mode_e mode)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;
	u32 offset;

	devaddr = voltages_table[type].devaddr;
	regaddr = voltages_table[type].mode_reg;
	offset  = voltages_table[type].mode_offset;

	//read-modify-write
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	data &= ( ~(1 << offset));
	data |= (mode << offset);
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	return OK;
}

s32 pmu_reg_write(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	ASSERT(len <= AXP_TRANS_BYTE_MAX);

#if RSB_USED
	u8 i;
	s32 result = OK;
	u32 data_temp = 0;

	for (i = 0; i < len; i++)
	{
		data_temp = *(data + i);
		result |= rsb_write(devaddr[i], regaddr[i], &data_temp, RSB_DATA_TYPE_BYTE);
	}
	return result;
#else
	return twi_write(regaddr, data, 1);
#endif  //RSB_USED
}

s32 pmu_reg_read(u8 *devaddr, u8 *regaddr, u8 *data, u32 len)
{
	ASSERT(len <= AXP_TRANS_BYTE_MAX);

#if RSB_USED
	//read register value by rsb
	u8 i;
	s32 result = OK;
	u32 data_temp = 0;

	for (i = 0; i < len; i++)
	{
		result |= rsb_read(devaddr[i], regaddr[i], &data_temp, RSB_DATA_TYPE_BYTE);
		*(data + i) = (u8)(data_temp & 0xff);
	}
	return result;
#else
	return twi_read(regaddr, data, len);
#endif  //RSB_USED
}

#if ((defined KERNEL_USED) || (defined TF_USED))
int get_nmi_int_type(void)
{
	u32 devaddr;
	u8 regaddr[6];
	u32 enable[6];
	u32 status[6];
	u32 nmi_int_type = 0;
	u32 len = 6;
	u8 i;

	//==================================check pmu interrupt===============================
	devaddr = RSB_RTSADDR_AW1660;
	//read enable regs
	regaddr[0] = AW1660_IRQ_ENABLE1;
	regaddr[1] = AW1660_IRQ_ENABLE2;
	regaddr[2] = AW1660_IRQ_ENABLE3;
	regaddr[3] = AW1660_IRQ_ENABLE4;
	regaddr[4] = AW1660_IRQ_ENABLE5;
	regaddr[5] = AW1660_IRQ_ENABLE6;
	for (i = 0; i < len; i++)
	{
		rsb_read(devaddr, regaddr[i], &enable[i], RSB_DATA_TYPE_BYTE);
		//LOG("%x:%x\n", regaddr[i], enable[i]);
	}

	//read status regs
	regaddr[0] = AW1660_IRQ_STATUS1;
	regaddr[1] = AW1660_IRQ_STATUS2;
	regaddr[2] = AW1660_IRQ_STATUS3;
	regaddr[3] = AW1660_IRQ_STATUS4;
	regaddr[4] = AW1660_IRQ_STATUS5;
	regaddr[5] = AW1660_IRQ_STATUS6;
	for (i = 0; i < len; i++)
	{
		rsb_read(devaddr, regaddr[i], &status[i], RSB_DATA_TYPE_BYTE);
		//LOG("%x:%x\n", regaddr[i], status[i]);
	}

	i = 0;
	while ((i < len) && (!(enable[i] & status[i])))
	{
		i++;
	}

	if (i != len)
	{
		nmi_int_type |= NMI_INT_TYPE_PMU_OFFSET;
	}

#if (defined CONFIG_ARCH_SUN50IW3P1)
	if (rsb_audio_used){ /* ac100 maybe not used in some proposals */
		/* check rtc interrupt */
		devaddr = RSB_RTSADDR_AW1653;
#if TWI_CODEC_USED /* twi interface used on rtc */
		regaddr[0] = RTC_ALARM_INT_EN_REG;
		twi_read(devaddr, regaddr[0], (u8 *)enable, RSB_DATA_TYPE_HWORD);
		regaddr[0] = RTC_ALARM_INT_ST_REG;
		twi_read(devaddr, regaddr[0], (u8 *)status, RSB_DATA_TYPE_HWORD);
#else /* rsb interface used on rtc */
		/* read enable regs */
		regaddr[0] = RTC_ALARM_INT_EN_REG;
		rsb_read(devaddr, regaddr[0], &enable[0], RSB_DATA_TYPE_HWORD);
		//LOG("%x:%x\n", regaddr[0], enable[0]);
		regaddr[0] = RTC_ALARM_INT_ST_REG;
		rsb_read(devaddr, regaddr[0], &status[0], RSB_DATA_TYPE_HWORD);
		//LOG("%x:%x\n", regaddr[0], status[0]);
#endif
		if (enable[0] & status[0])
		{
			nmi_int_type |= NMI_INT_TYPE_RTC_OFFSET;
		}
	}
#endif

	return nmi_int_type;
}
#endif

int nmi_int_handler(void *parg)
{
#if (defined KERNEL_USED)
	struct message *pmessage;
	u32 nmi_int_type = 0;
#else
	u32 event;
#endif

	//pmu interrupt coming,
	//notify ac327 pmu driver to handler
	LOG("NMI irq coming...\n");

#if (defined KERNEL_USED)
	nmi_int_type = get_nmi_int_type();

	if (nmi_int_type == 0)
	{
		WRN("invalid nmi int type\n");
		//clear interrupt flag first
		interrupt_clear_pending(EXT_NMI_IRQn);

		return FALSE;
	}

	pmessage = message_allocate();
	if (pmessage == NULL)
	{
		WRN("allocate message for nmi int notify failed\n");
		return FALSE;
	}

	//initialize message
	pmessage->type     = AXP_INT_COMING_NOTIFY;
	pmessage->private  = 0;
	pmessage->attr     = 0;
	pmessage->state    = MESSAGE_INITIALIZED;
	pmessage->paras[0] = nmi_int_type;
	hwmsgbox_send_message(pmessage, SEND_MSG_TIMEOUT);

	//disable axp interrupt
	interrupt_disable(EXT_NMI_IRQn);
#else /* !(defined KERNEL_USED)*/
	/* NOTE: if pmu interrupt enabled,
	 * means allow power key to power on system
	 */
	pmu_query_event(&event);
	if (event & (CPUS_WAKEUP_DESCEND | CPUS_WAKEUP_ASCEND | \
	    CPUS_WAKEUP_SHORT_KEY | CPUS_WAKEUP_LONG_KEY)) {
		//LOG("power key:%x\n", event);
		write_rtc_domain_reg(START_OS_REG, 0x0f);
		//LOG("reset system now\n");
		pmu_reset_system();
	}
	pmu_clear_pendings();
#endif

	//clear interrupt flag first
	interrupt_clear_pending(EXT_NMI_IRQn);

	return TRUE;
}

#if STANDBY_USED
#if 0
//backup pmu interrupt enable registers
static u8 int_enable_regs[6];

s32 pmu_standby_init(u32 event, u32 gpio_bitmap)
{
	u8  devaddr[6];
	u8  regaddr[6];
	u8  enable[6];
	u32 len = 6;

	devaddr[0] = RSB_RTSADDR_AW1660;
	devaddr[1] = RSB_RTSADDR_AW1660;
	devaddr[2] = RSB_RTSADDR_AW1660;
	devaddr[3] = RSB_RTSADDR_AW1660;
	devaddr[4] = RSB_RTSADDR_AW1660;
	devaddr[5] = RSB_RTSADDR_AW1660;
	//read pmu irq enable registers
	regaddr[0] = AW1660_IRQ_ENABLE1;
	regaddr[1] = AW1660_IRQ_ENABLE2;
	regaddr[2] = AW1660_IRQ_ENABLE3;
	regaddr[3] = AW1660_IRQ_ENABLE4;
	regaddr[4] = AW1660_IRQ_ENABLE5;
	regaddr[5] = AW1660_IRQ_ENABLE6;
	pmu_reg_read(devaddr, regaddr, enable, len);

	//backup enable registers
	memcpy(int_enable_regs, enable, len);

	//clear enable array firstly
	memset(enable, 0, len);

	//initialize pmu wakeup events
	if (event & CPUS_WAKEUP_USB)
	{
		//AW1660_IRQ_ENABLE1,
		//BIT2 : VBUS from high to low (USB plugin).
		//BIT3 : VBUS from low to high (USB plugout).
		enable[0] |= (0x1 << 2);
		enable[0] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_AC)
	{
		//AW1660_IRQ_ENABLE1,
		//BIT5 : ACIN from high to low (ACIN plugin).
		//BIT6 : ACIN from low to high (ACIN plugout).
		enable[0] |= (0x1 << 5);
		enable[0] |= (0x1 << 6);
	}
	if (event & CPUS_WAKEUP_FULLBATT)
	{
		//AW1660_IRQ_ENABLE2,
		//BIT2 : battary is full/charge done.
		enable[1] |= (0x1 << 2);
	}
	if (event & CPUS_WAKEUP_BAT_TEMP)
	{
		//AW1660_IRQ_ENABLE3,
		//BIT1 : battery temperature under the safe range IRQ.
		//BIT3 : battery temperature over the safe range IRQ.
		enable[2] |= (0x1 << 1);
		enable[2] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_LOWBATT)
	{
		//AW1660_IRQ_ENABLE4,
		//BIT0 : level2 low battary,
		//BIT1 : level1 low battary.
		enable[3] |= (0x1 << 0);
		enable[3] |= (0x1 << 1);
	}
	if (gpio_bitmap & CPUS_GPIO_AXP(0))
	{
		//AW1660_IRQ_ENABLE5,
		//BIT0 : GPIO0 input edge.
		enable[4] |= (0x1 << 0);
	}
	if (gpio_bitmap & CPUS_GPIO_AXP(1))
	{
		//AW1660_IRQ_ENABLE5,
		//BIT1 : GPIO1 input edge.
		enable[4] |= (0x1 << 1);
	}
	if (event & CPUS_WAKEUP_LONG_KEY)
	{
		//AW1660_IRQ_ENABLE5,
		//BIT3 : long key.
		enable[4] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_SHORT_KEY)
	{
		//AW1660_IRQ_ENABLE5,
		//BIT4 : short key.
		enable[4] |= (0x1 << 4);
	}
	if (event & CPUS_WAKEUP_DESCEND)
	{
		//AW1660_IRQ_ENABLE5,
		//BIT5 : POK negative.
		enable[4] |= (0x1 << 5);
	}
	if (event & CPUS_WAKEUP_ASCEND)
	{
		//AW1660_IRQ_ENABLE5,
		//BIT6 : POK postive.
		enable[4] |= (0x1 << 6);
	}

	//write pmu enable registers
	pmu_reg_write(devaddr, regaddr, enable, len);

	return OK;
}

s32 pmu_standby_exit(void)
{
	u8 devaddr[6];
	u8 regaddr[6];
	u32 len = 6;

	//restore pmu irq enable registers
	devaddr[0] = RSB_RTSADDR_AW1660;
	devaddr[1] = RSB_RTSADDR_AW1660;
	devaddr[2] = RSB_RTSADDR_AW1660;
	devaddr[3] = RSB_RTSADDR_AW1660;
	devaddr[4] = RSB_RTSADDR_AW1660;
	devaddr[5] = RSB_RTSADDR_AW1660;
	regaddr[0] = AW1660_IRQ_ENABLE1;
	regaddr[1] = AW1660_IRQ_ENABLE2;
	regaddr[2] = AW1660_IRQ_ENABLE3;
	regaddr[3] = AW1660_IRQ_ENABLE4;
	regaddr[4] = AW1660_IRQ_ENABLE5;
	regaddr[5] = AW1660_IRQ_ENABLE6;
	pmu_reg_write(devaddr, regaddr, int_enable_regs, len);

	return OK;
}
#endif
s32 pmu_reg_write_para(pmu_paras_t *para)
{
	return pmu_reg_write(para->devaddr, para->regaddr, para->data, para->len);
}

s32 pmu_reg_read_para(pmu_paras_t *para)
{
	return pmu_reg_read(para->devaddr, para->regaddr, para->data, para->len);
}
#endif

s32 pmu_query_event(u32 *event)
{
	u8 devaddr[6];
	u8 regaddr[6];
	u8 status[6];
	u32 len = 6;

	//read pmu irq enable registers
	devaddr[0] = RSB_RTSADDR_AW1660;
	devaddr[1] = RSB_RTSADDR_AW1660;
	devaddr[2] = RSB_RTSADDR_AW1660;
	devaddr[3] = RSB_RTSADDR_AW1660;
	devaddr[4] = RSB_RTSADDR_AW1660;
	devaddr[5] = RSB_RTSADDR_AW1660;
	regaddr[0] = AW1660_IRQ_STATUS1;
	regaddr[1] = AW1660_IRQ_STATUS2;
	regaddr[2] = AW1660_IRQ_STATUS3;
	regaddr[3] = AW1660_IRQ_STATUS4;
	regaddr[4] = AW1660_IRQ_STATUS5;
	regaddr[5] = AW1660_IRQ_STATUS6;
	pmu_reg_read(devaddr, regaddr, status, len);

	//query pmu wakeup events
	INF("query pmu wakeup event\n");
	if ((status[0] & (0x1 << 2)) || (status[0] & (0x1 << 3)))
	{
		//AW1660_IRQ_STATUS1,
		//BIT2 : VBUS from high to low (USB plugin).
		//BIT3 : VBUS from low to high (USB plugout).
		INF("vbus\n");
		*event |= CPUS_WAKEUP_USB;
	}
	if ((status[0] & (0x1 << 5)) || (status[0] & (0x1 << 6)))
	{
		//AW1660_IRQ_STATUS1,
		//BIT5 : ACIN from high to low (ACIN plugin).
		//BIT6 : ACIN from low to high (ACIN plugout).
		INF("acin\n");
		*event |= CPUS_WAKEUP_AC;
	}
	if (status[1] & (0x1 << 2))
	{
		//AW1660_IRQ_STATUS2,
		//BIT2 : battary is full/charge done.
		INF("battary full\n");
		*event |= CPUS_WAKEUP_FULLBATT;
	}
	if ((status[2] & (0x1 << 1)) || (status[2] & (0x1 << 3)))
	{
		//AW1660_IRQ_STATUS3,
		//BIT1 : battery temperature under the safe range IRQ.
		//BIT3 : battery temperature over the safe range IRQ.
		INF("bat temp\n");
		*event |= CPUS_WAKEUP_BAT_TEMP;
	}
	if ((status[3] & (0x1 << 0)) || (status[3] & (0x1 << 1)))
	{
		//AW1660_IRQ_STATUS4,
		//BIT0 : level2 low battary,
		//BIT1 : level1 low battary.
		INF("low battary\n");
		*event |= CPUS_WAKEUP_LOWBATT;
	}
	if (status[4] & (0x1 << 0))
	{
		//AW1660_IRQ_STATUS5,
		//BIT0 : GPIO0 input edge.
		INF("GPIO0 input edge\n");
		*event |= CPUS_WAKEUP_GPIO;
	}
	if (status[4] & (0x1 << 1))
	{
		//AW1660_IRQ_STATUS5,
		//BIT1 : GPIO1 input edge.
		INF("GPIO1 input edge\n");
		*event |= CPUS_WAKEUP_GPIO;
	}
	if (status[4] & (0x1 << 3))
	{
		//AW1660_IRQ_STATUS5,
		//BIT3 : long key.
		INF("long key\n");
		*event |= CPUS_WAKEUP_LONG_KEY;
	}
	if (status[4] & (0x1 << 4))
	{
		//AW1660_IRQ_STATUS5,
		//BIT4 : short key.
		INF("short key\n");
		*event |= CPUS_WAKEUP_SHORT_KEY;
	}
	if (status[4] & (0x1 << 5))
	{
		//AW1660_IRQ_STATUS5,
		//BIT5 : POK negative.
		INF("POK negative\n");
		*event |= CPUS_WAKEUP_DESCEND;
	}
	if (status[4] & (0x1 << 6))
	{
		//AW1660_IRQ_STATUS5,
		//BIT6 : POK postive.
		INF("POK postive\n");
		*event |= CPUS_WAKEUP_ASCEND;
	}

	INF("event:%x\n", *event);

	return OK;
}

s32 pmu_clear_pendings(void)
{
	u32 i;
	u8  devaddr[6];
	u8  regaddr[6];
	u8  data[6];
	u32 len = 6;

	for (i = 0; i < len; i++)
	{
		devaddr[i] = RSB_RTSADDR_AW1660;
		regaddr[i] = 0x48 + i;
		data[i] = 0xff;
	}
	return pmu_reg_write(devaddr, regaddr, data, len);
}

#if ((defined KERNEL_USED) || (defined TF_USED))
s32 pmu_get_chip_id(struct message *pmessage)
{
		u32 i = 0;
		u8 regaddr[23];
		u8 data_u8[23];
		u32 data_u32[23];
		u64 data_u64[2];
		u8 data[16];
		u32 singledata;
		s32 result = OK;

		//set pmu reg 0xff to 1
		singledata = 1;
		rsb_write(RSB_RTSADDR_AW1660, 0xff, &singledata, RSB_DATA_TYPE_BYTE);

		//init pmu regaddr
		for (i = 0; i < 23; i++)
		{
			regaddr[i] = 0x20 + i;
		}

		//read data one by one
		for (i = 0; i < 23; i++)
		{
			result |= rsb_read(RSB_RTSADDR_AW1660, regaddr[i], &(data_u32[i]), RSB_DATA_TYPE_BYTE);
			data_u8[i] = (u8)data_u32[i];
		}

		/* convert 256bit to 128bit */
		data[15] = data_u8[0];
		data[14] = data_u8[1];
		data[13] = data_u8[2];
		data[12] = data_u8[3];
		data[11] = data_u8[4];

		/* |6|6|6|2|8|6|6|6|6|6|6| */
		data_u64[0] = (u64)(data_u8[5] & 0x3f);
		data_u64[0] |= (u64)(data_u8[6] & 0x3f) << 6;
		data_u64[0] |= (u64)(data_u8[7] & 0x3f) << 12;
		data_u64[0] |= (u64)(data_u8[8] & 0x3f) << 18;
		data_u64[0] |= (u64)(data_u8[9] & 0x3f) << 24;
		data_u64[0] |= (u64)(data_u8[10] & 0x3f) << 30;
		data_u64[0] |= (u64)(data_u8[11] & 0xff) << 36;
		data_u64[0] |= (u64)(data_u8[12] & 0x3) << 44;
		data_u64[0] |= (u64)(data_u8[16] & 0x3f) << 46;
		data_u64[0] |= (u64)(data_u8[17] & 0x3f) << 52;
		data_u64[0] |= (u64)(data_u8[18] & 0x3f) << 58;

		/* |6|6|6|6| */
		data_u64[1] = (u64)(data_u8[19] & 0x3f);
		data_u64[1] |= (u64)(data_u8[20] & 0x3f) << 6;
		data_u64[1] |= (u64)(data_u8[21] & 0x3f) << 12;
		data_u64[1] |= (u64)(data_u8[22] & 0x3f) << 18;

		data[10] = data_u64[0] & 0xff;
		data[9] = (data_u64[0] >> 8) & 0xff;
		data[8] = (data_u64[0] >> 16) & 0xff;
		data[7] = (data_u64[0] >> 24) & 0xff;
		data[6] = (data_u64[0] >> 32) & 0xff;
		data[5] = (data_u64[0] >> 40) & 0xff;
		data[4] = (data_u64[0] >> 48) & 0xff;
		data[3] = (data_u64[0] >> 56) & 0xff;

		data[2] = data_u64[1] & 0xff;
		data[1] = (data_u64[1] >> 8) & 0xff;
		data[0] = (data_u64[1] >> 16) & 0xff;

		//copy readout data to pmessage->paras
		for (i = 0; i < 4; i++)
		{
			/* |paras[0]    |paras[1]    |paras[2]     |paras[3]      |
			 * |chip_id[0~3]|chip_id[4~7]|chip_id[8~11]|chip_id[12~15]|
			 */
			pmessage->paras[0] |= (data[i] << (i * 8));
			pmessage->paras[1] |= (data[4 + i] << (i * 8));
			pmessage->paras[2] |= (data[8 + i] << (i * 8));
			pmessage->paras[3] |= (data[12 + i] << (i * 8));
		}

		//set pmu reg 0xff to 0
		singledata = 0;
		rsb_write(RSB_RTSADDR_AW1660, 0xff, &singledata, RSB_DATA_TYPE_BYTE);

		return result;
}

s32 pmu_set_paras(struct message *pmessage)
{
#if (defined CONFIG_ARCH_SUN50IW3P1)
	if (pmessage->paras[2] == 1) {
		bat_charge_temp = pmessage->paras[0];
		pmu_max_chg_cur = pmessage->paras[1];
		//LOG("cfg pmu charge temp:%d, max_cur:%d\n", bat_charge_temp, pmu_max_chg_cur);

		return OK;
	}
#endif
	pmu_discharge_ltf = pmessage->paras[0] * 10;
	pmu_discharge_htf = pmessage->paras[1] * 10;
	//LOG("cfg pmu temp safe range:0x%x ~ 0x%x\n", pmu_discharge_ltf, pmu_discharge_htf);

	return OK;
}

s32 set_pwr_tree(struct message *pmessage)
{
	memcpy((void *)arisc_para.power_regu_tree, (const void *)pmessage->paras, sizeof(arisc_para.power_regu_tree));

	//hexdump("tree", (char *)arisc_para.power_regu_tree, sizeof(arisc_para.power_regu_tree));

	return OK;
}

#define axp22_vbat_to_mV(reg)           ((int)(((reg >> 8) << 4 ) | (reg & 0x000F)) * 11 / 10)
#define axp22_icharge_to_mA(reg)        ((int)(((reg >> 8) << 4 ) | (reg & 0x000F)))
#define axp22_ibat_to_mA(reg)           ((int)(((reg >> 8) << 4 ) | (reg & 0x000F)))
#define axp22_cibat_to_mA(reg)          ((reg & 0x0f) * 150000 + 300000)
#define axp22_qbat_to_C(reg)            (reg & 0x7f)
#define axp22_reg_to_tempc(reg)         DIV_ROUND_UP(reg*1063, 10000) - DIV_ROUND_UP(2667, 10);

#if PMU_CHRCUR_CRTL_USED
#define PMU_CRTL_PRINT INF
void pmu_contrl_batchrcur(void)
{
#define MIN_CHARGE_CUR 300000
	u32 rdata[2], wdata;
	u32 real_curr_ma, quan_c, temp_c;
	u32 curr_set_ma, curr_set_reg;
	u8 reg;
	s32 cpsr;
	s8 adj = -1;

	cpsr = cpuIntsOff();
	if (bat_charge_temp) { /* 0 is not use this function */
		PMU_CRTL_PRINT("bat_charge_temp:%d\n", bat_charge_temp);

		/* check if battery enable charge function */
		rsb_read(RSB_RTSADDR_AW1660, AW1660_PWR_SRC_STA, &rdata[0], RSB_DATA_TYPE_BYTE);
		if (!(rdata[0] & 0x04)) { /* reg00 bit2, 0:discharge, 1:charging */
			cpuIntsRestore(cpsr);
			return;
		}

		/* read set charge current */
		rsb_read(RSB_RTSADDR_AW1660, AW1660_CHARGER_CTRL1, &curr_set_reg, RSB_DATA_TYPE_BYTE);
		curr_set_ma = ((curr_set_reg & 0x0f) * 200000 + 200000);
		PMU_CRTL_PRINT("curr_set_ma:%d\n", curr_set_ma);

		/* read current */
		rsb_read(RSB_RTSADDR_AW1660, AW1660_CHAR_CURR_STA1, &rdata[0], RSB_DATA_TYPE_BYTE);
		rsb_read(RSB_RTSADDR_AW1660, AW1660_CHAR_CURR_STA2, &rdata[1], RSB_DATA_TYPE_BYTE);
		real_curr_ma = (rdata[0] << 4) | (rdata[1] & 0x0f);
		real_curr_ma *= 1000;
		PMU_CRTL_PRINT("rel_curr_ma:%d\n", real_curr_ma);

		/* read quan_c */
		rsb_read(RSB_RTSADDR_AW1660, AW1660_BAT_QUANTITY, &rdata[0], RSB_DATA_TYPE_BYTE);
		quan_c = axp22_qbat_to_C(rdata[0]);
		PMU_CRTL_PRINT("quan_c:%d\n", quan_c);

		/* over 99% */
		if (quan_c > 99) {
			reg = DIV_ROUND_UP((real_curr_ma - 200000), 200000);
			wdata = (curr_set_reg & 0xf0) | (reg & 0x0f);
			rsb_write(RSB_RTSADDR_AW1660, AW1660_CHARGER_CTRL1, &wdata, RSB_DATA_TYPE_BYTE);
			cpuIntsRestore(cpsr);
			return;
		}

		/* read temp_c */
		rsb_read(RSB_RTSADDR_AW1660, AW1660_IC_TEMP_HIG, &rdata[0], RSB_DATA_TYPE_BYTE);
		rsb_read(RSB_RTSADDR_AW1660, AW1660_IC_TEMP_LOW, &rdata[1], RSB_DATA_TYPE_BYTE);
		temp_c = (rdata[0] << 4) | (rdata[1] & 0x0f);
		temp_c = axp22_reg_to_tempc(temp_c);
		PMU_CRTL_PRINT("temp_c:%d\n", temp_c);

		if (temp_c > (bat_charge_temp + 1)) {
			adj = DIV_ROUND_UP(temp_c - bat_charge_temp, 2);
		} else if (temp_c < (bat_charge_temp - 1)) {
			if (curr_set_ma < pmu_max_chg_cur)
				adj = 0;
		}

		if (adj > 0) {
			adj = (curr_set_reg & 0x0f) < adj ? (curr_set_reg & 0x0f) : adj;
			wdata = curr_set_reg - adj;
		} else if (adj == 0){
			if ((curr_set_reg & 0x0f) < 0x0f)
				wdata = curr_set_reg + 1;
			else
				wdata = curr_set_reg;
		} else
			wdata = curr_set_reg;

		PMU_CRTL_PRINT("adjust:%d\n", adj);
		PMU_CRTL_PRINT("set cur:%x\n", wdata);
		rsb_write(RSB_RTSADDR_AW1660, AW1660_CHARGER_CTRL1, &wdata, RSB_DATA_TYPE_BYTE);
	}
	cpuIntsRestore(cpsr);
}
#endif
#if SUSPEND_POWER_CHECK
s32 pmu_get_batconsum(void)
{
	u32 rdata[2];
	s32 mv, mab, mac, mw;

	rsb_read(RSB_RTSADDR_AW1660, 0x50, &rdata[0], RSB_DATA_TYPE_BYTE);
	rsb_read(RSB_RTSADDR_AW1660, 0x51, &rdata[1], RSB_DATA_TYPE_BYTE);
	rdata[1] |= (rdata[0] << 8);
	mv = axp22_vbat_to_mV(rdata[1]);
	INF("mv:%x\n", mv);

	rsb_read(RSB_RTSADDR_AW1660, 0x52, &rdata[0], RSB_DATA_TYPE_BYTE);
	rsb_read(RSB_RTSADDR_AW1660, 0x53, &rdata[1], RSB_DATA_TYPE_BYTE);
	rdata[1] |= (rdata[0] << 8);
	mac = axp22_icharge_to_mA(rdata[1]);
	INF("mac:%x\n", mac);
	rsb_read(RSB_RTSADDR_AW1660, 0x54, &rdata[0], RSB_DATA_TYPE_BYTE);
	rsb_read(RSB_RTSADDR_AW1660, 0x55, &rdata[1], RSB_DATA_TYPE_BYTE);
	rdata[1] |= (rdata[0] << 8);
	mab = axp22_ibat_to_mA(rdata[1]);
	INF("mab:%x\n", mab);

	mw = mv * (mab - mac) / 1000;

	return mw;
}

u32 pmu_get_powerstate(u32 power_reg)
{
	u32 rdata;
	u32 cur_powsta = power_reg & 0x0ffff;

	/*
	 * power_reg bit0 ~ 7 AXP_main REG10, bit 8~15 AXP_main REG12,
	 * bit 16~23 AXP_main REG13, bit 24 AXP_main REG90, bit 24 AXP_main REG92.
	 */
	/* master_axp809 0x0ff */
	rsb_read(RSB_RTSADDR_AW1660, 0x10, &rdata, RSB_DATA_TYPE_BYTE);
	rdata &= 0x7f;
	if ((power_reg & 0x7f) != rdata) {
		cur_powsta &= ~0x0ff;
		cur_powsta |= rdata;
		//LOG("power wakeup axp main reg10:%x\n", rdata);
	}
	/* master_axp809 0x0ff00 */
	rsb_read(RSB_RTSADDR_AW1660, 0x12, &rdata, RSB_DATA_TYPE_BYTE);
	rdata <<= 8;
	if ((power_reg & 0xff00) != rdata) {
		cur_powsta &= ~0xff00;
		cur_powsta |= rdata;
		//LOG("power wakeup axp main reg12:%x\n", rdata);
	}
	/* master_axp809 0x010000 */
	rsb_read(RSB_RTSADDR_AW1660, 0x13, &rdata, RSB_DATA_TYPE_BYTE);
	rdata &= 0xfc;
	rdata <<= 16;
	if ((power_reg & 0xfc0000) != rdata) {
		cur_powsta &= ~0xff0000;
		cur_powsta |= rdata;
		//LOG("power wakeup axp main reg13:%x\n", rdata);
	}
	/* master_axp809 0x020000 */
	rsb_read(RSB_RTSADDR_AW1660, 0x90, &rdata, RSB_DATA_TYPE_BYTE);
	if ((rdata & 0x7) == 0x3)
		rdata = 0x1;
	else
		rdata = 0;
	rdata <<= 24;
	if ((power_reg & 0x1000000) != rdata) {
		cur_powsta &= ~0x1000000;
		cur_powsta |= rdata;
		//LOG("power wakeup axp main reg90:%x\n", rdata);
	}
	/* master_axp809 0x040000 */
	rsb_read(RSB_RTSADDR_AW1660, 0x92, &rdata, RSB_DATA_TYPE_BYTE);
	if ((rdata & 0x7) == 0x3)
		rdata = 0x1;
	else
		rdata = 0;
	rdata <<= 25;
	if ((power_reg & 0x2000000) != rdata) {
		cur_powsta &= ~0x2000000;
		cur_powsta |= rdata;
		//LOG("power wakeup axp main reg92:%x\n", rdata);
	}

	return cur_powsta;
}
#endif
#endif /* ((defined KERNEL_USED) || (defined TF_USED)) */

#if TF_USED
void pmu_sysconfig_cfg(void)
{
	//LOG("pwkey_used:%d\n", arisc_para.start_os.pmukey_used);
	if (arisc_para.start_os.pmukey_used) { /* physical key used */
		interrupt_enable(EXT_NMI_IRQn);
	}
}

void pmu_reset_system(void)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;

#if HOMELET_USED
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_PWR_WAKEUP_CTRL;
#else
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_PWR_WAKEUP_CTRL;
#endif
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("pmu reset reg:%x\n", data);
	data |= 1U << 6;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	printk("reset system\n");
	while(1);
}

void pmu_poweroff_system(void)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;

#if HOMELET_USED
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_PWR_OFF_CTRL;
#else
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_PWR_OFF_CTRL;
#endif
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("pmu pwroff reg:%x\n", data);
	data |= 1U << 7;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	printk("poweroff system\n");
	while(1);
}

/* set power key time, support 0~3
 * 0:128mS, 1:1S, 2:2S, 3:3S
 */
s32 pmu_set_pok_time(int time)
{
	u8 devaddr;
	u8 regaddr;
	u8 data;

#if HOMELET_USED
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_POK_SET;
#else
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_POK_SET;
#endif

	if (time > 4)
		return -EINVAL;

	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("pmu pwrkey reg:%x\n", data);
	data &= ~(3U << 6);
	data |= time << 6;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	return time;
}

void pmu_set_lowpcons(void)
{

	u8 devaddr;
	u8 regaddr;
	u8 data;

	//LOG("pmu enter low pow cons\n");
	hwspinlock_super_standby_init();

	/* set pll_audio&pll_periph&other plls power off gating valid */
	ccu_set_poweroff_gating_state(PWRCTL_VDD_CPUS, CCU_POWEROFF_GATING_VALID);
	ccu_set_poweroff_gating_state(PWRCTL_VDD_AVCC_A, CCU_POWEROFF_GATING_VALID);
	time_mdelay(1);

	/* set VDD_SYS reset as assert state */
	ccu_set_mclk_reset(CCU_MOD_CLK_VDD_SYS, CCU_CLK_RESET);

	/* bit define, see standby.h */
	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_OUTPUT_PWR_CTRL1;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("%x ", data);
	data = 0x0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	//LOG("%x reg12 poweroff\n", data);

	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_ALDO123_OP_MODE;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("%x ", data);
	data = 0x48; /* ALDO2(PL), FLDO2(cpus) */
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	//LOG("%x reg13 poweroff\n", data);

	devaddr = RSB_RTSADDR_AW1660;
	regaddr = AW1660_OUTPUT_PWR_CTRL0;
	data = 0x0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);
	//LOG("%x reg10 poweroff\n", data);
}

int pmu_set_gpio(unsigned int id, unsigned int state)
{

	u8 devaddr;
	u8 regaddr;
	u8 data;

	if ((id > 1) || (state > 1))
		return -EINVAL;

	devaddr = RSB_RTSADDR_AW1660;
	if (id)
		regaddr = AW1660_GPIOLDO1_CTRL;
	else
		regaddr = AW1660_GPIOLDO0_CTRL;
	pmu_reg_read(&devaddr, &regaddr, &data, 1);
	//LOG("pmu gpios:%x\n", data);
	data &= ~0x07;
	if (state)
		data |= 1; /* ldo on */
	else
		data |= 0;
	pmu_reg_write(&devaddr, &regaddr, &data, 1);

	return OK;
}

#endif
#endif /* PMU_USED */
#endif // sun50iw3
