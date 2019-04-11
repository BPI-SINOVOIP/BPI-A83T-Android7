#ifndef __MEM_CCU_SUN50IW3_H__
#define __MEM_CCU_SUN50IW3_H__

#if defined CONFIG_ARCH_SUN50IW3P1

#define CCM_REG_LENGTH		((0xFF0+0x4)>>2)

typedef union{
	unsigned int dwval;
	struct
	{
		u32 enable:1;		//bit31, 0-disable, 1-enable, (24Mhz*N*K)/(M)
		u32 reserved0:1;	//bit30, reserved
		u32 lock_en:1;		//bit29, 0-disable lock, 1-enable lock
		u32 lock_st:1;		//bit28, 0-unlocked, 1-locked(PLL has been stable)
		u32 reserved1:1;	//bit27, reserved
		u32 lock_time:3;	//bit24, lock time:freq scaling step
		u32 reserved2:6;	//bit18, reserved
		u32 pll_out_ext_divp:2; //bit16, PLL Output external divider P
		u32 factor_n:8;		//bit8,  PLL1 Factor_N
		u32 reserved3:6;	//bit2,  reserved
		u32 factor_m:2;		//bit0,  PLL1 Factor_M
	} bits;
} __ccmu_pll_cpux_reg0000_t;

#define AC327_CLKSRC_LOSC   (0)
#define AC327_CLKSRC_HOSC   (1)
#define AC327_CLKSRC_PLL1   (2)
typedef union{
	unsigned int dwval;
	struct
	{
		unsigned int   reserved2:6;       //bit18,  reserved
		unsigned int   CpuClkSrc:2;       //bit16, AXI1 clock divide ratio, 000-1, 001-2, 010-3, 011-4
		unsigned int   reserved1:14;        //bit10, reserved
		unsigned int   CpuApbFactorN:2;     //bit8, CPU0/1/2/3 clock source select, 0-HOSC, 1-PLL_C0CPUX
		unsigned int   reserved0:6;        //bit2,  reserved
		unsigned int   FactorM:2;        //bit0,  AXI0 clock divide ratio, 000-1, 001-2, 010-3, 011-4
	} bits;
}__ccmu_sysclk_ratio_reg0500_t;

#define AHB1_CLKSRC_LOSC    (0)
#define AHB1_CLKSRC_HOSC    (1)
#define AHB1_CLKSRC_RC     (2)
#define AHB1_CLKSRC_PLL6    (3)
typedef union{
	unsigned int dwval;
	struct
	{
		unsigned int   reserved2:6;      //bit26, reserved
		unsigned int   ClkSrcSel:2;      //bit24, ahb1 ahb2 clock source select, 00-LOSC, 01-OSC24M, 10/11-PLL6/ahb1_pre_div
		unsigned int   reserved1:14;     //bit8,  reserved
		unsigned int   FactorN:2;        //bit6,  pll facotrN
		unsigned int   reserved0:6;      //bit4,  reserved
		unsigned int   FactorM:2;        //bit0,  pll facotrM,tht range is from 1 to 4,M = FactorM+1.
	} bits;
}__ccmu_ahb1_ahb2_ratio_reg0510_t;

typedef union{
	unsigned int dwval;
	struct
	{
		unsigned int   reserved2:6;        //bit26, reserved
		unsigned int   ClkSrc:2;           //bit24, clock source select, 00-LOSC, 01-OSC24M, 10/11-PLL6
		unsigned int   reserved1:14;       //bit10, reserved
		unsigned int   FactorN:2;          //bit8,  pll facotrN
		unsigned int   reserved:6;         //bit2,  reserved
		unsigned int   FactortM:2;         //bit0,  pll facotrM,tht range is from 1 to 4,M = FactorM+1.
	} bits;
} __ccmu_bus_ratio_reg_t;

#define APB2_CLKSRC_LOSC    (0)
#define APB2_CLKSRC_HOSC    (1)
#define APB2_CLKSRC_PLL6    (2)
#define APB1_CLKSRC_LOSC    (0)
#define APB1_CLKSRC_HOSC    (1)
#define APB1_CLKSRC_PLL6    (2)
#define AHB3_CLKSRC_LOSC    (0)
#define AHB3_CLKSRC_HOSC    (1)
#define AHB3_CLKSRC_PLL6    (2)
#define	__ccmu_apb1_ratio_reg0520_t __ccmu_bus_ratio_reg_t
#define	__ccmu_apb2_ratio_reg0524_t __ccmu_bus_ratio_reg_t
#define	__ccmu_ahb3_ratio_reg051c_t __ccmu_bus_ratio_reg_t

typedef struct __CCMU_PLLLOCK_REG0200
{
	unsigned int   reserved:16;        //bit16, reserved
	unsigned int   LockTime:16;        //bit0,  PLL lock time, based on us
} __ccmu_plllock_reg0200_t;

typedef struct __CCMU_REG_LIST
{
	volatile __ccmu_pll_cpux_reg0000_t                 PllC0Ctl;               //0x0000, pll cpu0
	volatile unsigned int                              reserved4[3];           //0x0004, reserved
	volatile unsigned int                              PllDdr0Ctl;             //0x0010, pll ddr0
	volatile unsigned int                              reserved14[3];          //0x0014, reserved
	volatile unsigned int                              PllPeriph0Ctl;          //0x0020, pll periph0
	volatile unsigned int                              reserved24[1];          //0x0024, reserved
	volatile unsigned int                              PllPeriph1Ctl;	   //0x0028, pll periph1
	volatile unsigned int                              reserved2c[1];          //0x002c, reserved
	volatile unsigned int                              PllGpuCtl;              //0x0030, pll gpu
	volatile unsigned int                              reserved34[3];          //0x0034, reserved
	volatile unsigned int                              PllVideo0Ctl;           //0x0040, pll video0
	volatile unsigned int                              reserved44[1];          //0x0044, reserved
	volatile unsigned int                              PllVideo1Ctl;           //0x0048, pll video1
	volatile unsigned int                              reserved4c[3];          //0x004c, reserved
	volatile unsigned int                              PllVeCtl;               //0x0058, pll ve
	volatile unsigned int                              reserved5c[1];          //0x005c, reserved
	volatile unsigned int                              PllDeCtl;               //0x0060, pll de
	volatile unsigned int                              reserved64[3];          //0x0064, reserved
	volatile unsigned int                              PllHsicCtl;             //0x0070, pll hsic
	volatile unsigned int                              reserved74[1];          //0x0074, reserved
	volatile unsigned int                              PllAudioCtl;            //0x0078, pll audio
	volatile unsigned int                              reserved7c[37];         //0x007c, reserved
	volatile unsigned int                              PllDdr0RegPattern;      //0x0110, pll ddr pattern
	volatile unsigned int                              reserved114[5];         //0x0114, reserved
	volatile unsigned int                              PllPeri1Pattern0;       //0x0128, pll peri1 pattern0
	volatile unsigned int                              PllPeri1Pattern1;       //0x012c, pll peri1 pattern1
	volatile unsigned int                              PllGpuRegPattern0;      //0x0130, pll gpu pattern0
	volatile unsigned int                              PllGpuRegPattern1;      //0x0134, pll gpu pattern1
	volatile unsigned int                              reserved138[2];         //0x0138, reserved
	volatile unsigned int                              PllVideo0RegPattern0;   //0x0140, pll video0 pattern0
	volatile unsigned int                              PllVideo0RegPattern1;   //0x0144, pll video0 pattern1
	volatile unsigned int                              PllVideo1RegPattern0;   //0x0148, pll video1 pattern0
	volatile unsigned int                              PllVideo1RegPattern1;   //0x014c, pll video1 pattern1
	volatile unsigned int                              reserved150[2];         //0x0150, reserved
	volatile unsigned int                              PllVeRegPattern0;       //0x0158, pll ve pattern0
	volatile unsigned int                              PllVeRegPattern1;       //0x015c, pll ve pattern0
	volatile unsigned int                              PllDeRegPattern0;       //0x0160, pll de pattern0
	volatile unsigned int                              PllDeRegPattern1;       //0x0164, pll de pattern1
	volatile unsigned int                              reserved168[2];         //0x0168, reserved
	volatile unsigned int                              PllHsicRegPattern0;     //0x0170, pll hsic pattern0
	volatile unsigned int                              PllHsicRegPattern1;     //0x0174, pll hsic pattern1
	volatile unsigned int                              PllAudioRegPattern0;    //0x0178, pll audio pattern0
	volatile unsigned int                              PllAudioRegPattern1;    //0x017c, pll audio pattern1
	volatile unsigned int                              reserved180[96];        //0x0180, reserved
	volatile unsigned int                              PllC0Bias;              //0x0300, pll c0cpux bias reg;
	volatile unsigned int                              reserved304[3];         //0x0304, reserved
	volatile unsigned int                              PllDdr0Bias;            //0x0310, pll ddr0 bias reg;
	volatile unsigned int                              reserved314[3];         //0x0314, reserved
	volatile unsigned int                              PllPeriph0Bias;         //0x0320, pll periph0 bias reg;
	volatile unsigned int                              reserved324[1];         //0x0324, reserved
	volatile unsigned int                              PllPeriph1Bias;         //0x0328, pll periph1 bias reg;
	volatile unsigned int                              reserved32c[1];         //0x032c, reserved
	volatile unsigned int                              PllGpuBias;             //0x0330, pll gpu bias reg;
	volatile unsigned int                              reserved334[3];         //0x0334, reserved
	volatile unsigned int                              PllVideo0Bias;          //0x0340, pll video0 bias reg;
	volatile unsigned int                              reserved344[1];         //0x0344, reserved
	volatile unsigned int                              PllVideo1Bias;          //0x0348, pll video1 bias reg;
	volatile unsigned int                              reserved34c[3];         //0x034c, reserved
	volatile unsigned int                              PllVeBias;              //0x0358, pll Ve bias reg;
	volatile unsigned int                              reserved35c[1];         //0x035c, reserved
	volatile unsigned int                              PllDeBias;              //0x0360, pll de bias reg;
	volatile unsigned int                              reserved364[1];         //0x0364, reserved
	volatile unsigned int                              PllIspBias;             //0x0368, pll isp bias reg;
	volatile unsigned int                              reserved36c[1];         //0x036c, reserved
	volatile unsigned int                              PllHsicBias;            //0x0370, pll hsic bias reg;
	volatile unsigned int                              reserved374[1];         //0x0374, reserved
	volatile unsigned int                              PllAudioBias;           //0x0378, pll audio bias reg;
	volatile unsigned int                              reserved37c[33];        //0x037c, reserved
	volatile unsigned int                              PllCpuxTun;             //0x0400, pll cpux tun reg;
	volatile unsigned int                              reserved404[63];        //0x0404, reserved
	volatile __ccmu_sysclk_ratio_reg0500_t             CpuAxiCfg;              //0x0500, cpu axi cfg reg;
	volatile unsigned int                              reserved504[3];         //0x0504, reserved
	volatile __ccmu_ahb1_ahb2_ratio_reg0510_t          PsiAhb1Ahb2Cfg;         //0x0510, psi ahb1 ahb2 cfg reg;
	volatile unsigned int                              reserved514[2];         //0x0514, reserved
	volatile __ccmu_ahb3_ratio_reg051c_t               Ahb3Cfg;                //0x051c, ahb3 cfg reg;
	volatile __ccmu_apb1_ratio_reg0520_t               Apb1Cfg;                //0x0520, apb1 cfg reg;
	volatile __ccmu_apb2_ratio_reg0524_t               Apb2Cfg;                //0x0524, apb2 cfg reg;
	volatile unsigned int                              reserved528[2];         //0x0528, reserved
	volatile unsigned int                              Cci400Cfg;              //0x0530, cci400 cfg reg;
	volatile unsigned int                              reserved534[3];         //0x0534, reserved
	volatile unsigned int                              MbusCfg;                //0x0540, mbus cfg reg;
	volatile unsigned int                              reserved544[47];        //0x0544, reserved
	volatile unsigned int                              DeClk;                  //0x0600, de clk reg;
	volatile unsigned int                              reserved604[2];         //0x0604, reserved
	volatile unsigned int                              DeBgr;                  //0x060c, de bus gating reset reg;
	volatile unsigned int                              reserved610[4];         //0x0610, reserved
	volatile unsigned int                              DiClk;                  //0x0620, di clk reg;
	volatile unsigned int                              reserved624[2];         //0x0624, reserved
	volatile unsigned int                              DiBgr;                  //0x062c, di bgr reg;
	volatile unsigned int                              reserved630[16];        //0x0630, reserved
	volatile unsigned int                              GpuClk;                 //0x0670, gpu clk reg;
	volatile unsigned int                              reserved674[2];         //0x0674, reserved
	volatile unsigned int                              GpuBgr;                 //0x067c, gpu bgr reg;
	volatile unsigned int                              CeClk;                  //0x0680, ce clk reg;
	volatile unsigned int                              reserved684[2];         //0x0684, reserved
	volatile unsigned int                              CeBgr;                  //0x068c, ce bgr reg;
	volatile unsigned int                              VeClk;                  //0x0690, ve clk reg;
	volatile unsigned int                              reserved694[2];         //0x0694, reserved
	volatile unsigned int                              VeBgr;                  //0x069c, ve bgr reg;
	volatile unsigned int                              reserved6a0[4];         //0x06a0, reserved
	volatile unsigned int                              EmmcClk;                //0x06b0, emmc clk reg;
	volatile unsigned int                              reserved6b4[2];         //0x06b4, reserved
	volatile unsigned int                              EmmcBgr;                //0x06bc, emmc bgr reg;
	volatile unsigned int                              Vp9Clk;                 //0x06c0, vp9 clk reg;
	volatile unsigned int                              reserved6c4[2];         //0x06c4, reserved
	volatile unsigned int                              Vp9Bgr;                 //0x06cc, vp9 bgr reg;
	volatile unsigned int                              reserved6d0[15];        //0x06d0, reserved
	volatile unsigned int                              DmaBgr;                 //0x070c, dma bgr reg;
	volatile unsigned int                              reserved710[3];         //0x0710, reserved
	volatile unsigned int                              MsgboxBgr;              //0x071c, msgbox bgr reg;
	volatile unsigned int                              reserved720[3];         //0x0720, reserved
	volatile unsigned int                              SpinlockBgr;            //0x072c, spinlock bgr reg;
	volatile unsigned int                              reserved730[3];         //0x0730, reserved
	volatile unsigned int                              HstimerBgr;             //0x073c, hstimer bgr reg;
	volatile unsigned int                              AvsBgr;                 //0x0740, Avs bgr reg;
	volatile unsigned int                              reserved744[18];        //0x0744, reserved
	volatile unsigned int                              DbgsysBgr;              //0x078c, dbgsys bgr reg;
	volatile unsigned int                              reserved790[3];         //0x0790, reserved
	volatile unsigned int                              PsiBgr;                 //0x079c, psi bgr reg;
	volatile unsigned int                              reserved7a0[3];         //0x07a0, reserved
	volatile unsigned int                              PwmBgr;                 //0x07ac, pwm bgr reg;
	volatile unsigned int                              reserved7b0[3];         //0x07b0, reserved
	volatile unsigned int                              IommuBgr;               //0x07bc, iommu bgr reg;
	volatile unsigned int                              reserved7c0[16];        //0x07c0, reserved
	volatile unsigned int                              DramClk;                //0x0800, dram clk reg;
	volatile unsigned int                              MbusMstClkGating;       //0x0804, mbus master clk gating reg;
	volatile unsigned int                              reserved808[1];         //0x0808, reserved
	volatile unsigned int                              DramBgr;                //0x080c, mbus bgr reg;
	volatile unsigned int                              Nand0Clk;               //0x0810, nand0 clk reg;
	volatile unsigned int                              Nand1Clk;               //0x0814, nand1 clk reg;
	volatile unsigned int                              reserved818[5];         //0x0818, reserved
	volatile unsigned int                              NandBgr;                //0x082c, nand bgr reg;
	volatile unsigned int                              Smhc0Clk;               //0x0830, smhc0 reg;
	volatile unsigned int                              Smhc1Clk;               //0x0834, smhc1 reg;
	volatile unsigned int                              Smhc2Clk;               //0x0838, smhc2 reg;
	volatile unsigned int                              reserved83c[4];         //0x083c, reserved
	volatile unsigned int                              SmhcBgr;                //0x084c, smhc bgr reg;
	volatile unsigned int                              reserved850[47];        //0x0850, reserved
	volatile unsigned int                              UartBgr;                //0x090c, uart bgr reg;
	volatile unsigned int                              reserved910[3];         //0x0910, reserved
	volatile unsigned int                              TwiBgr;                 //0x091c, twi bgr reg;
	volatile unsigned int                              reserved920[7];         //0x0920, reserved
	volatile unsigned int                              ScrBgr;                 //0x093c, scr bgr reg;
	volatile unsigned int                              Spi0Clk;                //0x0940, spi0 clk reg;
	volatile unsigned int                              Spi1Clk;                //0x0944, spi1 clk reg;
	volatile unsigned int                              reserved948[9];         //0x0948, reserved
	volatile unsigned int                              SpiBgr;                 //0x096c, spi bgr reg;
	volatile unsigned int                              reserved970[3];         //0x0970, reserved
	volatile unsigned int                              GmacBgr;                //0x097c, gmac bgr reg;
	volatile unsigned int                              reserved980[12];        //0x0980, reserved
	volatile unsigned int                              TsClk;                  //0x09b0, ts clk reg;
	volatile unsigned int                              reserved9b4[2];         //0x09b4, reserved
	volatile unsigned int                              TsBgr;                  //0x09bc, ts bgr reg;
	volatile unsigned int                              IrtxClk;                //0x09c0, irtx clk reg;
	volatile unsigned int                              reserved9c4[2];         //0x09c4, reserved
	volatile unsigned int                              IrtxBgr;                //0x09cc, irtx bgr reg;
	volatile unsigned int                              reserved9d0[7];         //0x09d0, reserved
	volatile unsigned int                              GpadcBgr;               //0x09ec, gpadc bgr reg;
	volatile unsigned int                              reserved9f0[3];         //0x09f0, reserved
	volatile unsigned int                              ThsBgr;                 //0x09fc, ths bgr reg;

	volatile unsigned int                              reserveda00[3];         //0x0a00, reserved
	volatile unsigned int                              I2sPcm3Clk;             //0x0a0c, i2s/pcm3 clk reg;
	volatile unsigned int                              I2sPcm0Clk;             //0x0a10, i2s/pcm0 clk reg;
	volatile unsigned int                              I2sPcm1Clk;             //0x0a14, i2s/pcm1 clk reg;
	volatile unsigned int                              I2sPcm2Clk;             //0x0a18, i2s/pcm2 clk reg;
	volatile unsigned int                              I2sPcmBgr;              //0x0a1c, i2s/pcm bgr reg;

	volatile unsigned int                              SpdifClk;               //0x0a20, spdif clk reg;
	volatile unsigned int                              reserveda24[2];         //0x0a24
	volatile unsigned int                              SpdifBgr;               //0x0a2c, spdif bgr reg;

	volatile unsigned int                              reserveda30[4];         //0x0a30
	volatile unsigned int                              DmicClk;                //0x0a40, dmic clk reg;
	volatile unsigned int                              reserveda44[2];         //0x0a44
	volatile unsigned int                              DmicBgr;                //0x0a4c, dmic bgr reg;

	volatile unsigned int				   AudioCodec1xClk;        //0x0a50, audio codec 1x clk reg;
	volatile unsigned int				   AudioCodec4xClk;	   //0x0a54, audio codec 4x clk reg;
	volatile unsigned int				   reserveda58[1];	   //0x0a58
	volatile unsigned int                              AudioCodecBgr;          //0x0a5c, audio codec bgr;
	volatile unsigned int                              AudioHubClk;            //0x0a60, audio hub clk reg;
	volatile unsigned int                              reserveda64[2];         //0x0a64
	volatile unsigned int                              AudioHubBgr;            //0x0a6c, audio hub bgr;

	volatile unsigned int                              Usb0Clk;                //0x0a70, usb0 clk reg;
	volatile unsigned int                              Usb1Clk;                //0x0a74, usb1 clk reg;
	volatile unsigned int                              Usb3Clk;                //0x0a78, usb3 clk reg;
	volatile unsigned int                              reserveda80[4];         //0x0a7c
	volatile unsigned int                              UsbCgr;                 //0x0a8c, clk gating reset reg;

	volatile unsigned int                              reserveda90[8];         //0x0a90
	volatile unsigned int                              PcieRefClkReg;	   //0x0ab0, pcie ref clk reg;
	volatile unsigned int                              PcieAxiClkReg;          //0x0ab4, pcie axi clk reg;
	volatile unsigned int                              PcieAuxClkReg;	   //0x0ab8, pcie aux clk reg;
	volatile unsigned int                              PcieCgrReg;             //0x0abc, pcis clk gating rst reg;

	volatile unsigned int                              reservedac0[16];        //0x0ac0
	volatile unsigned int                              HdmiClk;                //0x0b00, clk reg;
	volatile unsigned int                              HdmiSlowClk;            //0x0b04, slow clk reg;
	volatile unsigned int                              reservedb08[2];         //0x0b08
	volatile unsigned int                              HdmiCecClk;             //0x0b10, slow cec clk reg;
	volatile unsigned int                              reservedb14[2];         //0x0b14
	volatile unsigned int                              HdmiBgr;                //0x0b1c, hdmi bgr reg;

	volatile unsigned int				   MipiDsiDphy0HsClk;      //0x0b20, mipi dsi dphy0 high speed clk;
	volatile unsigned int				   MipiDsiHost0Clk;	   //0x0b24, mipi dsi host0 clk;
	volatile unsigned int				   MipiDsiDphy1HsClk;      //0x0b28, mipi dsi dphy1 high speed clk;
	volatile unsigned int				   MipiDsiHost1Clk;	   //0x0b2c, mipi dsi host1 clk;
	volatile unsigned int                              reservedb30[7];         //0x0b30
	volatile unsigned int                              MipiDsiBgr;             //0x0b4c, mipi dsi bgr reg;

	volatile unsigned int                              reservedb50[3];         //0x0b50
	volatile unsigned int                              DispIfTopBgr;           //0x0b5c, display if top bgr reg;
	volatile unsigned int                              TconLcd0Clk;            //0x0b60, tcon_lcd0 clk reg;
	volatile unsigned int                              TconLcd1Clk;            //0x0b64, tcon_lcd1 clk reg;
	volatile unsigned int                              reservedb64[5];         //0x0b68
	volatile unsigned int                              TconLcdBgr;             //0x0b7c, tcon_lcd bgr reg;
	volatile unsigned int                              TconTvClk;              //0x0b80, tcon_tv clk reg;

	volatile unsigned int                              reservedb84[6];         //0x0b84
	volatile unsigned int                              TconTvBgr;              //0x0b9c, tcon_tv bgr reg;

	volatile unsigned int                              reservedba0[16];        //0x0ba0
	volatile unsigned int				   EdpClk;  		   //0x0be0, edp clk;
	volatile unsigned int                              reservedbe4[2];         //0x0be4
	volatile unsigned int				   EdpBgr;  		   //0x0bec, edp bgr;
	volatile unsigned int                              reservedbf0[4];         //0x0bf0

	volatile unsigned int                              CsiMiscClk;             //0x0c00, csi misc clk reg;
	volatile unsigned int                              CsiTopClk;              //0x0c04, csi top clk reg;
	volatile unsigned int                              CsiMstClk0;             //0x0c08, csi master clk0 reg;
	volatile unsigned int                              reservedc0c[8];         //0x0c0c
	volatile unsigned int                              CsiBgr;                 //0x0c2c, csi bgr reg;

	volatile unsigned int                              reservedc30[4];         //0x0c30
	volatile unsigned int                              HdmiHdcpClk;            //0x0c40, Hdmi Hdcp clk reg;
	volatile unsigned int                              reservedc44[2];         //0x0c44
	volatile unsigned int                              HdmiHdcpBgr;            //0x0c4c, Hdmi Hdcp bgr reg;

	volatile unsigned int                              reservedc50[172];       //0x0c50
	volatile unsigned int                              CcmuSecSwitch;          //0x0f00, ccmu security switch reg;
	volatile unsigned int                              PllLockDbgCtrl;         //0x0f04, pll lock debug control reg;
	volatile unsigned int                              reservedf08[6];         //0x0f08
	volatile unsigned int                              PllCpuxHwFm;            //0x0f20, pll_cpux hardware fm reg;
	volatile unsigned int                              reservedf24[51];        //0x0f24
	volatile unsigned int                              CcmuVersion;            //0x0ff0, ccmu version reg;
} __ccmu_reg_list_t;

struct ccm_state{
	__ccmu_reg_list_t *ccm_reg_base;
	__ccmu_reg_list_t ccm_reg_back;

};
#endif /* CONFIG_ARCH_SUN50IW3P1 */
#endif /* __MEM_CCU_SUN50IW3_H__ */

