/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                DVFS module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dvfs.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-14
* Descript: DVFS module.
* Update  : date                auther      ver     notes
*           2012-5-14 8:58:37   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "dvfs_i.h"

#if DVFS_USED
#if (defined CONFIG_ARCH_SUN50IW3P1)

//record the current freq and voltage
struct freq_voltage current_vf;
struct freq_voltage *vf_table = NULL;
#if 0
struct freq_voltage vf_table[] =
{
	//freq          //voltage   //axi_div
	{1728000000,    1060,       3}, //cpu0 vdd is 1.20v if cpu freq is (600Mhz, 1008Mhz]
	{1632000000,    1000,       3}, //cpu0 vdd is 1.20v if cpu freq is (420Mhz, 600Mhz]
	{1536000000,    960,        3}, //cpu0 vdd is 1.20v if cpu freq is (360Mhz, 420Mhz]
	{1440000000,    900,        3}, //cpu0 vdd is 1.20v if cpu freq is (300Mhz, 360Mhz]
	{1080000000,    800,        3}, //cpu0 vdd is 1.20v if cpu freq is (240Mhz, 300Mhz]
	{888000000,     760,        3}, //cpu0 vdd is 1.20v if cpu freq is (120Mhz, 240Mhz]
	{0,             760,        3}, //cpu0 vdd is 1.20v if cpu freq is (60Mhz,  120Mhz]
	{0,             760,        3}, //cpu0 vdd is 1.20v if cpu freq is (0Mhz,   60Mhz]
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}, //end of cpu dvfs table
	{0,             760,        3}  //end of cpu dvfs table
};
#endif

/*
*********************************************************************************************************
*                                       INIT DVFS
*
* Description:  initialize dvfs module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_init(void)
{
	//initialize current freq.
	current_vf.freq = ccu_get_sclk_freq(CCU_SYS_CLK_PLL1);

	//SOC->CPU0_VDD = PMU->DCDC3.
	current_vf.voltage = pmu_get_voltage(AW1660_POWER_DCDC2);

	//initialize axi bus divide ratio
	current_vf.axi_div = ccu_get_mclk_div(CCU_MOD_CLK_AXI);
	vf_table = arisc_para.vf;

	INF("freq   :%d\n", current_vf.freq);
	INF("voltage:%d\n", current_vf.voltage);
	INF("axi_div:%d\n", current_vf.axi_div);

	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT DVFS
*
* Description:  exit dvfs module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_exit(void)
{
	return OK;
}

/*
*********************************************************************************************************
*                                       SET FREQ
*
* Description:  set the frequency of a specical module.
*
* Arguments  :  freq    : the frequency which we want to set.
*
* Returns    :  OK if set frequency succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_set_freq(struct message *pmessage)
{
	struct freq_voltage *vf_inf;
	u32 current_voltage = 0;

	/* initialize message
	 *
	 * |paras[0]|
	 * |freq    |
	 */
	u32 freq = pmessage->paras[0];

	//freq base on khz
	freq = freq * 1000;

	//find target voltage
	vf_inf = &vf_table[0];
	while((vf_inf + 1)->freq >= freq)
	{
		vf_inf++;
	}

	//try to increase voltage first.
	if (vf_inf->voltage > current_vf.voltage)
	{
		//calc adjust voltage steps, DCDC3 voltage step is 20mV
		u32 steps = (vf_inf->voltage - current_vf.voltage) / 20;
		u32 mdelay;

		//SOC->CPU0_VDD = PMU->DCDC3.
		if (pmu_set_voltage(AW1660_POWER_DCDC2, vf_inf->voltage) != OK)
		{
			//dvfs failed, feedback the failed result to cpu0.
			ERR("dvfs try to increase voltage failed\n");
			return -EFAIL;
		}

		mdelay = ((steps * PMU_DCDC3_STEP_DELAY_US) + 1000 - 1) / 1000;
		time_mdelay(mdelay);

		//update current voltage
		current_voltage = pmu_get_voltage(AW1660_POWER_DCDC2);
		if (current_voltage != vf_inf->voltage)
		{
			//if current voltage != target voltage
			//just return FAIL, and not continue scaling voltage
			ERR("current voltage != target voltage\n");
			return -EFAIL;
		}
	}

	//try to adjust CPU0 target freq
	if (freq != current_vf.freq)
	{
		//try to increase axi divide ratio
		if (vf_inf->axi_div > current_vf.axi_div)
		{
			//the axi_div of target freq should be increase first,
			//this is to avoid axi bus clock beyond limition.
			ccu_set_mclk_div(CCU_MOD_CLK_AXI, vf_inf->axi_div);
			current_vf.axi_div = ccu_get_mclk_div(CCU_MOD_CLK_AXI);
		}

		//adjust PLL1 clock, this interface internal will wait PLL1 stable
		ccu_set_sclk_freq(CCU_SYS_CLK_PLL1, freq);
		current_vf.freq = ccu_get_sclk_freq(CCU_SYS_CLK_PLL1);

		//try to decrease axi divide ratio
		if (vf_inf->axi_div < current_vf.axi_div)
		{
			//the axi_div of target freq should be increase first,
			//this is to avoid axi bus clock beyond limition.
			ccu_set_mclk_div(CCU_MOD_CLK_AXI, vf_inf->axi_div);
			current_vf.axi_div = ccu_get_mclk_div(CCU_MOD_CLK_AXI);
		}
	}

	//try to decrease CPU0_VDD
	if (vf_inf->voltage < current_vf.voltage)
	{
		//SOC->CPU0_VDD = PMU->DCDC3.
		if (pmu_set_voltage(AW1660_POWER_DCDC2, vf_inf->voltage) != OK)
		{
			//dvfs failed, feedback the failed result to cpu0.
			ERR("dvfs try to decrease voltage failed\n");
			return -EFAIL;
		}

		//add 1m delay to wait dvm scaling voltage
		time_mdelay(1);

		//update current voltage
		current_voltage = pmu_get_voltage(AW1660_POWER_DCDC2);
		if (current_voltage != vf_inf->voltage)
		{
			//if current voltage != target voltage
			//just return FAIL, and not continue scaling voltage
			ERR("current voltage != target voltage\n");
			return -EFAIL;
		}
	}

	if (vf_inf->voltage != current_vf.voltage)
	{
		current_vf.voltage = current_voltage;
	}

	//dvfs succeeded, feedback the succeeded result to cpu0.
	INF("DVFS succeed, freq = %u, voltage = %u, axi_div = %u\n",
		current_vf.freq, current_vf.voltage, current_vf.axi_div);
	return OK;
}
#endif // sun50iw3
#endif
