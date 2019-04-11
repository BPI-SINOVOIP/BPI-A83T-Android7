/*******************************************************************************/
/*	Allwinner Technology, All Right Reserved. 2006-2015 Copyright (c)
 *	File:				mctl_standby.c
 *
 *	Description:  This file implements basic functions for standby test
 *
 * 	History:
 *		2017/03/28		HSD		V0.10		Initial version
 *		2017/04/27		HSD		V0.50		Sync with mctl_hal.c,
 *												add Auto SR control, default OFF
 *		2017/05/11		HSD		V0.51		improve performance in high freq
 *		2017/05/22		HSD		V0.52		update master priority
 *		2017/07/06		HSD		V0.53		update master (CPU, GPU, VE0, VE1, DE)priority
 *		2017/07/06		LHK		V0.54		update priority for pad
 *		2017/07/21		HSD		V0.54.1		fix mistake of V0.54 in set_master_priority_pad_standby
 *		2017/08/11		LHK		V0.55		update pad master priority
*/

/*******************************************************************************/
#include <plat/inc/include.h>
#if (defined CONFIG_ARCH_SUN50IW3P1)
#include "mctl_reg-sun50iw3.h"
#include "mctl_hal-sun50iw3.h"
#define printf INF
extern dram_para_t *pdram_para;
#ifndef FPGA_VERIFY

#define STB_VERSION		"V0.55"
/********************************************************************************
 *IC standby code
 ********************************************************************************/
static unsigned int reg[100] = {0};
/*****************************************************************************
Function : DRAM Master Access Enable
parameter : Void
return value : Void
*****************************************************************************/
void dram_enable_all_master(void)
{
	/* enable all master */
	mctl_write_w(0xffffffff, MC_MAER0);
	mctl_write_w(0xff, MC_MAER1);
	mctl_write_w(0xffff, MC_MAER2);
	udelay(10);
}
/*****************************************************************************
Function : DRAM Master Access Disable
parameter : Void
return value : Void
*****************************************************************************/
void dram_disable_all_master(void)
{
	/* disable all master except cpu/cpus */
	mctl_write_w(0x4, MC_MAER0);
	mctl_write_w(0x0, MC_MAER1);
	mctl_write_w(0x1, MC_MAER2);
	udelay(10);
}

void dram_master_enable_cpus(void)
{
	u32 value;

	value = mctl_read_w(MC_MAER0);
	value |= 1 << 2;
	mctl_write_w(value, MC_MAER0);

	value = mctl_read_w(MC_MAER2);
	value |= 1 << 0;
	mctl_write_w(value, MC_MAER2);
}

/*****************************************************************************
Function : DRAM Standby Function Entry
parameter : Void
return value : Void
*****************************************************************************/
static unsigned int __dram_power_save_process(void)
{
	unsigned int reg_val = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	/*Save Data Before Enter Self-Refresh*/
	reg[0] = mctl_read_w(MC_WORK_MODE);
	reg[1] = mctl_read_w(MC_R1_WORK_MODE);
	reg[2] = mctl_read_w(DRAMTMG0);
	reg[3] = mctl_read_w(DRAMTMG1);
	reg[4] = mctl_read_w(DRAMTMG2);
	reg[5] = mctl_read_w(DRAMTMG3);
	reg[6] = mctl_read_w(DRAMTMG4);
	reg[7] = mctl_read_w(DRAMTMG5);
	reg[8] = mctl_read_w(DRAMTMG8);
	reg[9] = mctl_read_w(PITMG0);
	reg[10] = mctl_read_w(PTR3);
	reg[11] = mctl_read_w(PTR4);
	reg[12] = mctl_read_w(ASRC);
	reg[13] = mctl_read_w(ASRTC);
	reg[14] = 0;

	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 11; j++)
		{
			reg[15 + j + (i * 11)] = mctl_read_w((DATX0IOCR(j) + i * 0x80));
		}
	}
	for(i = 0; i < 31; i++)
	{
		reg[60 + i] = mctl_read_w(ACIOCR1(i));
	}
	for(i = 0; i < 4; i++)
	{
		reg[95 + i] = mctl_read_w(DXnSDLR6(i));
	}
	/* disable all master access .
	 * After saving data ,disable all master access
	 */
	dram_disable_all_master();
	udelay(1);
	/* DRAM power down. */
	/*1.enter self refresh */
	reg_val = mctl_read_w(PWRCTL);
	reg_val |= 0x1 << 0;
	reg_val |= (0x1 << 8);
	mctl_write_w(reg_val, PWRCTL);
	/*confirm dram controller has enter selfrefresh */
	while (((mctl_read_w(STATR) & 0x7) != 0x3));
	udelay(1);

	/*disable DRAM CK, the ck output will be tied '0' */
	reg_val = mctl_read_w(PGCR3);
	reg_val &= (~(0xffU << 16));
	mctl_write_w(reg_val, PGCR3);
	udelay(1);

	/* 2.disable CK and power down pad include AC/DX/ pad,
	 * ZQ calibration module power down
	 */

	for(i = 0; i < 4; i++)
	{	/*DXIO POWER DOWN */
		reg_val = mctl_read_w(DXnGCR0(i));
		reg_val &= ~(0x3U << 22);
		reg_val |= (0x1U << 22); 	/*DQS receiver off */
		reg_val &= ~(0xfU << 12);
		reg_val |= (0x5U << 12); 	/*POWER DOWN RECEIVER/DRICER OFF */
		reg_val |= (0x2U << 2); 	/*OE mode disable */
		reg_val |= (0x1 << 1); 		/*IO CMOS mode */
		reg_val &= ~(0x1U << 0); 	/*DQ GROUP OFF */
		mctl_write_w(reg_val, DXnGCR0(i));
	}
	reg_val = mctl_read_w(ACIOCR0);	/*CA IO POWER DOWN */
	reg_val |= (0x3U << 8);			/*CKE ENABLE */
	reg_val &= ~(0x3U << 6);		/*CK OUTPUT DISABLE */
	reg_val &= ~(0x1U << 3);		/*CA OE OFF */
	reg_val |= (0x1 << 4);			/*IO CMOS mode */
	reg_val |= (0x3U << 0);			/*CA POWER DOWN RECEIVER/DRIVER ON */
	mctl_write_w(reg_val, ACIOCR0);

	/* 3.pad hold */
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING_REG);
	reg_val |= 0x3 << 0;
	mctl_write_w(reg_val, VDD_SYS_PWROFF_GATING_REG);
	udelay(10);

	/* 4.disable global clk and pll-ddr0/pll-ddr1 */
	/* disable DRAM CLK */
	mctl_write_w(0x0, CLKEN);
	/* close mbus gate */
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/* mbus reset */
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val &=~(1U<<30);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/* close DRAM AHB gate */
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val &= ~(1U<<0);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/* DRAM AHB reset */
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val &= ~(1U<<16);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/* close PLL_DDR0 */
	reg_val = mctl_read_w(_PLL_DDR0_CTRL_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, _PLL_DDR0_CTRL_REG);
	/* close PLL_DDR1 */
	reg_val = mctl_read_w(_PLL_DDR1_CTRL_REG);
	reg_val &=~(1U<<31);
	mctl_write_w(reg_val, _PLL_DDR1_CTRL_REG);
	udelay(100);

	return 0;
}
/*****************************************************************************
Function : 	Set Bit Delay
parameter : DRAM parameter
return value : Void
*****************************************************************************/
static void bit_delay_compensation_standby(dram_para_t *para)
{
	unsigned int reg_val, i, j;

	reg_val = mctl_read_w(PGCR0);
	reg_val &= (~(0x1<<26));
	mctl_write_w(reg_val, PGCR0);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 11; j++) {
			mctl_write_w(reg[15 + j + (i*11)], (DATX0IOCR(j) + i * 0x80));
		}
	}
	for (i = 0; i < 31; i++) {
		mctl_write_w(reg[60 + i], ACIOCR1(i));
	}
	for (i = 0; i < 4; i++) {
		mctl_write_w(reg[95 + i], DXnSDLR6(i));
	}
	reg_val = mctl_read_w(PGCR0);
	reg_val |= (0x1<<26);
	mctl_write_w(reg_val, PGCR0);
}
/*****************************************************************************
Function : 	Set Master Priority
parameter : Void
return value : Void
*****************************************************************************/
static void set_master_priority_standby(void)
{
	//enable bandwidth limit windows and set windows size 1us
	mctl_write_w(0x18F,MC_TMR);//1us
	mctl_write_w(0x00010000,MC_BWCR);

	//set CPU QOS3,no limit
	mctl_write_w(0x0000000f,MC_MnCR0(0));
	mctl_write_w(0x00000000,MC_MnCR1(0));
	//set GPU QOS2,and bandwidth limit---1536MB->1400MB->256MB
	mctl_write_w(0x06000509,MC_MnCR0(1));
	mctl_write_w(0x01000578,MC_MnCR1(1));
	//set MAHB QOS3,and bandwidth limit---512MB->256MB->96MB
	mctl_write_w(0x0200000d,MC_MnCR0(2));
	mctl_write_w(0x00600100,MC_MnCR1(2));
	//set DMA QOS2,and bandwidth limit---256MB->100MB->80MB
	mctl_write_w(0x01000009,MC_MnCR0(3));
	mctl_write_w(0x00500064,MC_MnCR1(3));
	//set VE0 QOS2,and bandwidth limit---8192MB->5500MB->5000MB
	mctl_write_w(0x20000009,MC_MnCR0(4));
	mctl_write_w(0x1388157c,MC_MnCR1(4));
	//set CE QOS2,and bandwidth limit---100MB->64MB->32MB
	mctl_write_w(0x00640209,MC_MnCR0(5));
	mctl_write_w(0x00200040,MC_MnCR1(5));
	//set NAND QOS2,and bandwidth limit---256MB->128MB->64MB
	mctl_write_w(0x01000009,MC_MnCR0(8));
	mctl_write_w(0x00400080,MC_MnCR1(8));
	//set CSI0 QOS2,and bandwidth limit---256MB->128MB->100MB
	mctl_write_w(0x01000009,MC_MnCR0(11));
	mctl_write_w(0x00640080,MC_MnCR1(11));
	//set DE300 QOS3,and bandwidth limit---8192MB->2800MB->2400MB
	mctl_write_w(0x2000000d,MC_MnCR0(16));
	mctl_write_w(0x09600AF0,MC_MnCR1(16));
	//set IOMMU QOS3,and bandwidth limit---100MB->64MB->32MB
	mctl_write_w(0x0064000d,MC_MnCR0(25));
	mctl_write_w(0x00200040,MC_MnCR1(25));
	//set VE1 QOS3,and bandwidth limit---8192MB->2800MB->2400MB
	mctl_write_w(0x20000009,MC_MnCR0(26));
	mctl_write_w(0x1388157c,MC_MnCR1(26));
	//set VP9 QOS2,and bandwidth limit---8192MB->5500MB->5000MB
	mctl_write_w(0x20000209,MC_MnCR0(39));
	mctl_write_w(0x1388157c,MC_MnCR1(39));

	mctl_write_w(0x02802f05,SCHED);
	mctl_write_w(0x000000ff,PERFHPR0);
	mctl_write_w(0x0f00003f,PERFHPR1);

	dram_dbg_8("DRAM master priority setting ok.\n");
}
/*****************************************************************************
Function : 	Set Master Priority For Pad
parameter : Void
return value : Void
*****************************************************************************/
static void set_master_priority_pad_standby(void)
{
	//enable bandwidth limit windows and set windows size 1us
	mctl_write_w(0x18F,MC_TMR);//1us
	mctl_write_w(0x00010000,MC_BWCR);

	//set CPU QOS3,no limit
	mctl_write_w(0x01000009,MC_MnCR0(0));
	mctl_write_w(0x00500064,MC_MnCR1(0));
	//set GPU QOS2,and bandwidth limit---1536MB->1400MB->256MB
	mctl_write_w(0x06000009,MC_MnCR0(1));
	mctl_write_w(0x01000400,MC_MnCR1(1));
	//set MAHB QOS3,and bandwidth limit---512MB->256MB->96MB
	mctl_write_w(0x0200000d,MC_MnCR0(2));
	mctl_write_w(0x00600100,MC_MnCR1(2));
	//set DMA QOS2,and bandwidth limit---256MB->100MB->80MB
	mctl_write_w(0x01000009,MC_MnCR0(3));
	mctl_write_w(0x00500064,MC_MnCR1(3));
	//set VE0 QOS2,and bandwidth limit---8192MB->5500MB->5000MB
	mctl_write_w(0x20000209,MC_MnCR0(4));
	mctl_write_w(0x1388157c,MC_MnCR1(4));
	//set CE QOS2,and bandwidth limit---100MB->64MB->32MB
	mctl_write_w(0x00640209,MC_MnCR0(5));
	mctl_write_w(0x00200040,MC_MnCR1(5));
	//set NAND QOS2,and bandwidth limit---256MB->128MB->64MB
	mctl_write_w(0x01000009,MC_MnCR0(8));
	mctl_write_w(0x00400080,MC_MnCR1(8));
	//set CSI0 QOS2,and bandwidth limit---256MB->128MB->100MB
	mctl_write_w(0x01000009,MC_MnCR0(11));
	mctl_write_w(0x00640080,MC_MnCR1(11));
	//set DE300 QOS3,and bandwidth limit---8192MB->2800MB->2400MB
	mctl_write_w(0x20003f0c,MC_MnCR0(16));
	mctl_write_w(0x1388157c,MC_MnCR1(16));
	//set IOMMU QOS3,and bandwidth limit---100MB->64MB->32MB
	mctl_write_w(0x0064000d,MC_MnCR0(25));
	mctl_write_w(0x00200040,MC_MnCR1(25));
	//set VE1 QOS3,and bandwidth limit---8192MB->2800MB->2400MB
	mctl_write_w(0x2000050b,MC_MnCR0(26));
	mctl_write_w(0x1388157c,MC_MnCR1(26));
	//set VP9 QOS2,and bandwidth limit---8192MB->5500MB->5000MB
	mctl_write_w(0x20000209,MC_MnCR0(39));
	mctl_write_w(0x1388157c,MC_MnCR1(39));

	mctl_write_w(0x02802f05,SCHED);
	mctl_write_w(0x1f00003f,PERFHPR1);
	mctl_write_w(0x3f00005f,PERFLPR1);
	dram_dbg_8("DRAM master priority setting ok.\n");
}
/*****************************************************************************
Function : DRAM Timing Calculation Function
parameter : (Timing,CLK)
return value : Calculated Value
*****************************************************************************/
static unsigned int auto_cal_timing_standby(unsigned int time_ns, unsigned int clk)
{
	unsigned int value;
	value = (time_ns * clk)/1000 + ((((time_ns * clk) % 1000) != 0) ? 1 : 0);
	return value;
}
/*****************************************************************************
Function : DRAM Timing configuration Function
parameter : (DRAM parameter)
return value : void
*****************************************************************************/
static void auto_set_timing_para_standby(dram_para_t *para)
{
	unsigned int trefi = 0;
	unsigned int trfc  = 0;
	unsigned int type = 0;
	unsigned int reg_val = 0;
	unsigned int ctrl_freq = 0;
	type = para->dram_type ;
	ctrl_freq = para->dram_clk/2;
	if (type == 3) {
		trefi	= auto_cal_timing_standby(7800, ctrl_freq) / 32;
		trfc = auto_cal_timing_standby(350, ctrl_freq);
	} else if (type == 2) {
		trefi	= auto_cal_timing_standby(7800, ctrl_freq) / 32;
		trfc = auto_cal_timing_standby(328, ctrl_freq);
	} else {
		trefi	= auto_cal_timing_standby(3900, ctrl_freq) / 32;
		trfc = auto_cal_timing_standby(210, ctrl_freq);
	}
	mctl_write_w((para->dram_mr0) & 0xffff, DRAM_MR0);
	mctl_write_w((para->dram_mr1) & 0xffff, DRAM_MR1);
	mctl_write_w((para->dram_mr2) & 0xffff, DRAM_MR2);
	mctl_write_w((para->dram_mr3) & 0xffff, DRAM_MR3);
	mctl_write_w(((para->dram_odt_en) >> 4) & 0x3, LP3MR11);
	mctl_write_w(reg[2], DRAMTMG0);
	mctl_write_w(reg[3], DRAMTMG1);
	mctl_write_w(reg[4], DRAMTMG2);
	mctl_write_w(reg[5], DRAMTMG3);
	mctl_write_w(reg[6], DRAMTMG4);
	mctl_write_w(reg[7], DRAMTMG5);
	mctl_write_w(reg[8], DRAMTMG8);
	mctl_write_w(reg[9], PITMG0);
	mctl_write_w(reg[10], PTR3);
	mctl_write_w(reg[11], PTR4);
	reg_val = 0;
	reg_val = (trefi << 16) | (trfc << 0);
	mctl_write_w(reg_val, RFSHTMG);
}

/*****************************************************************************
作用：DRAM时钟展频设置函数
参数：dram_para_t *para
返回值：1
1.该函数由SD1人员提供PLL展频配置方法
*****************************************************************************/
static unsigned int ccm_set_pll_ddr_sscg(dram_para_t *para, unsigned int PLL_CTRL, unsigned int PLL_PAT)
{
	unsigned int  reg_val;
	//如果PLLn的分频M1为1，需要将bit19设为 1
	reg_val = (mctl_read_w(PLL_CTRL) >> 1) & 0x1;
	if(reg_val)
		mctl_write_w((mctl_read_w(PLL_PAT) | (0x1 << 19)), PLL_PAT);
	//计算WAVE_BOT
	reg_val = (para->dram_tpr13 >> 20) & 0x7;

	switch(reg_val)
	{
		case 1:
			mctl_write_w((0xCCCCU | (0x3U << 17) | (0x48U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
		case 2:
			mctl_write_w((0x9999U | (0x3U << 17) | (0x90U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
		case 3:
			mctl_write_w((0x6666U | (0x3U << 17) | (0xD8U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
		case 4:
			mctl_write_w((0x3333U | (0x3 << 17) | (0x120U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
		case 5:
			mctl_write_w(((0x3U << 17) | (0x158U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
		default:	//0.4
			mctl_write_w((0x3333U | (0x3 << 17) | (0x120U << 20) | (0x3U << 29) | (0x1U << 31)), PLL_PAT);
			break;
	}
	reg_val = mctl_read_w(PLL_CTRL);
	reg_val |= 0x1 <<24;
	mctl_write_w(reg_val, PLL_CTRL);
	return 0;
}

/*****************************************************************************
作用：DRAM时钟设置函数
参数：dram_para_t *para ,PLL_DDRx_CTRL_REG
返回值：设置的PLL频率值
*****************************************************************************/
static unsigned int ccm_set_pll_ddr_clk(unsigned int pll, dram_para_t *para)
{
	unsigned int rval;
	unsigned int PLL_CTRL_ADDR, PLL_PAT_ADDR;
	unsigned int N = 12, M0 = 1, M1 = 0;
	unsigned int pll_clk, div;

	if(pll)
	{
		PLL_CTRL_ADDR = _PLL_DDR1_CTRL_REG;
		PLL_PAT_ADDR  = PLL_DDR1_PAT_CTL_REG;
	}
	else
	{
		PLL_CTRL_ADDR = _PLL_DDR0_CTRL_REG;
		PLL_PAT_ADDR  = PLL_DDR0_PAT_CTL_REG;
	}

	//时钟源选择
//	if((para->dram_tpr13 >> 6) & 0x1)
//		pll_clk = (para->dram_tpr9 << 1);
//	else
//		pll_clk = (para->dram_clk << 1);

	if(((para->dram_tpr13 >> 6) & 0x1) == pll)
		pll_clk = (para->dram_clk << 1);
	else
		pll_clk = (para->dram_tpr9 << 1);

	//获取频率值倍频系数
	div = pll_clk / 24;	//pll两倍于dram_clk, div = dram_clk * 2 / 24
	if(div < 12)		//实际应用中，N不应小于12
	{
		N = 12;
		M0 = 1;
		M1 = 1;
	}
	else
	{
		N = div;
		M0 = 1;
		M1 = 1;
	}
	//设定频率值，分频值M0默认为1，分频值M1默认为1，PLL_CLK = 24*div/M0/M1
	rval = mctl_read_w(PLL_CTRL_ADDR);
	rval &= ~(0x7f << 8 | 0x3 << 0);
	rval |= ((0x1U << 31) | ((N - 1) << 8) | ((M1 - 1) << 1) | ((M0 - 1) << 0));
	mctl_write_w(rval, PLL_CTRL_ADDR);
	/*解除锁定 */
	rval &= ~(1U << 29);
	mctl_write_w(rval, PLL_CTRL_ADDR);
	/*打开锁定  */
	mctl_write_w(rval|(1U << 29), PLL_CTRL_ADDR);
	/*等待PLL LOCK 标志 */
	while(!(mctl_read_w(PLL_CTRL_ADDR) & 0x1<<28));
	udelay(20);
	/*设置展频  */
	if((para->dram_tpr13 >> 23) & 0x1)
		ccm_set_pll_ddr_sscg(para, PLL_CTRL_ADDR, PLL_PAT_ADDR);
	return 24 * div;
}

/*****************************************************************************
Function : System resource initialization
parameter : (DRAM parameter)
return value : 1(Meaningless)
*****************************************************************************/
static unsigned int mctl_sys_init_standby(dram_para_t *para)
{
	unsigned int reg_val = 0;
	unsigned int ret_val = 0;
	/*close mbus gate*/
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val &= ~(1U << 31);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/*mbus reset*/
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val &= ~(1U << 30);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/*close DRAM AHB gate*/
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val &= ~(1U << 0);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/*DRAM AHB reset*/
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val &= ~(1U << 16);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/* 关闭PLL_DDR0 */
	reg_val = mctl_read_w(_PLL_DDR0_CTRL_REG);
	reg_val &= ~(1U << 31);
	mctl_write_w(reg_val, _PLL_DDR0_CTRL_REG);
	/* 关闭PLL_DDR1 */
	reg_val = mctl_read_w(_PLL_DDR1_CTRL_REG);
	reg_val &= ~(1U << 31);
	mctl_write_w(reg_val, _PLL_DDR1_CTRL_REG);
	/*DRAM控制器reset*/
	reg_val = mctl_read_w(_DRAM_CLK_REG);
	reg_val &= ~(0x1U << 30);
	mctl_write_w(reg_val, _DRAM_CLK_REG);
	udelay(10);
	/*Set DRAM PLL Frequency*/
	if(para->dram_tpr13 >> 6 & 0x1)
	{	/*选择PLL_DDR1*/
		ret_val = ccm_set_pll_ddr_clk(1, para);
		reg_val = mctl_read_w(_DRAM_CLK_REG);
		reg_val &= ~(0x3 << 24);
		reg_val |= (0x1 << 24);	//时钟源设为PLL1
		mctl_write_w(reg_val, _DRAM_CLK_REG);
		para->dram_clk = ret_val / 2;
		dram_dbg_4("pll_ddr1 = %d MHz\n",ret_val);
		/*如果使能DFS调频，开启另外一个PLL*/
		if(para->dram_tpr9 != 0)
		{
			ret_val = ccm_set_pll_ddr_clk(0, para);
			para->dram_tpr9 = ret_val / 2;
			dram_dbg_4("pll_ddr0 = %d MHz\n",ret_val);
		}
	}
	else
	{ 	/*选择PLL_DDR0*/
		ret_val = ccm_set_pll_ddr_clk(0, para);
		reg_val = mctl_read_w(_DRAM_CLK_REG);
		reg_val &= ~(0x3 << 24);
		reg_val |= (0x0 << 24);	//时钟源设为PLL0
		mctl_write_w(reg_val, _DRAM_CLK_REG);
		para->dram_clk = ret_val / 2;
		dram_dbg_4("pll_ddr0 = %d MHz\n",ret_val);
		/*如果使能DFS调频，开启另外一个PLL*/
		if(para->dram_tpr9 != 0)
		{
			ret_val = ccm_set_pll_ddr_clk(1, para);
			para->dram_tpr9 = ret_val / 2;
			dram_dbg_4("pll_ddr1 = %d MHz\n",ret_val);
		}
	}
	udelay(1000);
	reg_val = mctl_read_w(_DRAM_CLK_REG);
	reg_val &= ~(0x3 << 0);		//后分频系数设为1
	mctl_write_w(reg_val, _DRAM_CLK_REG);
	reg_val |= 0x1 << 27;		//update CLK
	mctl_write_w(reg_val, _DRAM_CLK_REG);

	/*释放DRAM AHB域的reset*/
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val |= (1U << 16);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/*打开DRAM AHB域的时钟*/
	reg_val = mctl_read_w(_DRAM_BGR_REG);
	reg_val |= (1U << 0);
	mctl_write_w(reg_val, _DRAM_BGR_REG);
	/*保证mbus时钟开前关闭可能的访问*/
	dram_disable_all_master();
	/*释放DRAM mbus域的reset*/
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val |= (1U << 30);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/*打开DRAM mbus域的时钟*/
	reg_val = mctl_read_w(_MBUS_CFG_REG);
	reg_val |= (1U << 31);
	mctl_write_w(reg_val, _MBUS_CFG_REG);
	/*释放DRAM控制器reset*/
	reg_val = mctl_read_w(_DRAM_CLK_REG);
	reg_val |= 0x1U << 30;
	mctl_write_w(reg_val, _DRAM_CLK_REG);
	udelay(10);
	/*Enable DRAM Controller CLK*/
	mctl_write_w(0x8000, CLKEN);
	udelay(10);
	return DRAM_RET_OK;
}
/*****************************************************************************
Function : DRAM Controller Configuration
parameter : (DRAM parameter)
return value : void
*****************************************************************************/
static void mctl_com_init_standby(dram_para_t *para)
{
	unsigned int reg_val, ret_val;
	mctl_write_w(reg[0], MC_WORK_MODE);
	mctl_write_w(reg[1], MC_R1_WORK_MODE);
	/*ODT MAP */
	reg_val = (mctl_read_w(MC_WORK_MODE) & 0x1);
	if (reg_val)
		ret_val = 0x303;
	else
		ret_val = 0x201;
	mctl_write_w(ret_val, ODTMAP);
	/*half DQ mode */
	if (para->dram_para2 & 0x1) {
		mctl_write_w(0, DXnGCR0(2));
		mctl_write_w(0, DXnGCR0(3));
	}
}
/*****************************************************************************
Function : DRAM Controller Basic Initialization
parameter : (0,DRAM parameter)
return value : 0-FAIL  , other-Success
*****************************************************************************/
static unsigned int mctl_channel_init_standby(unsigned int ch_index, dram_para_t *para)
{
	unsigned int reg_val = 0, ret_val = 0;
	unsigned int dqs_gating_mode = 0;
	unsigned int i = 0;
	unsigned int rval = 1;
	dqs_gating_mode = (para->dram_tpr13 >> 2) & 0x3;
	/***********************************
	 Function : Set Phase
	 **********************************/
	reg_val = mctl_read_w(PGCR2);
	reg_val &= ~(0x3 << 10);
	reg_val |= 0x0 << 10;
	reg_val &= ~(0x3 << 8);
	reg_val |= 0x3 << 8;
	mctl_write_w(reg_val, PGCR2);
	dram_dbg_8("PGCR2 is %x\n", reg_val);
	/***********************************
	 Function : AC/DX IO Configuration
	 **********************************/
	ret_val = para->dram_odt_en & 0x1;
	dram_dbg_8("DRAMC read ODT type : %d (0: off  1: dynamic on).\n",
			ret_val);
	ret_val = ~(para->dram_odt_en) & 0x1;
	for(i = 0; i < 4; i++)
	{
		/*byte 0/byte 1/byte 3/byte 4 */
		reg_val = mctl_read_w(DXnGCR0(i));
		reg_val &= ~(0x3U << 4);
		reg_val |= (ret_val << 5); /* ODT:2b'00 dynamic ,2b'10 off */
		reg_val &= ~(0x1U << 1); /* SSTL IO mode */
		reg_val &= ~(0x3U << 2); /*OE mode: 0 Dynamic */
		reg_val &= ~(0x3U << 12); /*Power Down Receiver: Dynamic */
		reg_val &= ~(0x3U << 14); /*Power Down Driver: Dynamic */
		if(para->dram_clk > 672)
		{
			reg_val &= ~(0x3U << 9);
			reg_val |= (0x2U << 9);
		}
		mctl_write_w(reg_val, DXnGCR0(i));
	}
	dram_dbg_8("DXnGCR0 = %x\n", reg_val);

	reg_val = mctl_read_w(ACIOCR0);
	reg_val |= (0x1 << 1);
	reg_val &= ~(0x1 << 11);
	mctl_write_w(reg_val, ACIOCR0);
	/***********************************
	 Function : AC/DX IO Bit Delay
	 **********************************/
	bit_delay_compensation_standby(para);
	/***********************************
	 Function : DQS Gate Mode Choose
	 **********************************/
	switch(dqs_gating_mode)
	{
		case 1: /*open DQS gating */
			reg_val = mctl_read_w(PGCR2);
			reg_val &= ~(0x3 << 6);
			mctl_write_w(reg_val, PGCR2);

			reg_val = mctl_read_w(DQSGMR);
			reg_val &= ~((0x1 << 8) | 0x7);
			mctl_write_w(reg_val, DQSGMR);
			dram_dbg_8("DRAM DQS gate is open.\n");

			break;
		case 2: /*auto gating pull up */
			reg_val = mctl_read_w(PGCR2);
			reg_val &= ~(0x3 << 6);
			reg_val |= (0x2 << 6);
			mctl_write_w(reg_val, PGCR2);

			ret_val = ((mctl_read_w(DRAMTMG2) >> 16) & 0x1f) - 2;
			reg_val = mctl_read_w(DQSGMR);
			reg_val &= ~((0x1 << 8) | (0x7));
			reg_val |= ((0x1 << 8) | (ret_val));
			mctl_write_w(reg_val, DQSGMR);

			reg_val = mctl_read_w(DXCCR); /*dqs pll up */
			reg_val |= (0x1 << 27);
			reg_val &= ~(0x1U << 31);
			mctl_write_w(reg_val, DXCCR);
			dram_dbg_8("DRAM DQS gate is PU mode.\n");
			break;
		default:
			/*close DQS gating--auto gating pull down */
			/*for aw1680 standby problem, reset gate */
			reg_val = mctl_read_w(PGCR2);
			reg_val &= ~(0x1 << 6);
			mctl_write_w(reg_val, PGCR2);

			reg_val = mctl_read_w(PGCR2);
			reg_val |= (0x3 << 6);
			mctl_write_w(reg_val, PGCR2);
			dram_dbg_8("DRAM DQS gate is PD mode.\n");
			break;
	}
	/***********************************
	 Function : Pull Up/Down Strength
	 **********************************/
	if((para->dram_type == 6) || (para->dram_type == 7))
	{
		reg_val = mctl_read_w(DXCCR);
		reg_val &= ~(0x7U << 28);
		reg_val &= ~(0x7U << 24);
		reg_val |= (0x2U << 28);
		reg_val |= (0x2U << 24);
		mctl_write_w(reg_val, DXCCR);
	}
	/***********************************
	 Function : Training
	 **********************************/
	if((para->dram_para2 >> 12) & 0x1)
	{
		reg_val = mctl_read_w(DTCR);
		reg_val &= (0xfU << 28);
		reg_val |= 0x03003087;
		mctl_write_w(reg_val, DTCR); /*two rank */
	}
	else
	{
		reg_val = mctl_read_w(DTCR);
		reg_val &= (0xfU << 28);
		reg_val |= 0x01003087;
		mctl_write_w(reg_val, DTCR); /*one rank */
	}
	/* ZQ pad release */
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING_REG);
	reg_val &= (~(0x1 << 1));
	mctl_write_w(reg_val, VDD_SYS_PWROFF_GATING_REG);
	udelay(10);
	/* ZQ calibration */
	reg_val = mctl_read_w(ZQCR);
	reg_val &= ~(0x03ffffff);
	reg_val |= ((para->dram_zq) & 0xffffff);
	reg_val |= (0x2 << 24);		//divide 32
	mctl_write_w(reg_val, ZQCR);
	if(dqs_gating_mode == 1)
	{
		reg_val = 0x52;
		mctl_write_w(reg_val, PIR);
		reg_val |= (0x1 << 0);
		mctl_write_w(reg_val, PIR);
		while((mctl_read_w(PGSR0) & 0x1) != 0x1);
		udelay(10);
		reg_val = 0x20; /*DDL CAL; */
	}
	else
	{
		reg_val = 0x62;
	}
	mctl_write_w(reg_val, PIR);
	reg_val |= (0x1 << 0);
	mctl_write_w(reg_val, PIR);
	udelay(10);
	while((mctl_read_w(PGSR0) & 0x1) != 0x1);

	reg_val = mctl_read_w(PGCR3);
	reg_val &= (~(0x3 << 25));
	reg_val |= (0x2 << 25);
	mctl_write_w(reg_val, PGCR3);
	udelay(10);
	/* entry self-refresh */
	reg_val = mctl_read_w(PWRCTL);
	reg_val |= 0x1 << 0;
	mctl_write_w(reg_val, PWRCTL);
	while(((mctl_read_w(STATR) & 0x7) != 0x3));

	/* pad release */
	reg_val = mctl_read_w(VDD_SYS_PWROFF_GATING_REG);
	reg_val &= ~(0x1 << 0);
	mctl_write_w(reg_val, VDD_SYS_PWROFF_GATING_REG);
	udelay(10);

	/* exit self-refresh but no enable all master access */
	reg_val = mctl_read_w(PWRCTL);
	reg_val &= ~(0x1 << 0);
	mctl_write_w(reg_val, PWRCTL);
	while(((mctl_read_w(STATR) & 0x7) != 0x1));
	udelay(15);

	/* training :DQS gate training */
	if(dqs_gating_mode == 1)
	{
		reg_val = mctl_read_w(PGCR2);
		reg_val &= ~(0x3 << 6);
		mctl_write_w(reg_val, PGCR2);

		reg_val = mctl_read_w(PGCR3);
		reg_val &= (~(0x3 << 25));
		reg_val |= (0x1 << 25);
		mctl_write_w(reg_val, PGCR3);
		udelay(1);

		reg_val = 0x401;
		mctl_write_w(reg_val, PIR);
		while((mctl_read_w(PGSR0) & 0x1) != 0x1);
	}

	/***********************************
	 Function : Training Information
	 **********************************/
	reg_val = mctl_read_w(PGSR0);
	if((reg_val >> 20) & 0xff)
	{
		/* training ERROR information */
		dram_dbg_4("[DEBUG_4]PGSR0 = 0x%x\n", reg_val);
		if((reg_val >> 20) & 0x1)
		{
			dram_dbg_4("ZQ calibration error, check external 240 ohm resistor.\n");
		}
		rval = 0;
	}
	/***********************************
	 Function : Controller Setting
	 **********************************/
	/*after initial done */
	while((mctl_read_w(STATR) & 0x1) != 0x1);
	/*refresh update, from AW1680/1681 */
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val |= (0x1U) << 31;
	mctl_write_w(reg_val, RFSHCTL0);
	udelay(10);
	reg_val = mctl_read_w(RFSHCTL0);
	reg_val &= ~(0x1U << 31);
	mctl_write_w(reg_val, RFSHCTL0);
	udelay(10);

	/*after initial before write or read must clear credit value */
	reg_val = mctl_read_w(MC_CCCR);
	reg_val |= (0x1U) << 31;
	mctl_write_w(reg_val, MC_CCCR);
	udelay(10);
	/*PHY choose to update PHY or command mode */
	reg_val = mctl_read_w(PGCR3);
	reg_val &= ~(0x3 << 25);
	mctl_write_w(reg_val, PGCR3);
	/***********************************
	 Function : DQS Gate Optimization
	 **********************************/
//	if((para->dram_type) == 6 || (para->dram_type) == 7)
//	{
		if(dqs_gating_mode == 1)
		{
			reg_val = mctl_read_w(DXCCR);
			reg_val &= ~(0x3 << 6);
			reg_val |= (0x1 << 6);
			mctl_write_w(reg_val, DXCCR);
		}
//	}
	return rval;
}
/*****************************************************************************
Function : DRAM Controller Basic Initialization
parameter : (DRAM parameter)
return value : 0-FAIL  , other-Success
*****************************************************************************/
static unsigned int mctl_core_init_standby(dram_para_t *para)
{
	unsigned int ret_val = 0;
	mctl_sys_init_standby(para);
	mctl_com_init_standby(para);
	auto_set_timing_para_standby(para);
	ret_val = mctl_channel_init_standby(0, para);
	return ret_val;
}
/*****************************************************************************
Function : DRAM Initialization Function Entry
parameter : (Meaningless,DRAM parameter)
return value : 0-FAIL  , other-DRAM size
*****************************************************************************/
static signed int init_DRAM_standby(int type, dram_para_t *para)
{
	unsigned int ret_val = 0;
	unsigned int reg_val = 0;
	//unsigned int pad_hold = 0;
	unsigned int dram_size = 0;

	dram_dbg_0("DRAM STANDBY DRIVE INFO: %s\n",STB_VERSION);
/*****************************************************************************
Function : DRAM Controller Basic Initialization
*****************************************************************************/
	dram_dbg_4("DRAM CLK =%d MHZ\n", para->dram_clk);
	dram_dbg_4("DRAM Type =%d (2:DDR2, 3:DDR3, 6:LPDDR2, 7:LPDDR3)\n", para->dram_type);
	dram_dbg_4("DRAM zq value: 0x%x\n", para->dram_zq);
	ret_val = mctl_core_init_standby(para);
	if (ret_val == 0) {
		dram_dbg_4("DRAM initial error : 1 !\n");
		return 0;
	}
/*****************************************************************************
Function : DRAM size
*****************************************************************************/
	dram_size = (para->dram_para2 >> 16) & 0x7fff;
/*****************************************************************************
Function : Auto SR Config
*****************************************************************************/
	mctl_write_w(reg[12], ASRC);
	mctl_write_w(reg[13], ASRTC);

	if((para->dram_tpr13 >> 30) & 0x1)
	{
		reg_val = mctl_read_w(PWRCTL);
		reg_val |= (0x1 << 0);
		mctl_write_w(reg_val, PWRCTL);
		dram_dbg_0("Enable Auto SR\n");
	}
	else
	{
		reg_val = mctl_read_w(PWRCTL);
		reg_val &= ~(0x1 << 0);
		mctl_write_w(reg_val, PWRCTL);
		dram_dbg_0("Disable Auto SR\n");
	}

/*****************************************************************************
Function : Power Related
1.HDR/DDR CLK dynamic
2.Close ZQ calibration module
*****************************************************************************/
	if ((para->dram_tpr13 >> 9) & 0x1) {
		reg_val = mctl_read_w(PGCR0);
		reg_val &= ~(0xf << 12);
		reg_val |= (0x5 << 12);
		mctl_write_w(reg_val, PGCR0);
		dram_dbg_8("HDR/DDR always on mode!\n");
	} else {
		reg_val = mctl_read_w(PGCR0);
		reg_val &= ~(0xf << 12);
		mctl_write_w(reg_val, PGCR0);
		dram_dbg_8("HDR/DDR dynamic mode!\n");
	}
	reg_val = mctl_read_w(ZQCR);
	reg_val |= (0x1U << 31);
	mctl_write_w(reg_val, ZQCR);
/*****************************************************************************
Function : Performance Optimization
1.VTF Function
2.PAD HOLD Function
3.LPDDR3 ODT delay
*****************************************************************************/
	if((para->dram_tpr13 >> 8) & 0x1)
	{
		reg_val = mctl_read_w(VTFCR);
		reg_val |= (0x1 << 8);
		reg_val |= (0x1 << 9);
		mctl_write_w(reg_val, VTFCR);
		dram_dbg_8("VTF enable\n");
	}
	if((para->dram_tpr13 >> 26) & 0x1)
	{
		reg_val = mctl_read_w(PGCR2);
		reg_val &= ~(0x1 << 13);
		mctl_write_w(reg_val, PGCR2);
		dram_dbg_8("DQ hold disable!\n");
	}
	else
	{
		reg_val = mctl_read_w(PGCR2);
		reg_val |= (0x1 << 13);
		mctl_write_w(reg_val, PGCR2);
		dram_dbg_8("DQ hold enable!\n");
	}
	if(para->dram_type == 7)
	{
		reg_val = mctl_read_w(ODTCFG);
		reg_val &= ~(0xf << 16);
		reg_val |= (0x1 << 16);
		mctl_write_w(reg_val, ODTCFG);
	}
/*****************************************************************************
Function : Priority Setting Module
*****************************************************************************/
	if(!((para->dram_tpr13 >> 27) & 0x1))
	{
		if((para->dram_tpr13 >> 24) & 0x1)
		{
			set_master_priority_pad_standby();
		}else
		{
			set_master_priority_standby();
		}
	}
/*****************************************************************************
Function End
*****************************************************************************/
	return dram_size;
}
/*****************************************************************************
Function : DRAM Standby Function Exit
parameter : (DRAM parameter)
return value : 0-FAIL  , other-DRAM size
*****************************************************************************/
static unsigned int __dram_power_up_process(dram_para_t *para)
{
	unsigned int ret = 0;
	ret = init_DRAM_standby(0, para);
	return ret;
}

unsigned int dram_power_save_process(void)
{
	__dram_power_save_process();
	return 0;
}

unsigned int dram_power_up_process(void)
{
	unsigned int ret = 0;
	ret = __dram_power_up_process(pdram_para);

	return ret;
}
#else
/********************************************************************************
 *FPGA standby code
 ********************************************************************************/

#endif
#endif //CONFIG_ARCH_SUN50IW3P1