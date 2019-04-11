/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 DMA module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dma_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-7-2
* Descript: DMA controller internal header.
* Update  : date                auther      ver     notes
*           2012-7-2 18:37:17   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DMA_I_H__
#define __DMA_I_H__

#include <plat/inc/include.h>

//cpus use chanel number for memcopy
#define DMA_CH_NO   (15)

//DMA registers list
#define DMA_BASE                (0x01c02000)
#define DMA_IRQ_EN_REG(ch)      (DMA_BASE + 0x0    + ((ch) >> 3) * 0x4)
#define DMA_IRQ_PEND_REG(ch)    (DMA_BASE + 0x10   + ((ch) >> 3) * 0x4)
#define DMA_CHAN_STA_REG        (DMA_BASE + 0x30)
#define DMA_ENABLE_REG(ch)      (DMA_BASE + 0x0100 + ((ch) << 6) + 0x00)
#define DMA_PAUSE_REG(ch)       (DMA_BASE + 0x0100 + ((ch) << 6) + 0x04)
#define DMA_DESADDR_REG(ch)     (DMA_BASE + 0x0100 + ((ch) << 6) + 0x08)
#define DMA_CONFIGR_REG(ch)     (DMA_BASE + 0x0100 + ((ch) << 6) + 0x0C)
#define DMA_CUR_SADDR_REG(ch)   (DMA_BASE + 0x0100 + ((ch) << 6) + 0x10)
#define DMA_CUR_DADDR_REG(ch)   (DMA_BASE + 0x0100 + ((ch) << 6) + 0x14)
#define DMA_LEFTCNT_REG(ch)     (DMA_BASE + 0x0100 + ((ch) << 6) + 0x18)
#define DMA_PARAM_REG(ch)       (DMA_BASE + 0x0100 + ((ch) << 6) + 0x1C)
#define DMA_LOCKFLG_REG(ch)     (DMA_BASE + 0x0100 + ((ch) << 6) + 0x20)
#define DMA_NULLFLG_REG(ch)     (DMA_BASE + 0x0100 + ((ch) << 6) + 0x24)

//DMA descriptor structure
struct dma_des
{
	volatile u32              config;
	volatile u32              saddr;
	volatile u32              daddr;
	volatile u32              bcnt;
	volatile u32              param;
	volatile struct dma_des  *next;
};

//dma config burst length
enum dma_cfg_burst
{
	DMA_CFG_BST1,
	DMA_CFG_BST4,
	DMA_CFG_BST8
};

//dma config width length
enum dma_cfg_width
{
	DMA_CFG_WID8,
	DMA_CFG_WID16,
	DMA_CFG_WID32
};

//dma source ot dest address mode
enum dma_addr_mode
{
	DMA_LINEAR_MODE,
	DMA_IO_MODE
};

// source drq type
enum drqsrc_type_e
{
	DRQSRC_SRAM         = 0,
	DRQSRC_SDRAM        = 1,
	DRQSRC_SPDIFRX      = 2,
	DRQSRC_DAUDIO_0_RX  = 3,
	DRQSRC_DAUDIO_1_RX  = 4,
	DRQSRC_NAND0        = 5,
	DRQSRC_UART0RX      = 6,
	DRQSRC_UART1RX      = 7,
	DRQSRC_UART2RX      = 8,
	DRQSRC_UART3RX      = 9,
	DRQSRC_UART4RX      = 10,
	DRQSRC_HDMI_DDC     = 13,
	DRQSRC_HDMI_AUDIO   = 14,
	DRQSRC_AUDIO_CODEC  = 15,
	DRQSRC_SS_RX        = 16,
	DRQSRC_OTG_EP1      = 17,
	DRQSRC_OTG_EP2      = 18,
	DRQSRC_OTG_EP3      = 19,
	DRQSRC_OTG_EP4      = 20,
	DRQSRC_OTG_EP5      = 21,
	DRQSRC_UART5RX      = 22,
	DRQSRC_SPI0RX       = 23,
	DRQSRC_SPI1RX       = 24,
	DRQSRC_SPI2RX       = 25,
	DRQSRC_SPI3RX       = 26,
	DRQSRC_TP           = 27,
	DRQSRC_NAND1        = 28,
	DRQSRC_MTC_ACC      = 29,
	DRQSRC_DIGITAL_MIC  = 30
};

//dest drq type
enum drqdst_type_e
{
	DRQDST_SRAM         = 0,
	DRQDST_SDRAM        = 1,
	DRQDST_SPDIFTX      = 2,
	DRQDST_DAUDIO_0_TX  = 3,
	DRQDST_DAUDIO_1_TX  = 4,
	DRQDST_NAND0        = 5,
	DRQDST_UART0TX      = 6,
	DRQDST_UART1TX      = 7,
	DRQDST_UART2TX      = 8,
	DRQDST_UART3TX      = 9,
	DRQDST_UART4TX      = 10,
	DRQDST_TCON0        = 11,
	DRQDST_TCON1        = 12,
	DRQDST_HDMI_DDC     = 13,
	DRQDST_HDMI_AUDIO   = 14,
	DRQDST_AUDIO_CODEC  = 15,
	DRQDST_SS_TX        = 16,
	DRQDST_OTG_EP1      = 17,
	DRQDST_OTG_EP2      = 18,
	DRQDST_OTG_EP3      = 19,
	DRQDST_OTG_EP4      = 20,
	DRQDST_OTG_EP5      = 21,
	DRQDST_UART5TX      = 22,
	DRQDST_SPI0TX       = 23,
	DRQDST_SPI1TX       = 24,
	DRQDST_SPI2TX       = 25,
	DRQDST_SPI3TX       = 26,
	DRQDST_NAND1        = 28,
	DRQDST_MTC_ACC      = 29,
	DRQDST_DIGITAL_MIC  = 30
};

//dma config register
typedef union
{
	u32 dwval;
	struct
	{
		u32 reserved0       :   5;      //bit27, reserved
		u32 dest_width      :   2;      //bit25, dma destintion data width
		u32 dest_bst        :   2;      //bit23, dma destintion burst length
		u32 dest_addr_mode  :   2;      //bit21, dma destintion address mode
		u32 dest_drq_type   :   5;      //bit16, dma destintion DRQ type
		u32 reserved1       :   5;      //bit11, reserved0
		u32 src_width       :   2;      //bit9,  dma source data width
		u32 src_bst         :   2;      //bit7,  dma source burst length
		u32 src_addr_mode   :   2;      //bit5,  dma source address mode
		u32 src_drq_type    :   5;      //bit0,  dma source DRQ type
	} bits;
} dma_cfg_reg_t;

//the DMA NULL descriptor
#define DMA_DES_NULL    (0xfffff800)

#endif  //__DMA_I_H__
