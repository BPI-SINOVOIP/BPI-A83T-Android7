/*
******************************************************************************
*						        eBIOS
*           the Easy Portable/Player Develop Kits
*				           dma sub system
*
*	        (c) Copyright 2006-2008, David China
*							All	Rights Reserved
*
* File    : clk_for_nand.c
* By      : Richard
* Version : V1.00
*****************************************************************************
*/

#include "nand_osal_for_linux.h"

#define  NAND_DRV_VERSION_0		0x03
#define  NAND_DRV_VERSION_1		0x6006
#define  NAND_DRV_DATE			0x20170329
#define  NAND_DRV_TIME			0x1726


int NAND_Print(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk(fmt, args);
	va_end(args);

	return r;
}

int NAND_Print_DBG(const char *fmt, ...)
{

	va_list args;
	int r;

	if ((PRINT_LEVEL == 0) || (PRINT_LEVEL == 0xffffffff))
		return 0;
	else {
		va_start(args, fmt);
		r = vprintk(fmt, args);
		va_end(args);

		return r;
	}
}

int NAND_ClkRequest(__u32 nand_index)
{
	long rate;

	NAND_Print("NAND_ClkRequest\n");

	pll6 = of_clk_get(ndfc_dev->of_node, 0);
	if (NULL == pll6 || IS_ERR(pll6)) {
		NAND_Print("%s: pll6 clock handle invalid!\n", __func__);
		return -1;
	}

	rate = clk_get_rate(pll6);
	NAND_Print_DBG("%s: get pll6 rate %dHZ\n", __func__, (__u32) rate);

	if (nand_index == 0) {

		nand0_clk = of_clk_get(ndfc_dev->of_node, 1);

		if (NULL == nand0_clk || IS_ERR(nand0_clk)) {
			NAND_Print("%s: nand0 cclock handle invalid!\n",
				 __func__);
			return -1;
		}

		if (clk_set_parent(nand0_clk, pll6))
			NAND_Print("%s:set nand0_cclk parent to pll6 failed\n",
			       __func__);

		rate = clk_round_rate(nand0_clk, 20000000);
		if (clk_set_rate(nand0_clk, rate))
			NAND_Print("%s: set nand0_cclk rate to %dHZ failed!\n",
			       __func__, (__u32) rate);

		if (clk_prepare_enable(nand0_clk))
			NAND_Print("%s: enable nand0_cclk failed!\n",
			       __func__);
	} else {
		NAND_Print("NAND_ClkRequest, nand_index error: 0x%x\n",
		       nand_index);
		return -1;
	}

	return 0;
}

void NAND_ClkRelease(__u32 nand_index)
{
	if (nand_index == 0) {
		if (NULL != nand0_clk && !IS_ERR(nand0_clk)) {

			clk_disable_unprepare(nand0_clk);

			clk_put(nand0_clk);
			nand0_clk = NULL;
		}
	} else {
		NAND_Print("NAND_ClkRequest, nand_index error: 0x%x\n",
			nand_index);
	}

	if (NULL != pll6 && !IS_ERR(pll6)) {
		clk_put(pll6);
		pll6 = NULL;
	}
}

int NAND_SetClk(__u32 nand_index, __u32 nand_clk0, __u32 nand_clk1)
{
	long rate;

	if (nand_index == 0) {

		if (NULL == nand0_clk || IS_ERR(nand0_clk)) {
			NAND_Print("%s: clock handle invalid!\n",
			       __func__);
			return -1;
		}

		rate = clk_round_rate(nand0_clk, nand_clk0 * 2000000);
		if (clk_set_rate(nand0_clk, rate))
			NAND_Print("%s: set nand0_dclk to %dHZ failed! nand_clk: 0x%x\n",
			       __func__, (__u32) rate, nand_clk0);
	} else {
		NAND_Print("NAND_SetClk, nand_index error: 0x%x\n",
		       nand_index);
		return -1;
	}

	return 0;
}

int NAND_GetClk(__u32 nand_index, __u32 *pnand_clk0, __u32 *pnand_clk1)
{
	long rate;

	if (nand_index == 0) {
		if (NULL == nand0_clk || IS_ERR(nand0_clk)) {
			NAND_Print("%s: clock handle invalid!\n",
			       __func__);
			return -1;
		}
		rate = clk_get_rate(nand0_clk);
	} else {
		NAND_Print("NAND_GetClk, nand_index error: 0x%x\n",
		       nand_index);
		return -1;
	}

	*pnand_clk0 = (rate / 2000000);
	*pnand_clk1 = 0;

	return 0;
}

void eLIBs_CleanFlushDCacheRegion_nand(void *adr, size_t bytes)
{
/*  __flush_dcache_area(adr, bytes + (1 << 5) * 2 - 2);*/
}

__s32 NAND_CleanFlushDCacheRegion(void *buff_addr, __u32 len)
{
	eLIBs_CleanFlushDCacheRegion_nand((void *)buff_addr, (size_t) len);
	return 0;
}

__s32 NAND_InvaildDCacheRegion(__u32 rw, __u32 buff_addr, __u32 len)
{
	return 0;
}

void *NAND_DMASingleMap(__u32 rw, void *buff_addr, __u32 len)
{
	void *mem_addr;

	if (rw == 1)
		mem_addr = (void *)dma_map_single(ndfc_dev,
				(void *)buff_addr, len,
				DMA_TO_DEVICE);
	else
		mem_addr = (void *)dma_map_single(ndfc_dev,
				(void *)buff_addr, len,
				DMA_BIDIRECTIONAL);

	if (dma_mapping_error(ndfc_dev, (dma_addr_t) mem_addr))
		NAND_Print("dma mapping error\n");

	return mem_addr;
}

void *NAND_DMASingleUnmap(__u32 rw, void *buff_addr, __u32 len)
{
	void *mem_addr = buff_addr;
	if (rw == 1) {
		dma_unmap_single(ndfc_dev, (dma_addr_t) mem_addr, len,
				 DMA_TO_DEVICE);
	} else {
		dma_unmap_single(ndfc_dev, (dma_addr_t) mem_addr, len,
				 DMA_BIDIRECTIONAL);
	}

	return mem_addr;
}

void *NAND_VA_TO_PA(void *buff_addr)
{
	return (void *)(__pa((void *)buff_addr));
}

__s32 NAND_PIORequest(__u32 nand_index)
{
	struct pinctrl *pinctrl = NULL;

	PRINT_LEVEL = NAND_Print_level();

	pinctrl = pinctrl_get_select(ndfc_dev, "default");
	if (!pinctrl || IS_ERR(pinctrl)) {
		NAND_Print("NAND_PIORequest: set nand0 pin error!\n");
		return -1;
	}

	return 0;
}

void NAND_PIORelease(__u32 nand_index)
{

	struct pinctrl *pinctrl = NULL;

	pinctrl = pinctrl_get_select(ndfc_dev, "sleep");
	if (!pinctrl || IS_ERR(pinctrl))
		NAND_Print("NAND_PIORelease: set nand0 pin error!\n");

}

void NAND_Memset(void *pAddr, unsigned char value, unsigned int len)
{
	memset(pAddr, value, len);
}

void NAND_Memcpy(void *pAddr_dst, void *pAddr_src, unsigned int len)
{
	memcpy(pAddr_dst, pAddr_src, len);
}

void *NAND_Malloc(unsigned int Size)
{
	return kmalloc(Size, GFP_KERNEL);
}

void NAND_Free(void *pAddr, unsigned int Size)
{
	kfree(pAddr);
}

void *NAND_IORemap(void *base_addr, unsigned int size)
{
	return base_addr;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
DEFINE_SEMAPHORE(nand_physic_mutex);

int NAND_PhysicLockInit(void)
{
	return 0;
}

int NAND_PhysicLock(void)
{
	down(&nand_physic_mutex);
	return 0;
}

int NAND_PhysicUnLock(void)
{
	up(&nand_physic_mutex);
	return 0;
}

int NAND_PhysicLockExit(void)
{
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/

int nand_dma_config_start(__u32 rw, dma_addr_t addr, __u32 length)
{
	struct dma_slave_config dma_conf = { 0 };
	struct dma_async_tx_descriptor *dma_desc = NULL;

	dma_conf.direction = DMA_DEV_TO_MEM;
	dma_conf.src_addr = 0x01c03300;
	dma_conf.dst_addr = 0x01c03300;
	dma_conf.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dma_conf.src_maxburst = 1;
	dma_conf.dst_maxburst = 1;
	dma_conf.slave_id =
	    rw ? sunxi_slave_id(DRQDST_NAND0,
				DRQSRC_SDRAM) : sunxi_slave_id(DRQDST_SDRAM,
							       DRQSRC_NAND0);
	dmaengine_slave_config(dma_hdl, &dma_conf);

	dma_desc = dmaengine_prep_slave_single(dma_hdl, addr, length,
					       (rw ? DMA_TO_DEVICE :
						DMA_FROM_DEVICE),
					       DMA_PREP_INTERRUPT |
					       DMA_CTRL_ACK);
	if (!dma_desc) {
		NAND_Print("dmaengine prepare failed!\n");
		return -1;
	}

	dma_desc->callback = (void *)nand_dma_callback;
	if (rw == 0)
		dma_desc->callback_param = NULL;
	else
		dma_desc->callback_param = (void *)(dma_desc);

	dmaengine_submit(dma_desc);

	dma_async_issue_pending(dma_hdl);

	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
__u32 NAND_GetNdfcDmaMode(void)
{
	/*
	   0: General DMA;
	   1: MBUS DMA

	   Only support MBUS DMA!!!!
	 */
	return 1;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
__u32 NAND_GetNandExtPara(__u32 para_num)
{
	int ret = 0;
	int nand_para = 0xffffffff;

	if (para_num == 0) {	/*frequency */
		ret = of_property_read_u32(ndfc_dev->of_node, "nand0_p0",
					 &nand_para);
		if (ret) {
			NAND_Print("Failed to get nand_p0\n");
			return 0xffffffff;
		} else {
			if (nand_para == 0x55aaaa55) {
				NAND_Print("nand_p0 is no used\n");
				nand_para = 0xffffffff;
			} else
				NAND_Print("nand : get nand_p0 %x\n",
				       nand_para);
		}
	} else if (para_num == 1) {	/*SUPPORT_TWO_PLANE */
		ret = of_property_read_u32(ndfc_dev->of_node, "nand0_p1",
					 &nand_para);
		if (ret) {
			NAND_Print("Failed to get nand_p1\n");
			return 0xffffffff;
		} else {
			if (nand_para == 0x55aaaa55) {
				NAND_Print("nand_p1 is no used\n");
				nand_para = 0xffffffff;
			} else
				NAND_Print("nand : get nand_p1 %x\n",
				       nand_para);
		}
	} else if (para_num == 2) {	/*SUPPORT_VERTICAL_INTERLEAVE */
		ret = of_property_read_u32(ndfc_dev->of_node, "nand0_p2",
					 &nand_para);
		if (ret) {
			NAND_Print("Failed to get nand_p2\n");
			return 0xffffffff;
		} else {
			if (nand_para == 0x55aaaa55) {
				NAND_Print("nand_p2 is no used\n");
				nand_para = 0xffffffff;
			} else
				NAND_Print("nand : get nand_p2 %x\n",
				       nand_para);
		}
	} else if (para_num == 3) {	/*SUPPORT_DUAL_CHANNEL */
		ret = of_property_read_u32(ndfc_dev->of_node, "nand0_p3",
					 &nand_para);
		if (ret) {
			NAND_Print("Failed to get nand_p3\n");
			return 0xffffffff;
		} else {
			if (nand_para == 0x55aaaa55) {
				NAND_Print("nand_p3 is no used\n");
				nand_para = 0xffffffff;
			} else
				NAND_Print("nand : get nand_p3 %x\n",
				       nand_para);
		}
	} else {
		NAND_Print("NAND_GetNandExtPara: wrong para num: %d\n",
		       para_num);
		return 0xffffffff;
	}
	return nand_para;
}

__u32 NAND_GetNandIDNumCtrl(void)
{
	int ret;
	int id_number_ctl = 0;

	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_id_number_ctl",
				 &id_number_ctl);
	if (ret) {
		NAND_Print_DBG("Failed to get id_number_ctl\n");
		id_number_ctl = 0;
	} else {
		if (id_number_ctl == 0x55aaaa55) {
			NAND_Print_DBG("id_number_ctl is no used\n");
			id_number_ctl = 0;
		} else
			NAND_Print_DBG("nand : get id_number_ctl %x\n",
				       id_number_ctl);
	}
	return id_number_ctl;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
__u32 NAND_GetMaxChannelCnt(void)
{
	return 1;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_request_dma(void)
{
	dma_cap_mask_t mask;

	NAND_Print("request DMA");

	/* Try to acquire a generic DMA engine slave channel */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	if (dma_hdl == NULL) {
		dma_hdl = dma_request_channel(mask, NULL, NULL);
		if (dma_hdl == NULL) {
			NAND_Print("Request DMA failed!\n");
			return -EINVAL;
		}
	}
	NAND_Print("chan_id: %d", dma_hdl->chan_id);

	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int NAND_ReleaseDMA(__u32 nand_index)
{
	if (dma_hdl != NULL) {
		NAND_Print("nand release dma\n");
		dma_release_channel(dma_hdl);
		dma_hdl = NULL;
		return 0;
	}
	return 0;

}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
__u32 NAND_GetNdfcVersion(void)
{
	/*
	   0:
	   1: A31/A31s/A21/A23
	   2:
	 */
	return 1;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void *NAND_GetIOBaseAddrCH0(void)
{
	return (void *)0xf1c03000;
}

void *NAND_GetIOBaseAddrCH1(void)
{
	return (void *)0xf1c05000;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         : wait rb
*****************************************************************************/
static DECLARE_WAIT_QUEUE_HEAD(NAND_RB_WAIT_CH0);
static DECLARE_WAIT_QUEUE_HEAD(NAND_RB_WAIT_CH1);

__s32 nand_rb_wait_time_out(__u32 no, __u32 *flag)
{
	__s32 ret;
	if (no == 0)
		ret = wait_event_timeout(NAND_RB_WAIT_CH0, *flag, HZ >> 1);
	else
		ret = wait_event_timeout(NAND_RB_WAIT_CH1, *flag, HZ >> 1);

	return ret;
}

__s32 nand_rb_wake_up(__u32 no)
{
	if (no == 0)
		wake_up(&NAND_RB_WAIT_CH0);
	else
		wake_up(&NAND_RB_WAIT_CH1);

	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         : wait dma
*****************************************************************************/
static DECLARE_WAIT_QUEUE_HEAD(NAND_DMA_WAIT_CH0);
static DECLARE_WAIT_QUEUE_HEAD(NAND_DMA_WAIT_CH1);

__s32 nand_dma_wait_time_out(__u32 no, __u32 *flag)
{
	__s32 ret;
	if (no == 0)
		ret = wait_event_timeout(NAND_DMA_WAIT_CH0, *flag, HZ >> 1);
	else
		ret = wait_event_timeout(NAND_DMA_WAIT_CH1, *flag, HZ >> 1);

	return ret;
}

__s32 nand_dma_wake_up(__u32 no)
{
	if (no == 0)
		wake_up(&NAND_DMA_WAIT_CH0);
	else
		wake_up(&NAND_DMA_WAIT_CH1);

	return 0;
}

__u32 nand_dma_callback(void *para)
{
	return 0;
}

int NAND_get_storagetype(void)
{
	return 0;
}

int NAND_GetVoltage(void)
{

	int ret = 0;
	const char *sti_vcc_nand = NULL;
	const char *sti_vcc_io = NULL;

	ret = of_property_read_string(ndfc_dev->of_node, "nand0_regulator1",
				    &sti_vcc_nand);
	NAND_Print("nand0_regulator1 %s\n", sti_vcc_nand);
	if (ret)
		NAND_Print("Failed to get vcc_nand\n");

	regu1 = regulator_get(NULL, sti_vcc_nand);
	if (IS_ERR(regu1))
		NAND_Print("nand:fail to get regulator vcc-nand!\n");
	else {
		/*enable regulator */
		ret = regulator_enable(regu1);
		if (IS_ERR(regu1)) {
			NAND_Print
			    ("fail to enable regulator vcc-nand!\n");
			return -1;
		}
		NAND_Print_DBG("nand:get voltage vcc-nand ok:%p\n", regu1);
	}

	ret = of_property_read_string(ndfc_dev->of_node, "nand0_regulator2",
				    &sti_vcc_io);
	NAND_Print("nand0_regulator2 %s\n", sti_vcc_io);
	if (ret)
		NAND_Print("Failed to get vcc_io\n");

	regu2 = regulator_get(NULL, sti_vcc_io);
	if (IS_ERR(regu2))
		NAND_Print("nand:fail to get regulator vcc-io!\n");
	else {
		/*enable regulator */
		ret = regulator_enable(regu2);
		if (IS_ERR(regu2)) {
			NAND_Print
			    ("fail to enable regulator vcc-io!\n");
			return -1;
		}
		NAND_Print_DBG("nand:get voltage vcc-io ok:%p\n", regu2);
	}

	NAND_Print("nand:has already get voltage\n");

	return ret;
}

int NAND_ReleaseVoltage(void)
{
	int ret = 0;

	if (!IS_ERR(regu1)) {
		NAND_Print("nand release voltage vcc-nand\n");
		ret = regulator_disable(regu1);
		if (ret)
			NAND_Print("regulator disable fail ret is %x\n", ret);
		if (IS_ERR(regu1))
			NAND_Print("fail to disable regulator vcc-nand!");

		/*put regulator when module exit */
		regulator_put(regu1);

		regu1 = NULL;
	}

	if (!IS_ERR(regu2)) {
		NAND_Print("nand release voltage vcc-io\n");
		ret = regulator_disable(regu2);
		if (ret)
			NAND_Print("regulator disable fail ret=%x\n", ret);
		if (IS_ERR(regu2))
			NAND_Print("fail to disable regulator vcc-io!");

		/*put regulator when module exit */
		regulator_put(regu2);

		regu2 = NULL;
	}

	NAND_Print("nand had already release voltage\n");

	return ret;

}

int NAND_IS_Secure_sys(void)
{
	if (sunxi_soc_is_secure()) {
		NAND_Print("secure system\n");
		return 1;
	} else {
		NAND_Print("non secure\n");
		return 0;
	}
}

__u32 NAND_Print_level(void)
{
	int ret;
	int print_level = 0xffffffff;

	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_print_level",
				 &print_level);
	if (ret) {
		NAND_Print("Failed to get print_level\n");
		print_level = 0xffffffff;
	} else {
		if (print_level == 0x55aaaa55) {
			NAND_Print("print_level is no used\n");
			print_level = 0xffffffff;
		} else
			NAND_Print("nand : get print_level %x\n", print_level);
	}

	return print_level;
}

int NAND_Get_Dragonboard_Flag(void)
{
	int ret;
	int dragonboard_flag = 0;

	ret = of_property_read_u32(ndfc_dev->of_node, "nand0_dragonboard",
				 &dragonboard_flag);
	if (ret) {
		NAND_Print("Failed to get dragonboard_flag\n");
		dragonboard_flag = 0;
	} else {
		NAND_Print("nand: dragonboard_flag %x\n", dragonboard_flag);
	}

	return dragonboard_flag;
}

void NAND_Print_Version(void)
{
	int val[4] = { 0 };

	val[0] = NAND_DRV_VERSION_0;
	val[1] = NAND_DRV_VERSION_1;
	val[2] = NAND_DRV_DATE;
	val[3] = NAND_DRV_TIME;

	NAND_Print("kernel: nand version: %x %x %x %x\n", val[0],
	       val[1], val[2], val[3]);
}

int NAND_Get_Version(void)
{
	return NAND_DRV_DATE;
}

void Dump_Gpio_Reg_Show(void)
{
	void __iomem *gpio_ptr = ioremap(GPIO_BASE_ADDR, 0x300);

	NAND_Print("Reg 0x01c20848: 0x%x\n",
	       *((__u32 *)gpio_ptr + 18));
	NAND_Print("Reg 0x01c2084c: 0x%x\n",
	       *((__u32 *)gpio_ptr + 19));
	NAND_Print("Reg 0x01c20850: 0x%x\n",
	       *((__u32 *)gpio_ptr + 20));
	NAND_Print("Reg 0x01c2085c: 0x%x\n",
	       *((__u32 *)gpio_ptr + 23));
	NAND_Print("Reg 0x01c20864: 0x%x\n",
	       *((__u32 *)gpio_ptr + 24));
	NAND_Print("Reg 0x01c20864: 0x%x\n",
	       *((__u32 *)gpio_ptr + 25));
	NAND_Print("Reg 0x01c20864: 0x%x\n",
	       *((__u32 *)gpio_ptr + 26));
	iounmap(gpio_ptr);

}

void Dump_Ccmu_Reg_Show(void)
{
	void __iomem *ccmu_ptr = ioremap(CCMU_BASE_ADDR, 0x300);

	NAND_Print("Reg 0x01c20028: 0x%x\n",
	       *((__u32 *)ccmu_ptr + 10));
	NAND_Print("Reg 0x01c20080: 0x%x\n", *((__u32 *)ccmu_ptr + 32));

	iounmap(ccmu_ptr);

}

void nand_cond_resched(void)
{
	cond_resched();
}
