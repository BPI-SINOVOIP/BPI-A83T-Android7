/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 rsb module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : rsb.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: rsb module.
* Update  : date                auther      ver     notes
*           2012-5-22 9:49:38   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "rsb_i.h"

//rsb can be config used or not
#if  RSB_USED
static volatile bool rsb_lock = TRUE;
#if (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
volatile u32 rsb_audio_used = 0;
#endif

static void rsb_set_clk(u32 sck)
{
#ifndef FPGA_PLATFORM
	u32 sda_odly;
	u32 src_clk = ccu_get_sclk_freq(CCU_SYS_CLK_APBS2);
	u32 div = src_clk / sck / 2 - 1;

	if (div > 0xff) {
		ERR("rsb div overflow");
		div = 0xff;
	}

	div &= 0xff;
	sda_odly = ((div >> 1) - 1) >> 1;
	if (sda_odly < 1)
		sda_odly = 1;
	else if (sda_odly > 7)
		sda_odly = 7;
	u32 rval = (div & 0xff) | ((sda_odly & 0x7) << 8);
	writel(rval, RSB_REG_CCR);
#else
	//fpga platmform: apb clock = 24M, rsb clock = 2M.
	//0x201->6M
	//0x202->4M
	//0x203->3M
	//0x204->2.4M
	//0x205->2M
	//0x206->1.7M
	//0x205 = 0x200 + 0x005,the 0x200 is SDA output delay.
	writel(0x20b, RSB_REG_CCR);
#endif
	INF("RSB_REG_CCR value = %x\n", readl(RSB_REG_CCR));
}

static s32 rsb_clkchangecb(u32 command, u32 freq)
{
#ifndef     FPGA_PLATFORM
	u32 div;
	u32 sda_odly;
	u32 rval;
#endif

	switch(command) {
	case CCU_CLK_CLKCHG_REQ:
		//check rsb is busy now
		//...

		//clock will be change, lock rsb interface
		rsb_lock = TRUE;
		INF("rsb clk change request\n");
		return OK;
	case CCU_CLK_CLKCHG_DONE:
		//clock change finish, re-config rsb clock,
		//maybe re-config rsb clock should do somethings???
#ifndef     FPGA_PLATFORM
		div = freq / RSB_SCK_FREQ / 2 - 1;
		if (div > 0xff) {
			ERR("rsb div overflow");
			div = 0xff;
		}

		div &= 0xff;
		sda_odly = ((div >> 1) - 1) >> 1;
		if (sda_odly < 1)
			sda_odly = 1;
		else if (sda_odly > 7)
			sda_odly = 7;
		rval = (div & 0xff) | ((sda_odly & 0x7) << 8);
		writel(rval, RSB_REG_CCR);
#endif
		//unlock rsb interface
		rsb_lock = FALSE;
		INF("rsb clk change done\n");
		return OK;
	default:
		break;
	}
	return -ESRCH;
}

s32 rsb_init(void)
{
	//initialize rsb sck and sda pins
//#if PIN_INIT_BY_CPUS
	//judge pmu whether exist
	//pin_set_multi_sel(PIN_GRP_PL, 0, 0); /* PL0 config as input */
	//pin_set_pull(PIN_GRP_PL, 0, PIN_PULL_DOWN); /* RSB_SCK pull-down */
	//time_cdelay(10);
	//if (pin_read_data(PIN_GRP_PL, 0) == 0) {
	//	rsb_lock = TRUE;
	//	return OK;
	//}
	pin_set_multi_sel(PIN_GRP_PL, 0, 2);                //PL0 config as RSB_SCK
	pin_set_pull     (PIN_GRP_PL, 0, PIN_PULL_UP);      //RSB_SCK pull-up
	pin_set_drive    (PIN_GRP_PL, 0, PIN_MULTI_DRIVE_2);//RSB_SCK drive level 2
	pin_set_multi_sel(PIN_GRP_PL, 1, 2);                //PL1 config as RSB_SDA
	pin_set_pull     (PIN_GRP_PL, 1, PIN_PULL_UP);      //RSB_SDA pull-up
	pin_set_drive    (PIN_GRP_PL, 1, PIN_MULTI_DRIVE_2);//RSB_SDA drive level 2
//#endif
	//enbale rsb clock, set reset as de-assert state.
	//boot0 maybe use the rsb, so we must reset rsb firstly at 2015-10-23.
	ccu_set_mclk_reset(CCU_MOD_CLK_R_RSB, CCU_CLK_RESET);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_RSB, CCU_CLK_OFF);
	udelay(2);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_RSB, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_R_RSB, CCU_CLK_NRESET);

	//reset rsb controller
	writel(RSB_SOFT_RST, RSB_REG_CTRL);

	/* set twi clock for translate devices form twi to rsb */
	rsb_set_clk(TWI_CLOCK_FREQ);

	//pmu initialize as rsb mode
	rsb_set_pmu_mode(0x00, 0x3e, 0x7c);

	/* init rsb to normal clk */
	rsb_set_clk(RSB_SCK_FREQ);

	rsb_set_run_time_addr(RSB_SADDR_AW1660, RSB_RTSADDR_AW1660);

	//register uart clock change notifier
	ccu_reg_mclk_cb(CCU_MOD_CLK_R_RSB, rsb_clkchangecb);

	//ensure rsb is unlock
	rsb_lock = FALSE;

	return OK;
}

s32 rsb_exit(void)
{
	return OK;
}

s32 rsb_read(u32 devaddr, u8 regaddr, u32 *data, u32 datatype)
{
	u32 cmd = 0;

	if (rsb_lock)
	{
		return -EACCES;
	}

	if ((datatype != RSB_DATA_TYPE_BYTE) && (datatype != RSB_DATA_TYPE_HWORD) && (datatype != RSB_DATA_TYPE_WORD)) {
		ERR("err datatype %d\n", datatype);
		return -EINVAL;
	}

	if(NULL==data){
		ERR("data=0\n");
		return -EINVAL;
	}

	writel(devaddr << RSB_RTSADDR_SHIFT, RSB_REG_SADDR);
	writel(regaddr, RSB_REG_DADDR0);

	switch(datatype){
	case RSB_DATA_TYPE_BYTE:
		cmd = RSB_CMD_BYTE_READ;
		break;
	case RSB_DATA_TYPE_HWORD:
		cmd = RSB_CMD_HWORD_READ;
		break;
	case RSB_DATA_TYPE_WORD:
		cmd = RSB_CMD_WORD_READ;
		break;
	default:
		break;
	}
	writel(cmd,RSB_REG_CMD);

	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);

	//wait transfer complete
	if (rsb_wait_state() != OK)
	{
		ERR("rsb read failed\n");
		ERR("read devaddr:%x, regaddr:%x, data:%x, datatype:%x\n", devaddr, regaddr, *data, datatype);
		return -EFAIL;
	}

	switch(datatype){
	case RSB_DATA_TYPE_BYTE:
		*data = readl(RSB_REG_DATA0) & 0xff;
		break;
	case RSB_DATA_TYPE_HWORD:
		*data = readl(RSB_REG_DATA0) & 0xffff;
		break;
	case RSB_DATA_TYPE_WORD:
		*data = readl(RSB_REG_DATA0);
		break;
	default:
		break;
	}

	INF("read devaddr:%x, regaddr:%x, data:%x, datatype:%x\n", devaddr, regaddr, *data, datatype);

	return OK;
}

s32 rsb_write(u32 devaddr, u8 regaddr, u32 *data, u32 datatype)
{
	u32 cmd = 0;

	if (rsb_lock)
	{
		return -EACCES;
	}

	if(data == NULL){
		ERR("data=0\n");
		return -EINVAL;
	}

	if ((datatype != RSB_DATA_TYPE_BYTE) && (datatype != RSB_DATA_TYPE_HWORD) && (datatype != RSB_DATA_TYPE_WORD)) {
		ERR("err datatype %d\n", datatype);
		return -EINVAL;
	}

	INF("write devaddr:%x, regaddr:%x, data:%x, datatype:%x\n", devaddr, regaddr, *data, datatype);

	writel(devaddr << RSB_RTSADDR_SHIFT, RSB_REG_SADDR);
	writel(regaddr, RSB_REG_DADDR0);

	writel(*data, RSB_REG_DATA0);

	switch(datatype)    {
	case RSB_DATA_TYPE_BYTE:
		cmd = RSB_CMD_BYTE_WRITE;
		break;
	case RSB_DATA_TYPE_HWORD:
		cmd = RSB_CMD_HWORD_WRITE;
		break;
	case RSB_DATA_TYPE_WORD:
		cmd = RSB_CMD_WORD_WRITE;
		break;
	default:
		break;
	}
	writel(cmd,RSB_REG_CMD);

	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);

	//wait transfer complete
	if (rsb_wait_state() != OK)
	{
		ERR("rsb write failed\n");
		ERR("write devaddr:%x, regaddr:%x, data:%x, datatype:%x\n", devaddr, regaddr, *data, datatype);
		return -EFAIL;
	}

	return 0;
}

s32 rsb_set_pmu_mode(u32 slave_addr, u32 reg, u32 data)
{
	//set pmu work mode
	writel(RSB_PMU_INIT | (slave_addr << 1) | (reg << 8) | (data << 16), RSB_REG_PMCR);

	//wait pmu mode set complete
	while(readl(RSB_REG_PMCR) & RSB_PMU_INIT)
	{
		;
	}

	//wait transfer complete
	if (rsb_wait_state() != OK)
	{
		ERR("rsb set pmu mode failed\n");
		return -EFAIL;
	}

	return OK;
}

s32 rsb_set_run_time_addr(u32 saddr,u32 rtsaddr)
{
	writel((saddr<<RSB_SADDR_SHIFT) | (rtsaddr<<RSB_RTSADDR_SHIFT), RSB_REG_SADDR);
	writel(RSB_CMD_SET_RTSADDR, RSB_REG_CMD);
	writel(readl(RSB_REG_CTRL)|RSB_START_TRANS, RSB_REG_CTRL);

	//wait transfer complete
	if (rsb_wait_state() != OK)
	{
		ERR("set rtsaddr failed, saddr:%x, rtsaddr:%x\n", saddr, rtsaddr);
		return -EFAIL;
	}
#if (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN50IW1P1)
	if (RSB_RTSADDR_AW1653 == rtsaddr) {
		rsb_audio_used = 1;
	}
#endif
	return OK;
}

s32 rsb_wait_state(void)
{
	s32  ret = FAIL;
	u32  stat;
	while (1)
	{
		stat = readl(RSB_REG_STAT);
		if (stat & RSB_LBSY_INT)
		{
			//transfer error
			ERR("loading busy\n");
			ret = -EBUSY;
			break;
		}
		if (stat & RSB_TERR_INT)
		{
			//transfer error
			ERR("rsb trans err[%x]\n", ((stat >> 8) & 0xffffff));
			ret = -EFAIL;
			break;
		}
		if (stat & RSB_TOVER_INT)
		{
			//transfer complete
			ret = OK;
			break;
		}
	}
	//clear state flag
	writel(stat, RSB_REG_STAT);

	return ret;
}

#if ((defined KERNEL_USED) || (defined TF_USED)) /* only used in kernel mode */
s32 rsb_read_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u32 datatype = 0;
	u32 devaddr;
	u8  regaddr[RSB_TRANS_BYTE_MAX];
	u32 data[RSB_TRANS_BYTE_MAX];
	s32 result = OK;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]       |para[1]|para[2]   |para[3]|para[4]|para[5]|para[6]|
	 * |(len|datatype)|devaddr|regaddr0~3|data0  |data1  |data2  |data3  |
	 */
	len      = pmessage->paras[0] & 0xffff;
	datatype = (pmessage->paras[0] >> 16) & 0xffff;
	devaddr  = pmessage->paras[1];

	if ((len > RSB_TRANS_BYTE_MAX) || ((datatype !=  RSB_DATA_TYPE_BYTE) && (datatype !=  RSB_DATA_TYPE_HWORD) && (datatype !=  RSB_DATA_TYPE_WORD))) {
		WRN("rsb read reg paras error\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
	{
		regaddr[i] = ((pmessage->paras[2]) >> (i * 8)) & 0xff;
	}

	//read data one by one
	for (i = 0; i < len; i++)
	{
		result |= rsb_read(devaddr, regaddr[i], &(data[i]), datatype);
	}

	//copy readout data to pmessage->paras
	for (i = 0; i < len; i++)
	{
		//pack 32bit data0~data3 into 32bit pmessage->paras[3]~paras[6]
		pmessage->paras[3 + i] = data[i];
	}

	return result;
}

s32 rsb_write_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u32 datatype = 0;
	u32 devaddr;
	u8  regaddr[RSB_TRANS_BYTE_MAX];
	u32 data[RSB_TRANS_BYTE_MAX];
	s32 result = OK;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]       |para[1]|para[2]   |para[3]|para[4]|para[5]|para[6]|
	 * |(len|datatype)|devaddr|regaddr0~3|data0  |data1  |data2  |data3  |
	 */
	len      = pmessage->paras[0] & 0xffff;
	datatype = (pmessage->paras[0] >> 16) & 0xffff;
	devaddr  = pmessage->paras[1];

	if ((len > RSB_TRANS_BYTE_MAX) || ((datatype !=  RSB_DATA_TYPE_BYTE) && (datatype !=  RSB_DATA_TYPE_HWORD) && (datatype !=  RSB_DATA_TYPE_WORD))) {
		WRN("rsb write reg paras error\n");
		return -EINVAL;
	}

	for (i = 0; i < len; i++)
	{
		regaddr[i] = ((pmessage->paras[2]) >> (i * 8)) & 0xff;
		data[i] = pmessage->paras[3 + i];
	}

	//write data one by one
	for (i = 0; i < len; i++)
	{
		result |= rsb_write(devaddr, regaddr[i], &(data[i]), datatype);
	}

	return result;
}

s32 rsb_bits_ops_sync(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u32 datatype = 0;
	u32 ops = 0;
	u32 devaddr;
	u8  regaddr[RSB_TRANS_BYTE_MAX];
	u32 mask[RSB_TRANS_BYTE_MAX];
	u8  delay[RSB_TRANS_BYTE_MAX];
	u32 data[RSB_TRANS_BYTE_MAX];

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]       |para[1]|para[2]   |para[3]|para[4]|para[5]|para[6]|para[7] |para[8]|
	 * |(len|datatype)|devaddr|regaddr0~3|mask0  |mask1  |mask2  |mask3  |delay0~3|ops    |
	 */
	len      = pmessage->paras[0] & 0xffff;
	datatype = (pmessage->paras[0] >> 16) & 0xffff;
	ops      = pmessage->paras[8];
	devaddr  = pmessage->paras[1];


	for (i = 0; i < len; i++)
	{
		regaddr[i] = ((pmessage->paras[2]) >> (i * 8)) & 0xff;
		mask[i]    = pmessage->paras[3 + i];
		delay[i]   = ((pmessage->paras[7]) >> (i * 8)) & 0xff;
	}

	//clear regs with interel delay
	for (i = 0; i < len; i++)
	{
		//read reg first
		rsb_read(devaddr, regaddr[i], &data[i], datatype);
		if (ops == RSB_CLR_BITS)
		{
			//clear target bits
			data[i] = data[i] & (~mask[i]);
		}
		else if (ops == RSB_SET_BITS)
		{
			//set target bits
			data[i] = data[i] | mask[i];
		}

		//writeback register value
		rsb_write(devaddr, regaddr[i], &data[i], datatype);
		if (delay[i])
		{
			//delay some time
			time_udelay(delay[i] * 1000);
		}
	}

	return 0;
}
#endif /* ((defined KERNEL_USED) || (defined TF_USED)) */

bool is_rsb_lock(void)
{
	return rsb_lock;
}
#endif  //RSB_USED
