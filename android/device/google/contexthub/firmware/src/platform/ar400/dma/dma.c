/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 DMA module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dma.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-7-2
* Descript: DMA controller driver.
* Update  : date                auther      ver     notes
*           2012-7-2 18:36:31   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "dma_i.h"

#if DMA_USED
/*
*********************************************************************************************************
*                                       DMA MEMORY COPY
*
* Description:  copy 'src' to 'dest' use DMA.
*
* Arguments  :  dest    : the dest buffer;
*               src     : the source buffer;
*               len     : the byte size of data to copy.
*
* Returns    :  OK if copy succeeded, others if failed.
*********************************************************************************************************
*/
s32 dma_memcpy(void *dest, void *src, u32 len)
{
	dma_cfg_reg_t   cfg;
	struct dma_des  des;
	u32             bit_pos;
	u32             channel;

	//config channel number
	channel = DMA_CH_NO;

	//set mbus clock to 24Mosc,because PLL6/PLL5 hasn't init here.
	writel(0x80000000, 0x1c2015c);
	writel(0x80000000, 0x1c20160);

	ccu_set_mclk_onoff(CCU_MOD_CLK_DMA, CCU_CLK_ON);
	ccu_set_mclk_onoff(CCU_MOD_CLK_SDRAM, CCU_CLK_ON);

	ccu_reset_module(CCU_MOD_CLK_DMA);

	//config dma
	cfg.bits.dest_width     = DMA_CFG_WID32;
	cfg.bits.dest_bst       = DMA_CFG_BST8;
	cfg.bits.dest_addr_mode = DMA_LINEAR_MODE;
	cfg.bits.src_width      = DMA_CFG_WID32;
	cfg.bits.src_bst        = DMA_CFG_BST8;
	cfg.bits.src_addr_mode  = DMA_LINEAR_MODE;

	if (((u32)(dest)) >= DRAM_BASE_ADDR)
	{
		//DRAM
		cfg.bits.dest_drq_type = DRQDST_SDRAM;
	}
	else
	{
		//SRAM
		cfg.bits.dest_drq_type = DRQDST_SRAM;
	}

	if (((u32)(src)) >= DRAM_BASE_ADDR)
	{
		//DRAM
		cfg.bits.src_drq_type = DRQSRC_SDRAM;
	}
	else
	{
		//SRAM
		cfg.bits.src_drq_type = DRQSRC_SRAM;
	}

	//config dma descriptor
	des.config = (u32)cfg.dwval;
	des.param  = (u32)0;
	des.saddr  = (u32)src;
	des.daddr  = (u32)dest;
	des.bcnt   = (u32)len;
	des.next   = (struct dma_des *)DMA_DES_NULL;

	if (channel < 8)
	{
		bit_pos = channel << 2;
	}
	else
	{
		bit_pos = (channel - 8) << 2;
	}

	//write dma descriptor to start address
	writel(((u32)(&des) + (u32)0x40000), DMA_DESADDR_REG(channel));

	//start dma
	writel(0x1, DMA_ENABLE_REG(channel));

	//wait dma finished
	while (1)
	{
		if ((readl(DMA_IRQ_PEND_REG(channel)) & (1 << (bit_pos + 2))))
		{
			volatile u32 pending;
			//queue transfer end pending ready.
			//clear transfer pending : queue transfer end +
			//package transfer end + half package transfer end.
			pending = 0x7 << bit_pos;
			writel(pending, DMA_IRQ_PEND_REG(channel));
			break;
		}
	}
	//stop dma
	writel(0, DMA_ENABLE_REG(channel));

	ccu_reset_module(CCU_MOD_CLK_DMA);

	ccu_set_mclk_onoff(CCU_MOD_CLK_DMA, CCU_CLK_OFF);

	return OK;
}
#endif
