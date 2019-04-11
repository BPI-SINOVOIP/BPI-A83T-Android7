/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 p2wi module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : p2wi.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: p2wi module.
* Update  : date                auther      ver     notes
*           2012-5-22 9:49:38   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "p2wi_i.h"

//p2wi can be config used or not
#if  P2WI_USED

volatile u32 p2wi_lock;

s32 p2wi_init(void)
{
#ifdef  FPGA_PLATFORM
	//PH[14][15] used as p2wi SCK and SDA under FPGA platform.
	writel(0x33333333, 0x01c20900);
#endif

	//initialize p2wi sck and sda pins
	pin_set_multi_sel(PIN_GRP_PL, 0, 3);                //PL0 config as P2WI_SCK
	pin_set_pull     (PIN_GRP_PL, 0, PIN_PULL_UP);      //P2WI_SCK pull-up
	pin_set_drive    (PIN_GRP_PL, 0, PIN_MULTI_DRIVE_2);//P2WI_SCK drive level 2
	pin_set_multi_sel(PIN_GRP_PL, 1, 3);                //PL1 config as P2WI_SDA
	pin_set_pull     (PIN_GRP_PL, 1, PIN_PULL_UP);      //P2WI_SDA pull-up
	pin_set_drive    (PIN_GRP_PL, 1, PIN_MULTI_DRIVE_2);//P2WI_SDA drive level 2

	//enbale p2wi clock, set reset as de-assert state.
	ccu_set_mclk_onoff(CCU_MOD_CLK_APB0_R_P2WI, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_APB0_R_P2WI, CCU_CLK_NRESET);

	//reset p2wi controller
	writel(P2WI_SOFT_RST, P2WI_REG_CTRL);

	//set p2wi clock
	p2wi_set_clk(P2WI_SCK_FREQ);

	//pmu initialize as p2wi mode
	p2wi_set_pmu_mode(0x68, 0x3e, 0x3e);

	//register uart clock change notifier
	ccu_reg_mclk_cb(CCU_MOD_CLK_APB0_R_P2WI, p2wi_clkchangecb);

	//ensure p2wi is unlock
	p2wi_lock = 0;

	return OK;
}

s32 p2wi_exit(void)
{
	return OK;
}

s32 p2wi_read(u8 *addr, u8 *data, u32 len)
{
	u32 i;
	u32 addr0 = 0, addr1 = 0;
	u32 data0 = 0, data1 = 0;

	ASSERT((len < PMU_TRANS_BYTE_MAX) && (len > 0));

	if (p2wi_lock)
	{
		return -EACCES;
	}

	for (i = 0; i < len; i++)
	{
		//INF("read data[%d] : addr = %x\n", i, addr[i]);
		if (i < 4)
		{
			//pack 8bit addr0~addr3 into 32bit addr0
			addr0 |= addr[i] << (i * 8);
		}
		else
		{
			//pack 8bit addr4~addr7 into 32bit addr1
			addr1 |= addr[i] << ((i - 4) * 8);
		}
	}

	//write address to register
	writel(addr0, P2WI_REG_DADDR0);
	writel(addr1, P2WI_REG_DADDR1);
	writel((len - 1) | (1 << 4), P2WI_REG_DLEN);

	//start transmit
	writel(readl(P2WI_REG_CTRL) | P2WI_START_TRANS, P2WI_REG_CTRL);

	//INF("read config finished\n");
	//INF("P2WI_REG_DADDR0 = %x\n", readl(P2WI_REG_DADDR0));
	//INF("P2WI_REG_DADDR1 = %x\n", readl(P2WI_REG_DADDR1));
	//INF("P2WI_REG_DLEN   = %x\n", readl(P2WI_REG_DLEN));
	//INF("P2WI_REG_CTRL   = %x\n", readl(P2WI_REG_CTRL));
	//INF("P2WI_REG_CCR    = %x\n", readl(P2WI_REG_CCR));

	//wait transfer complete
	if (p2wi_wait_state() != OK)
	{
		ERR("p2wi read failed\n");
		return -EFAIL;
	}

	//read out data
	data0 = readl(P2WI_REG_DATA0);
	data1 = readl(P2WI_REG_DATA1);

	//INF("P2WI_REG_DATA0   = %x\n", data0);
	//INF("P2WI_REG_DATA1   = %x\n", data1);

	for (i = 0; i < len; i++)
	{
		if (i < 4)
		{
			data[i] = (data0 >> (i * 8)) & 0xff;
		}
		else
		{
			data[i] = (data1 >> ((i - 4) * 8)) & 0xff;
		}
	}

	return OK;
}

s32 p2wi_write(u8 *addr, u8 *data, u32 len)
{
	u32 i;
	u32 addr0 = 0, addr1 = 0;
	u32 data0 = 0, data1 = 0;

	ASSERT((len < PMU_TRANS_BYTE_MAX) && (len > 0));

	if (p2wi_lock)
	{
		return -EACCES;
	}

	for (i = 0; i < len; i++)
	{
		//INF("write data[%d] : data = %x, addr = %x\n", i, data[i], addr[i]);
		if (i < 4)
		{
			//pack 8bit data0~data3 into 32bit data0
			//pack 8bit addr0~addr3 into 32bit addr0
			addr0 |= addr[i] << (i * 8);
			data0 |= data[i] << (i * 8);
		}
		else
		{
			//pack 8bit data4~data7 into 32bit data1
			//pack 8bit addr4~addr7 into 32bit addr4
			addr1 |= addr[i] << ((i - 4) * 8);
			data1 |= data[i] << ((i - 4) * 8);
		}
	}

	//write register
	writel(addr0, P2WI_REG_DADDR0);
	writel(addr1, P2WI_REG_DADDR1);
	writel(data0, P2WI_REG_DATA0);
	writel(data1, P2WI_REG_DATA1);
	writel(len - 1, P2WI_REG_DLEN);

	//start transfer
	writel(readl(P2WI_REG_CTRL) | P2WI_START_TRANS, P2WI_REG_CTRL);

	//INF("write config finished\n");
	//INF("P2WI_REG_DADDR0 = %x\n", readl(P2WI_REG_DADDR0));
	//INF("P2WI_REG_DADDR1 = %x\n", readl(P2WI_REG_DADDR1));
	//INF("P2WI_REG_DATA0  = %x\n", readl(P2WI_REG_DATA0));
	//INF("P2WI_REG_DATA1  = %x\n", readl(P2WI_REG_DATA1));
	//INF("P2WI_REG_DLEN   = %x\n", readl(P2WI_REG_DLEN));
	//INF("P2WI_REG_CTRL   = %x\n", readl(P2WI_REG_CTRL));

	//wait transfer complete
	if (p2wi_wait_state() != OK)
	{
		ERR("p2wi write failed\n");
		return -EFAIL;
	}

	return OK;
}

void p2wi_set_pmu_mode(u32 slave_addr, u32 reg, u32 data)
{
	//set pmu work mode
	writel(P2WI_PMU_INIT | (slave_addr) | (reg << 8) | (data << 16), P2WI_REG_PMCR);

	//wait pmu mode set complete
	while(readl(P2WI_REG_PMCR) & P2WI_PMU_INIT)
	{
		;
	}
}

void p2wi_set_clk(u32 sck)
{
#ifndef FPGA_PLATFORM
	u32 src_clk = ccu_get_sclk_freq(CCU_SYS_CLK_APB0);
	u32 div = src_clk / sck / 2 - 1;
	u32 sda_odly = div >> 1;
	u32 rval = div | (sda_odly << 8);
	writel(rval, P2WI_REG_CCR);
#else
	//fpga platmform: apb clock = 24M, p2wi clock = 2M.
	//0x201->6M
	//0x202->4M
	//0x203->3M
	//0x204->2.4M
	//0x205->2M
	//0x206->1.7M
	//0x205 = 0x200 + 0x005,the 0x200 is SDA output delay.
	writel(0x205, P2WI_REG_CCR);
	//INF("P2WI_REG_CCR value = %x\n", readl(P2WI_REG_CCR));
#endif
}

s32 p2wi_wait_state(void)
{
	s32  ret = FAIL;
	u32  stat;
	while (1)
	{
		stat = readl(P2WI_REG_STAT);
		if (stat & P2WI_TERR_INT)
		{
			//transfer error
			ERR("p2wi transfer error [%x]\n", ((stat >> 8) & 0xff));
			ret = -EFAIL;
			break;
		}
		if (stat & P2WI_TOVER_INT)
		{
			//transfer complete
			ret = OK;
			break;
		}
	}
	//clear state flag
	writel(stat, P2WI_REG_STAT);

	return ret;
}

s32 p2wi_clkchangecb(u32 command, u32 freq)
{
	switch(command)
	{
		case CCU_CLK_CLKCHG_REQ:
		{
			//check p2wi is busy now
			//...

			//clock will be change, lock p2wi interface
			p2wi_lock = 1;
			INF("p2wi clk change request\n");
			return OK;
		}
		case CCU_CLK_CLKCHG_DONE:
		{
			//clock change finish, re-config p2wi clock,
			//maybe re-config p2wi clock should do somethings???
#ifndef     FPGA_PLATFORM
			u32 div;
			u32 sda_odly;
			u32 rval;
			if (freq >= P2WI_SCK_FREQ)
			{
				div = freq / P2WI_SCK_FREQ / 2 - 1;
				sda_odly = div >> 1;
			}
			else
			{
				div = 0;
				sda_odly = 0;
			}
			rval = div | (sda_odly << 8);
			writel(rval, P2WI_REG_CCR);
#endif
			//unlock p2wi interface
			p2wi_lock = 0;
			INF("p2wi clk change done\n");
			return OK;
		}
		default:
		{
			break;
		}
	}
	return -ESRCH;
}

s32 p2wi_read_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u8  addr[P2WI_TRANS_BYTE_MAX];
	u8  data[P2WI_TRANS_BYTE_MAX];
	s32 result = OK;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]|para[1]|para[2]|para[3]|para[4]|
	 * |len    |addr0~3|addr4~7|data0~3|data4~7|
	 */
	len = pmessage->paras[0];

	for (i = 0; i < len; i++)
	{
		if (i < 4)
		{
			addr[i] = ((pmessage->paras[1]) >> (i * 8)) & 0xff;
		}
		else
		{
			addr[i] = ((pmessage->paras[2]) >> ((i - 4) * 8)) & 0xff;
		}
	}

	result = p2wi_read(addr, data, len);

	//copy readout data to pmessage->paras
	for (i = 0; i < len; i++)
	{
		if (i < 4)
		{
			//pack 8bit data0~data3 into 32bit pmessage->paras[3]
			pmessage->paras[3] |= (data[i] << (i * 8));
		}
		else
		{
			//pack 8bit data4~data7 into 32bit pmessage->paras[4]
			pmessage->paras[4] |= (data[i] << ((i - 4) * 8));
		}
	}

	return result;
}

s32 p2wi_write_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u8  addr[P2WI_TRANS_BYTE_MAX];
	u8  data[P2WI_TRANS_BYTE_MAX];
	s32 result = OK;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]|para[1]|para[2]|para[3]|para[4]|
	 * |len    |addr0~3|addr4~7|data0~3|data4~7|
	 */
	len = pmessage->paras[0];

	for (i = 0; i < len; i++)
	{
		if (i < 4)
		{
			addr[i] = ((pmessage->paras[1]) >> (i * 8)) & 0xff;
			data[i] = ((pmessage->paras[3]) >> (i * 8)) & 0xff;
		}
		else
		{
			addr[i] = ((pmessage->paras[2]) >> ((i - 4) * 8)) & 0xff;
			data[i] = ((pmessage->paras[4]) >> ((i - 4) * 8)) & 0xff;
		}
	}


	result = p2wi_write(addr, data, len);

	return result;
}

s32 p2wi_bits_ops_sync(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u32 ops = 0;
	u8  addr[P2WI_TRANS_BYTE_MAX];
	u8  mask[P2WI_TRANS_BYTE_MAX];
	u8  delay[P2WI_TRANS_BYTE_MAX];
	u8  data[P2WI_TRANS_BYTE_MAX];
	s32 result = OK;

	/*
	 * package address and data to message->paras,
	 * message->paras data layout:
	 * |para[0]|para[1]|para[2]|para[3]|para[4]|para[5] |para[6] |para[7]|
	 * |len    |addr0~3|addr4~7|mask0~3|mask4~7|delay0~3|delay4~7|ops    |
	 */
	len = pmessage->paras[0];
	ops = pmessage->paras[7];

	for (i = 0; i < len; i++)
	{
		if (i < 4)
		{
			addr[i]  = ((pmessage->paras[1]) >> (i * 8)) & 0xff;
			mask[i]  = ((pmessage->paras[3]) >> (i * 8)) & 0xff;
			delay[i] = ((pmessage->paras[5]) >> (i * 8)) & 0xff;
		}
		else
		{
			addr[i]  = ((pmessage->paras[2]) >> ((i - 4) * 8)) & 0xff;
			mask[i]  = ((pmessage->paras[4]) >> ((i - 4) * 8)) & 0xff;
			delay[i] = ((pmessage->paras[6]) >> ((i - 4) * 8)) & 0xff;
		}
	}

	//clear regs with interel delay
	for (i = 0; i < len; i++)
	{
		//read pmu reg first
		p2wi_read(&addr[i], &data[i], 1);
		if (ops == P2WI_CLR_BITS)
		{
			//clear target bits
			data[i] = data[i] & (~mask[i]);
		}
		else if (ops == P2WI_CLR_BITS)
		{
			//set target bits
			data[i] = data[i] | mask[i];
		}

		//writeback register value
		result = p2wi_write(addr, data, len);
		if (delay[i])
		{
			//delay some time
			time_udelay(delay[i] * 1000);
		}
	}

	return result;
}

#endif  //P2WI_USED
