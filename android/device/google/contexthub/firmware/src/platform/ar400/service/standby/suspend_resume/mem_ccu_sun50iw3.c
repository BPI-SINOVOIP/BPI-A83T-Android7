#include <plat/inc/include.h>
#if MEM_USED
#if defined CONFIG_ARCH_SUN50IW3P1
#include "mem_ccu_sun50iw3.h"

#define CCM_REG_BASE system_back->ccm_back.ccm_reg_base
#define CCM_REG_BACK system_back->ccm_back.ccm_reg_back

int mem_ccu_save(void)
{
	ASSERT(&CCM_REG_BASE->PllLockCtrl != SUNXI_CCM_PEND);

	system_back->ccm_back.ccm_reg_base = (__ccmu_reg_list_t *)IO_ADDRESS(SUNXI_CCM_PBASE);

	//ctrl:
	CCM_REG_BACK.PllC0Ctl			= CCM_REG_BASE->PllC0Ctl;
//	CCM_REG_BACK.PllDdr0Ctl			= CCM_REG_BASE->PllDdr0Ctl;
	CCM_REG_BACK.PllPeriph0Ctl     	 	= CCM_REG_BASE->PllPeriph0Ctl;
	CCM_REG_BACK.PllPeriph1Ctl      	= CCM_REG_BASE->PllPeriph1Ctl;
	CCM_REG_BACK.PllGpuCtl      		= CCM_REG_BASE->PllGpuCtl;
	CCM_REG_BACK.PllVideo0Ctl      		= CCM_REG_BASE->PllVideo0Ctl;
	CCM_REG_BACK.PllVideo1Ctl	 	= CCM_REG_BASE->PllVideo1Ctl;
	CCM_REG_BACK.PllVeCtl      		= CCM_REG_BASE->PllVeCtl;
	CCM_REG_BACK.PllDeCtl      		= CCM_REG_BASE->PllDeCtl;
	//CCM_REG_BACK.PllHsicCtl			= CCM_REG_BASE->PllHsicCtl;
	CCM_REG_BACK.PllAudioCtl		= CCM_REG_BASE->PllAudioCtl;
		//pat:
//	CCM_REG_BACK.PllDdr0RegPattern		= CCM_REG_BASE->PllDdr0RegPattern;
	CCM_REG_BACK.PllPeri1Pattern0		= CCM_REG_BASE->PllPeri1Pattern0;
	CCM_REG_BACK.PllPeri1Pattern1		= CCM_REG_BASE->PllPeri1Pattern1;
	CCM_REG_BACK.PllGpuRegPattern0		= CCM_REG_BASE->PllGpuRegPattern0;
	CCM_REG_BACK.PllGpuRegPattern1		= CCM_REG_BASE->PllGpuRegPattern1;
	CCM_REG_BACK.PllVideo0RegPattern0	= CCM_REG_BASE->PllVideo0RegPattern0;
	CCM_REG_BACK.PllVideo0RegPattern1	= CCM_REG_BASE->PllVideo0RegPattern1;
	CCM_REG_BACK.PllVideo1RegPattern0	= CCM_REG_BASE->PllVideo1RegPattern0;
	CCM_REG_BACK.PllVideo1RegPattern1	= CCM_REG_BASE->PllVideo1RegPattern1;
	CCM_REG_BACK.PllVeRegPattern0		= CCM_REG_BASE->PllVeRegPattern0;
	CCM_REG_BACK.PllVeRegPattern1		= CCM_REG_BASE->PllVeRegPattern1;
	CCM_REG_BACK.PllDeRegPattern0		= CCM_REG_BASE->PllDeRegPattern0;
	CCM_REG_BACK.PllDeRegPattern1		= CCM_REG_BASE->PllDeRegPattern1;
	//CCM_REG_BACK.PllHsicRegPattern0		= CCM_REG_BASE->PllHsicRegPattern0;
	//CCM_REG_BACK.PllHsicRegPattern1		= CCM_REG_BASE->PllHsicRegPattern1;
	CCM_REG_BACK.PllAudioRegPattern0	= CCM_REG_BASE->PllAudioRegPattern0;
	CCM_REG_BACK.PllAudioRegPattern1	= CCM_REG_BASE->PllAudioRegPattern1;

	//bias:
	CCM_REG_BACK.PllC0Bias      	= CCM_REG_BASE->PllC0Bias;
//	CCM_REG_BACK.PllDdr0Bias      	= CCM_REG_BASE->PllDdr0Bias;
	CCM_REG_BACK.PllPeriph0Bias     = CCM_REG_BASE->PllPeriph0Bias;
	CCM_REG_BACK.PllPeriph1Bias     = CCM_REG_BASE->PllPeriph1Bias;
	CCM_REG_BACK.PllGpuBias      	= CCM_REG_BASE->PllGpuBias;
	CCM_REG_BACK.PllVideo0Bias      = CCM_REG_BASE->PllVideo0Bias;
	CCM_REG_BACK.PllVideo1Bias     	= CCM_REG_BASE->PllVideo1Bias;
	CCM_REG_BACK.PllVeBias      	= CCM_REG_BASE->PllVeBias;
	CCM_REG_BACK.PllDeBias     	= CCM_REG_BASE->PllDeBias;
	//CCM_REG_BACK.PllIspBias      	= CCM_REG_BASE->PllIspBias;
	//CCM_REG_BACK.PllHsicBias      	= CCM_REG_BASE->PllHsicBias;
	CCM_REG_BACK.PllAudioBias      	= CCM_REG_BASE->PllAudioBias;

	//tun
	CCM_REG_BACK.PllCpuxTun      	= CCM_REG_BASE->PllCpuxTun;

	//cfg
	CCM_REG_BACK.CpuAxiCfg		= CCM_REG_BASE->CpuAxiCfg;
	CCM_REG_BACK.PsiAhb1Ahb2Cfg  	= CCM_REG_BASE->PsiAhb1Ahb2Cfg;
	CCM_REG_BACK.Ahb3Cfg		= CCM_REG_BASE->Ahb3Cfg;
	CCM_REG_BACK.Apb1Cfg		= CCM_REG_BASE->Apb1Cfg;
	CCM_REG_BACK.Apb2Cfg		= CCM_REG_BASE->Apb2Cfg;
	CCM_REG_BACK.Cci400Cfg   	= CCM_REG_BASE->Cci400Cfg;
//	CCM_REG_BACK.MbusCfg		= CCM_REG_BASE->MbusCfg;

	//accelerator
	CCM_REG_BACK.DeClk		= CCM_REG_BASE->DeClk;
	CCM_REG_BACK.DeBgr	     	= CCM_REG_BASE->DeBgr;
	//CCM_REG_BACK.DiClk	    	= CCM_REG_BASE->DiClk;
	//CCM_REG_BACK.DiBgr	    	= CCM_REG_BASE->DiBgr;

	CCM_REG_BACK.GpuClk		= CCM_REG_BASE->GpuClk;
	CCM_REG_BACK.GpuBgr		= CCM_REG_BASE->GpuBgr;

	CCM_REG_BACK.CeClk		= CCM_REG_BASE->CeClk;
	CCM_REG_BACK.CeBgr		= CCM_REG_BASE->CeBgr;

	CCM_REG_BACK.VeClk	    	= CCM_REG_BASE->VeClk;
	CCM_REG_BACK.VeBgr  		= CCM_REG_BASE->VeBgr;

	CCM_REG_BACK.EmmcClk    	= CCM_REG_BASE->EmmcClk;
	CCM_REG_BACK.EmmcBgr    	= CCM_REG_BASE->EmmcBgr;

	CCM_REG_BACK.Vp9Clk    		= CCM_REG_BASE->Vp9Clk;
	CCM_REG_BACK.Vp9Bgr    		= CCM_REG_BASE->Vp9Bgr;

	//sys resource
	CCM_REG_BACK.DmaBgr     	= CCM_REG_BASE->DmaBgr;
	CCM_REG_BACK.MsgboxBgr  	= CCM_REG_BASE->MsgboxBgr;
	CCM_REG_BACK.SpinlockBgr       	= CCM_REG_BASE->SpinlockBgr;
	CCM_REG_BACK.HstimerBgr       	= CCM_REG_BASE->HstimerBgr;
	CCM_REG_BACK.AvsBgr       	= CCM_REG_BASE->AvsBgr;
	CCM_REG_BACK.DbgsysBgr      	= CCM_REG_BASE->DbgsysBgr;
	CCM_REG_BACK.PsiBgr	      	= CCM_REG_BASE->PsiBgr;
	CCM_REG_BACK.PwmBgr	      	= CCM_REG_BASE->PwmBgr;
	CCM_REG_BACK.IommuBgr	      	= CCM_REG_BASE->IommuBgr;

	//store media
//	CCM_REG_BACK.DramClk	      	= CCM_REG_BASE->DramClk;
	CCM_REG_BACK.MbusMstClkGating	= CCM_REG_BASE->MbusMstClkGating;
//	CCM_REG_BACK.DramBgr		= CCM_REG_BASE->DramBgr;
	CCM_REG_BACK.Nand0Clk		= CCM_REG_BASE->Nand0Clk;
	CCM_REG_BACK.Nand1Clk		= CCM_REG_BASE->Nand1Clk;
	CCM_REG_BACK.NandBgr     	= CCM_REG_BASE->NandBgr;
	CCM_REG_BACK.Smhc0Clk  		= CCM_REG_BASE->Smhc0Clk;
	CCM_REG_BACK.Smhc1Clk   	= CCM_REG_BASE->Smhc1Clk;
	CCM_REG_BACK.Smhc2Clk  		= CCM_REG_BASE->Smhc2Clk;
	CCM_REG_BACK.SmhcBgr		= CCM_REG_BASE->SmhcBgr;

	CCM_REG_BACK.UartBgr      	= CCM_REG_BASE->UartBgr;
	CCM_REG_BACK.TwiBgr     	= CCM_REG_BASE->TwiBgr;
	CCM_REG_BACK.Spi0Clk     	= CCM_REG_BASE->Spi0Clk;
	CCM_REG_BACK.Spi1Clk       	= CCM_REG_BASE->Spi1Clk;
	CCM_REG_BACK.SpiBgr     	= CCM_REG_BASE->SpiBgr;
	//CCM_REG_BACK.GmacBgr    	= CCM_REG_BASE->GmacBgr;
	CCM_REG_BACK.GpadcBgr    	= CCM_REG_BASE->GpadcBgr;
	//CCM_REG_BACK.TsClk		= CCM_REG_BASE->TsClk;
	//CCM_REG_BACK.TsBgr		= CCM_REG_BASE->TsBgr;
	//CCM_REG_BACK.IrtxClk		= CCM_REG_BASE->IrtxClk;
	//CCM_REG_BACK.IrtxBgr		= CCM_REG_BASE->IrtxBgr;
	CCM_REG_BACK.ThsBgr		= CCM_REG_BASE->ThsBgr;
	//CCM_REG_BACK.I2sPcm3Clk		= CCM_REG_BASE->I2sPcm3Clk;
	CCM_REG_BACK.I2sPcm0Clk		= CCM_REG_BASE->I2sPcm0Clk;
	CCM_REG_BACK.I2sPcm1Clk		= CCM_REG_BASE->I2sPcm1Clk;
	CCM_REG_BACK.I2sPcm2Clk		= CCM_REG_BASE->I2sPcm2Clk;
	CCM_REG_BACK.I2sPcmBgr		= CCM_REG_BASE->I2sPcmBgr;
	//CCM_REG_BACK.SpdifClk		= CCM_REG_BASE->SpdifClk;
	//CCM_REG_BACK.SpdifBgr		= CCM_REG_BASE->SpdifBgr;
	CCM_REG_BACK.DmicClk		= CCM_REG_BASE->DmicClk;
	CCM_REG_BACK.DmicBgr		= CCM_REG_BASE->DmicBgr;
	CCM_REG_BACK.AudioCodec1xClk	= CCM_REG_BASE->AudioCodec1xClk;
	CCM_REG_BACK.AudioCodec4xClk	= CCM_REG_BASE->AudioCodec4xClk;
	CCM_REG_BACK.AudioCodecBgr      = CCM_REG_BASE->AudioCodecBgr;

	CCM_REG_BACK.Usb0Clk		= CCM_REG_BASE->Usb0Clk;
	CCM_REG_BACK.Usb1Clk		= CCM_REG_BASE->Usb1Clk;
	//CCM_REG_BACK.Usb3Clk		= CCM_REG_BASE->Usb3Clk;
	CCM_REG_BACK.UsbCgr		= CCM_REG_BASE->UsbCgr;

	//CCM_REG_BACK.PcieRefClkReg	= CCM_REG_BASE->PcieRefClkReg;
	//CCM_REG_BACK.PcieAxiClkReg	= CCM_REG_BASE->PcieAxiClkReg;
	//CCM_REG_BACK.PcieAuxClkReg	= CCM_REG_BASE->PcieAuxClkReg;
	//CCM_REG_BACK.PcieCgrReg  	= CCM_REG_BASE->PcieCgrReg;

	//CCM_REG_BACK.HdmiClk		= CCM_REG_BASE->HdmiClk;
	//CCM_REG_BACK.HdmiSlowClk	= CCM_REG_BASE->HdmiSlowClk;
	//CCM_REG_BACK.HdmiBgr		= CCM_REG_BASE->HdmiBgr;
	CCM_REG_BACK.MipiDsiDphy0HsClk	= CCM_REG_BASE->MipiDsiDphy0HsClk;
	CCM_REG_BACK.MipiDsiHost0Clk	= CCM_REG_BASE->MipiDsiHost0Clk;
	CCM_REG_BACK.MipiDsiDphy1HsClk	= CCM_REG_BASE->MipiDsiDphy1HsClk;
	CCM_REG_BACK.MipiDsiHost1Clk	= CCM_REG_BASE->MipiDsiHost1Clk;
	CCM_REG_BACK.MipiDsiBgr	        = CCM_REG_BASE->MipiDsiBgr;
	CCM_REG_BACK.DispIfTopBgr	= CCM_REG_BASE->DispIfTopBgr;
	CCM_REG_BACK.TconLcd0Clk	= CCM_REG_BASE->TconLcd0Clk;
	CCM_REG_BACK.TconLcd1Clk	= CCM_REG_BASE->TconLcd1Clk;
	CCM_REG_BACK.TconLcdBgr		= CCM_REG_BASE->TconLcdBgr;
	//CCM_REG_BACK.TconTvClk		= CCM_REG_BASE->TconTvClk;
	//CCM_REG_BACK.TconTvBgr		= CCM_REG_BASE->TconTvBgr;
	CCM_REG_BACK.EdpClk		= CCM_REG_BASE->EdpClk;
	CCM_REG_BACK.EdpBgr		= CCM_REG_BASE->EdpBgr;

	CCM_REG_BACK.CsiMiscClk		= CCM_REG_BASE->CsiMiscClk;
	CCM_REG_BACK.CsiTopClk		= CCM_REG_BASE->CsiTopClk;
	CCM_REG_BACK.CsiMstClk0		= CCM_REG_BASE->CsiMstClk0;
	CCM_REG_BACK.CsiBgr		= CCM_REG_BASE->CsiBgr;
	//CCM_REG_BACK.HdmiHdcpClk	= CCM_REG_BASE->HdmiHdcpClk;
	//CCM_REG_BACK.HdmiHdcpBgr	= CCM_REG_BASE->HdmiHdcpBgr;
	CCM_REG_BACK.CcmuSecSwitch	= CCM_REG_BASE->CcmuSecSwitch;
	CCM_REG_BACK.PllLockDbgCtrl	= CCM_REG_BASE->PllLockDbgCtrl;
	CCM_REG_BACK.PllCpuxHwFm	= CCM_REG_BASE->PllCpuxHwFm;
	CCM_REG_BACK.CcmuVersion	= CCM_REG_BASE->CcmuVersion;

	return 0;
}



int mem_ccu_restore(void)
{
	volatile u32 value;
#ifndef FPGA_PLATFORM
	int cnt = 1000;
#endif
	//bias:
	CCM_REG_BASE->PllC0Bias       	= CCM_REG_BACK.PllC0Bias;
//	CCM_REG_BASE->PllDdr0Bias      	= CCM_REG_BACK.PllDdr0Bias;
	CCM_REG_BASE->PllPeriph0Bias    = CCM_REG_BACK.PllPeriph0Bias;
	CCM_REG_BASE->PllPeriph1Ctl     = CCM_REG_BACK.PllPeriph1Ctl;
	CCM_REG_BASE->PllGpuBias      	= CCM_REG_BACK.PllGpuBias;
	CCM_REG_BASE->PllVideo0Bias     = CCM_REG_BACK.PllVideo0Bias;
	CCM_REG_BASE->PllVideo1Bias	= CCM_REG_BACK.PllVideo1Bias;
	CCM_REG_BASE->PllVeBias      	= CCM_REG_BACK.PllVeBias;
	CCM_REG_BASE->PllDeBias     	= CCM_REG_BACK.PllDeBias;
	//CCM_REG_BASE->PllIspBias      	= CCM_REG_BACK.PllIspBias;
	//CCM_REG_BASE->PllHsicBias      	= CCM_REG_BACK.PllHsicBias;
	CCM_REG_BASE->PllAudioBias	= CCM_REG_BACK.PllAudioBias;

	//tune&pattern:
	CCM_REG_BASE->PllCpuxTun	      	= CCM_REG_BACK.PllCpuxTun;
//	CCM_REG_BASE->PllDdr0RegPattern		= CCM_REG_BACK.PllDdr0RegPattern;
	CCM_REG_BASE->PllPeri1Pattern0		= CCM_REG_BACK.PllPeri1Pattern0;
	CCM_REG_BASE->PllPeri1Pattern1		= CCM_REG_BACK.PllPeri1Pattern1;
	CCM_REG_BASE->PllGpuRegPattern0		= CCM_REG_BACK.PllGpuRegPattern0;
	CCM_REG_BASE->PllGpuRegPattern1		= CCM_REG_BACK.PllGpuRegPattern1;
	CCM_REG_BASE->PllVideo0RegPattern0	= CCM_REG_BACK.PllVideo0RegPattern0;
	CCM_REG_BASE->PllVideo0RegPattern1	= CCM_REG_BACK.PllVideo0RegPattern1;
	CCM_REG_BASE->PllVideo1RegPattern0	= CCM_REG_BACK.PllVideo1RegPattern0;
	CCM_REG_BASE->PllVideo1RegPattern1	= CCM_REG_BACK.PllVideo1RegPattern1;
	CCM_REG_BASE->PllVeRegPattern0		= CCM_REG_BACK.PllVeRegPattern0;
	CCM_REG_BASE->PllVeRegPattern1		= CCM_REG_BACK.PllVeRegPattern1;
	CCM_REG_BASE->PllDeRegPattern0		= CCM_REG_BACK.PllDeRegPattern0;
	CCM_REG_BASE->PllDeRegPattern1		= CCM_REG_BACK.PllDeRegPattern1;
	//CCM_REG_BASE->PllHsicRegPattern0	= CCM_REG_BACK.PllHsicRegPattern0;
	//CCM_REG_BASE->PllHsicRegPattern1	= CCM_REG_BACK.PllHsicRegPattern1;
	CCM_REG_BASE->PllAudioRegPattern0	= CCM_REG_BACK.PllAudioRegPattern0;
	CCM_REG_BASE->PllAudioRegPattern1	= CCM_REG_BACK.PllAudioRegPattern1;

	//ctlr:
	CCM_REG_BASE->PllC0Ctl			= CCM_REG_BACK.PllC0Ctl;
//	CCM_REG_BASE->PllDdr0Ctl		             = CCM_REG_BACK.PllDdr0Ctl;
	CCM_REG_BASE->PllPeriph0Ctl      	= CCM_REG_BACK.PllPeriph0Ctl;
	CCM_REG_BASE->PllPeriph1Ctl      	= CCM_REG_BACK.PllPeriph1Ctl;
	CCM_REG_BASE->PllGpuCtl      		= CCM_REG_BACK.PllGpuCtl;;
	CCM_REG_BASE->PllVideo0Ctl      	= CCM_REG_BACK.PllVideo0Ctl;
	CCM_REG_BASE->PllVideo1Ctl	 	= CCM_REG_BACK.PllVideo1Ctl;
	CCM_REG_BASE->PllVeCtl      		= CCM_REG_BACK.PllVeCtl;
	CCM_REG_BASE->PllDeCtl      		= CCM_REG_BACK.PllDeCtl;
	//CCM_REG_BASE->PllHsicCtl		= CCM_REG_BACK.PllHsicCtl;
	CCM_REG_BASE->PllAudioCtl		= CCM_REG_BACK.PllAudioCtl;

	//cfg
	CCM_REG_BASE->CpuAxiCfg		= CCM_REG_BACK.CpuAxiCfg;
	CCM_REG_BASE->PsiAhb1Ahb2Cfg  	= CCM_REG_BACK.PsiAhb1Ahb2Cfg;
	CCM_REG_BASE->Ahb3Cfg		= CCM_REG_BACK.Ahb3Cfg;
	CCM_REG_BASE->Apb1Cfg		= CCM_REG_BACK.Apb1Cfg;
	CCM_REG_BASE->Apb2Cfg		= CCM_REG_BACK.Apb2Cfg;
	CCM_REG_BASE->Cci400Cfg   	= CCM_REG_BACK.Cci400Cfg;
//	CCM_REG_BASE->MbusCfg		= CCM_REG_BACK.MbusCfg;

#ifndef FPGA_PLATFORM
	while (cnt--) {
		//mem_long_jump((mem_long_jump_fn)time_udelay, 10);
		time_udelay(10);
		if (((((CCM_REG_BASE->PllC0Ctl.dwval) >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllAudioCtl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllVideo0Ctl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllVeCtl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllDeCtl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllPeriph0Ctl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllPeriph1Ctl >> 28) & 0x1) == 1) \
			&& ((((u32)CCM_REG_BASE->PllGpuCtl >> 28) & 0x1) == 1))
			break;
	}

	if (cnt < 0) {
		//MEM_ERR("can't wait the pll stable\n");
	}
#endif
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//notice: be care to cpu, axi restore sequence!.
	value = (u32)CCM_REG_BASE->CpuAxiCfg.dwval;
	value &= ~0x3;
	value |= (CCM_REG_BACK.CpuAxiCfg.dwval & 0x3);
	CCM_REG_BASE->CpuAxiCfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	value = CCM_REG_BASE->CpuAxiCfg.dwval;
	value &= ~(0x3 << 24);
	value |= (CCM_REG_BACK.CpuAxiCfg.dwval & (0x3 << 24));
	CCM_REG_BASE->CpuAxiCfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	value &= ~(0x3 << 8);
	value |= (CCM_REG_BACK.CpuAxiCfg.dwval & (0x3 << 8));
	CCM_REG_BASE->CpuAxiCfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//set ahb1 and apb1
	//increase freq, set div first
	value = CCM_REG_BASE->PsiAhb1Ahb2Cfg.dwval;
	value &= ~0x3ff;
	value |= (CCM_REG_BACK.PsiAhb1Ahb2Cfg.dwval & 0x3ff);
	CCM_REG_BASE->PsiAhb1Ahb2Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	//set ahb1 src
	value &= ~(0x3 << 24);
	value |=  CCM_REG_BACK.PsiAhb1Ahb2Cfg.dwval & (0x3 << 24);
	CCM_REG_BASE->PsiAhb1Ahb2Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//apb1
	value = CCM_REG_BASE->Apb1Cfg.dwval;
	value &= ~0x3ff;
	value |= (CCM_REG_BACK.Apb1Cfg.dwval & 0x3ff);
	CCM_REG_BASE->Apb1Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	//set apb1 src
	value &= ~(0x3 << 24);
	value |=  CCM_REG_BACK.Apb1Cfg.dwval & (0x3 << 24);
	CCM_REG_BASE->Apb1Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//set apb2 div
	value = CCM_REG_BASE->Apb2Cfg.dwval;
	value &=~0x3ff;
	value |= CCM_REG_BACK.Apb2Cfg.dwval & 0x3ff;
	CCM_REG_BASE->Apb2Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	value &= ~(0x3 << 24);
	value |= CCM_REG_BACK.Apb2Cfg.dwval & (0x3 << 24);
	CCM_REG_BASE->Apb2Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//ahb3
	value = CCM_REG_BASE->Ahb3Cfg.dwval;
	value &=~0x3ff;
	value |= CCM_REG_BACK.Ahb3Cfg.dwval & 0x3ff;
	CCM_REG_BASE->Ahb3Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);
	value &= ~(0x3 << 24);
	value |= CCM_REG_BACK.Ahb3Cfg.dwval & (0x3 << 24);
	CCM_REG_BASE->Ahb3Cfg.dwval = value;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 2);
	time_udelay(2);

	//CCM_REG_BASE->SysClkDiv		= CCM_REG_BACK.SysClkDiv;
	//CCM_REG_BASE->Ahb1Div			= CCM_REG_BACK.Ahb1Div;
	//mem_long_jump((mem_long_jump_fn)time_udelay, 1);
	//CCM_REG_BASE->Apb2Div			= CCM_REG_BACK.Apb2Div;

	//mem_long_jump((mem_long_jump_fn)time_udelay, 10);
	time_udelay(10);
	CCM_REG_BASE->DeClk		= CCM_REG_BACK.DeClk;
	CCM_REG_BASE->DeBgr	     	= CCM_REG_BACK.DeBgr;
	//CCM_REG_BASE->DiClk	    	= CCM_REG_BACK.DiClk;
	//CCM_REG_BASE->DiBgr	    	= CCM_REG_BACK.DiBgr;
	CCM_REG_BASE->GpuClk		= CCM_REG_BACK.GpuClk;
	CCM_REG_BASE->GpuBgr		= CCM_REG_BACK.GpuBgr;
	CCM_REG_BASE->CeClk		= CCM_REG_BACK.CeClk;
	CCM_REG_BASE->CeBgr		= CCM_REG_BACK.CeBgr;
	CCM_REG_BASE->VeClk	    	= CCM_REG_BACK.VeClk;
	CCM_REG_BASE->VeBgr  		= CCM_REG_BACK.VeBgr;
	CCM_REG_BASE->EmmcClk    	= CCM_REG_BACK.EmmcClk;
	CCM_REG_BASE->EmmcBgr    	= CCM_REG_BACK.EmmcBgr;
	CCM_REG_BASE->Vp9Clk    	= CCM_REG_BACK.Vp9Clk;
	CCM_REG_BASE->Vp9Bgr    	= CCM_REG_BACK.Vp9Bgr;
	CCM_REG_BASE->DmaBgr     	= CCM_REG_BACK.DmaBgr;
	CCM_REG_BASE->MsgboxBgr  	= CCM_REG_BACK.MsgboxBgr;
	CCM_REG_BASE->SpinlockBgr  	= CCM_REG_BACK.SpinlockBgr;
	CCM_REG_BASE->HstimerBgr       	= CCM_REG_BACK.HstimerBgr;
	CCM_REG_BASE->AvsBgr       	= CCM_REG_BACK.AvsBgr;
	CCM_REG_BASE->DbgsysBgr      	= CCM_REG_BACK.DbgsysBgr;
	CCM_REG_BASE->PsiBgr	      	= CCM_REG_BACK.PsiBgr;
	CCM_REG_BASE->PwmBgr	      	= CCM_REG_BACK.PwmBgr;
	CCM_REG_BASE->IommuBgr	      	= CCM_REG_BACK.IommuBgr;

	//store                                   media
//	CCM_REG_BASE->DramClk	      	= CCM_REG_BACK.DramClk;
	CCM_REG_BASE->MbusMstClkGating	= CCM_REG_BACK.MbusMstClkGating;
//	CCM_REG_BASE->DramBgr		= CCM_REG_BACK.DramBgr;
	CCM_REG_BASE->Nand0Clk		= CCM_REG_BACK.Nand0Clk;
	CCM_REG_BASE->Nand1Clk		= CCM_REG_BACK.Nand1Clk;
	CCM_REG_BASE->NandBgr     	= CCM_REG_BACK.NandBgr;
	CCM_REG_BASE->Smhc0Clk  	= CCM_REG_BACK.Smhc0Clk;
	CCM_REG_BASE->Smhc1Clk   	= CCM_REG_BACK.Smhc1Clk;
	CCM_REG_BASE->Smhc2Clk  	= CCM_REG_BACK.Smhc2Clk;
	CCM_REG_BASE->SmhcBgr		= CCM_REG_BACK.SmhcBgr;
	CCM_REG_BASE->UartBgr      	= CCM_REG_BACK.UartBgr;
	CCM_REG_BASE->TwiBgr     	= CCM_REG_BACK.TwiBgr;
	CCM_REG_BASE->Spi0Clk     	= CCM_REG_BACK.Spi0Clk;
	CCM_REG_BASE->Spi1Clk       	= CCM_REG_BACK.Spi1Clk;
	CCM_REG_BASE->SpiBgr     	= CCM_REG_BACK.SpiBgr;
	//CCM_REG_BASE->GmacBgr    	= CCM_REG_BACK.GmacBgr;
	CCM_REG_BASE->GpadcBgr    	= CCM_REG_BACK.GpadcBgr;
	//CCM_REG_BASE->TsClk		= CCM_REG_BACK.TsClk;
	//CCM_REG_BASE->TsBgr		= CCM_REG_BACK.TsBgr;
	//CCM_REG_BASE->IrtxClk		= CCM_REG_BACK.IrtxClk;
	//CCM_REG_BASE->IrtxBgr		= CCM_REG_BACK.IrtxBgr;
	CCM_REG_BASE->ThsBgr		= CCM_REG_BACK.ThsBgr;
	//CCM_REG_BASE->I2sPcm3Clk	= CCM_REG_BACK.I2sPcm3Clk;
	CCM_REG_BASE->I2sPcm0Clk	= CCM_REG_BACK.I2sPcm0Clk;
	CCM_REG_BASE->I2sPcm1Clk	= CCM_REG_BACK.I2sPcm1Clk;
	CCM_REG_BASE->I2sPcm2Clk	= CCM_REG_BACK.I2sPcm2Clk;
	CCM_REG_BASE->I2sPcmBgr		= CCM_REG_BACK.I2sPcmBgr;
	//CCM_REG_BASE->SpdifClk		= CCM_REG_BACK.SpdifClk;
	//CCM_REG_BASE->SpdifBgr		= CCM_REG_BACK.SpdifBgr;
	CCM_REG_BASE->DmicClk		= CCM_REG_BACK.DmicClk;
	CCM_REG_BASE->DmicBgr		= CCM_REG_BACK.DmicBgr;
	CCM_REG_BASE->AudioCodec1xClk	= CCM_REG_BACK.AudioCodec1xClk;
	CCM_REG_BASE->AudioCodec4xClk	= CCM_REG_BACK.AudioCodec4xClk;
	CCM_REG_BASE->AudioCodecBgr	= CCM_REG_BACK.AudioCodecBgr;

	CCM_REG_BASE->Usb0Clk		= CCM_REG_BACK.Usb0Clk;
	CCM_REG_BASE->Usb1Clk		= CCM_REG_BACK.Usb1Clk;
	//CCM_REG_BASE->Usb3Clk		= CCM_REG_BACK.Usb3Clk;
	CCM_REG_BASE->UsbCgr		= CCM_REG_BACK.UsbCgr;

	//CCM_REG_BASE->HdmiClk		= CCM_REG_BACK.HdmiClk;
	//CCM_REG_BASE->HdmiSlowClk	= CCM_REG_BACK.HdmiSlowClk;
	//CCM_REG_BASE->HdmiBgr		= CCM_REG_BACK.HdmiBgr;
	CCM_REG_BASE->MipiDsiDphy0HsClk	= CCM_REG_BACK.MipiDsiDphy0HsClk;
	CCM_REG_BASE->MipiDsiHost0Clk	= CCM_REG_BACK.MipiDsiHost0Clk;
	CCM_REG_BASE->MipiDsiDphy1HsClk	= CCM_REG_BACK.MipiDsiDphy1HsClk;
	CCM_REG_BASE->MipiDsiHost1Clk	= CCM_REG_BACK.MipiDsiHost1Clk;
	CCM_REG_BASE->MipiDsiBgr	= CCM_REG_BACK.MipiDsiBgr;

	CCM_REG_BASE->DispIfTopBgr	= CCM_REG_BACK.DispIfTopBgr;
	CCM_REG_BASE->TconLcd0Clk	= CCM_REG_BACK.TconLcd0Clk;
	CCM_REG_BASE->TconLcd1Clk	= CCM_REG_BACK.TconLcd1Clk;
	CCM_REG_BASE->TconLcdBgr	= CCM_REG_BACK.TconLcdBgr;
	//CCM_REG_BASE->TconTvClk		= CCM_REG_BACK.TconTvClk;
	//CCM_REG_BASE->TconTvBgr		= CCM_REG_BACK.TconTvBgr;
	CCM_REG_BASE->EdpClk		= CCM_REG_BACK.EdpClk;
	CCM_REG_BASE->EdpBgr		= CCM_REG_BACK.EdpBgr;
	CCM_REG_BASE->CsiMiscClk	= CCM_REG_BACK.CsiMiscClk;
	CCM_REG_BASE->CsiTopClk		= CCM_REG_BACK.CsiTopClk;
	CCM_REG_BASE->CsiMstClk0	= CCM_REG_BACK.CsiMstClk0;
	CCM_REG_BASE->CsiBgr		= CCM_REG_BACK.CsiBgr;
	//CCM_REG_BASE->HdmiHdcpClk	= CCM_REG_BACK.HdmiHdcpClk;
	//CCM_REG_BASE->HdmiHdcpBgr	= CCM_REG_BACK.HdmiHdcpBgr;

	//mem_long_jump((mem_long_jump_fn)time_udelay, 10);
	time_udelay(10);

	CCM_REG_BASE->CcmuSecSwitch	= CCM_REG_BACK.CcmuSecSwitch;
	CCM_REG_BASE->PllLockDbgCtrl	= CCM_REG_BACK.PllLockDbgCtrl;
	CCM_REG_BASE->CcmuVersion	= CCM_REG_BACK.CcmuVersion;
	//printk("CcmuVersion:0x%x\n", (u32)(&(CCM_REG_BASE->CcmuVersion)));

	//mem_long_jump((mem_long_jump_fn)time_mdelay, 1);
	time_mdelay(1);

	if (((u32)(&(CCM_REG_BASE->CcmuVersion)) == (CCU_REG_BASE + 0xff0)) && (CCM_REG_BASE->CcmuVersion == 0x00020001))
		//ccu initialize succeeded
		return OK;
	else
		while(1);
}
#endif /* CONFIG_ARCH_SUN50IW3P1 */
#endif /* MEM_USED */
