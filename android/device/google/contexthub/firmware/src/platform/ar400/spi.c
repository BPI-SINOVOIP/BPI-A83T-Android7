/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <string.h>

#include <gpio.h>
#include <spi.h>
#include <spi_priv.h>
#include <util.h>
#include <atomicBitset.h>
#include <atomic.h>
#include <platform.h>

#include <plat/inc/cmsis.h>
#include <plat/inc/gpio.h>
#include <plat/inc/spi.h>
#include <isr.h>
#include <plat/inc/plat.h>
#include <plat/inc/include.h>
#include <cpu.h>


#include <cpu/inc/barrier.h>
/* run time control */
#define TEST_SPI_NO		(0)
#define SPI_DEFAULT_CLK	(40000000)
#define SPI_TX_WL		(32)
#define SPI_RX_WL		(32)
#define SPI_FIFO_SIZE	(64)
#define SPI_CLK_SRC		(1)	//0-24M, 1-PLL6
#define SPI_MCLK		(40000000)
#define SPI_CR1_CPHA                (1 << 0)
#define SPI_CR1_CPOL                (1 << 1)

/* bit field of registers */
/* SPI Master Burst Counter Register Bit Fields & Masks,default:0x0000_0000 */
/* master mode: when SMC = 1,BC specifies total burst number, Max length is 16Mbytes */
#define SPI_BC_CNT_MASK			(0xFFFFFF << 0) 	/* Total Burst Counter, tx length + rx length ,SMC=1 */

/* SPI Master Transmit Counter reigster default:0x0000_0000 */
#define SPI_TC_CNT_MASK			(0xFFFFFF << 0)		/* Write Transmit Counter, tx length, NOT rx length!!! */

/* SPI Master Burst Control Counter reigster Bit Fields & Masks,default:0x0000_0000 */
#define SPI_BCC_STC_MASK		(0xFFFFFF <<  0)	/* master single mode transmit counter */
#define SPI_BCC_DBC_MASK		(0xF	  << 24)	/* master dummy burst counter */
#define SPI_BCC_DUAL_MOD_RX_EN	(0x1	  << 28)	/* master dual mode RX enable */

/* SPI Transfer Control Register Bit Fields & Masks,defualt value:0x0000_0087 */
#define SPI_TC_CPHA			(0x1 <<  0)		/* SPI Clock/Data phase control,0: phase0,1: phase1;default:1 */
#define SPI_TC_CPOL			(0x1 <<  1)		/* SPI Clock polarity control,0:low level idle,1:high level idle;default:1 */
#define SPI_TC_SPOL	        (0x1 <<  2)  	/* SPI Chip select signal polarity control,default: 1,low effective like this:~~|_____~~ */
#define SPI_TC_SSCTL        (0x1 <<  3)  	/* SPI chip select control,default 0:SPI_SSx remains asserted between SPI bursts,1:negate SPI_SSx between SPI bursts */
#define SPI_TC_SS_MASK		(0x3 <<  4)		/* SPI chip select:00-SPI_SS0;01-SPI_SS1;10-SPI_SS2;11-SPI_SS3*/
#define SPI_TC_SS_OWNER		(0x1 <<  6)		/* SS output mode select default is 0:automatic output SS;1:manual output SS */
#define SPI_TC_SS_LEVEL		(0x1 <<  7)		/* defautl is 1:set SS to high;0:set SS to low */
#define SPI_TC_DHB			(0x1 <<  8)		/* Discard Hash Burst,default 0:receiving all spi burst in BC period 1:discard unused,fectch WTC bursts */
#define SPI_TC_DDB			(0x1 <<  9)		/* Dummy burst Type,default 0: dummy spi burst is zero;1:dummy spi burst is one */
#define SPI_TC_RPSM			(0x1 << 10) 	/* select mode for high speed write,0:normal write mode,1:rapids write mode,default 0 */
#define SPI_TC_SDC			(0x1 << 11) 	/* master sample data control, 1: delay--high speed operation;0:no delay. */
#define SPI_TC_FBS			(0x1 << 12) 	/* LSB/MSB transfer first select 0:MSB,1:LSB,default 0:MSB first */
#define SPI_TC_XCH          (0x1 << 31) 	/* Exchange burst default 0:idle,1:start exchange;when BC is zero,this bit cleared by SPI controller*/
#define SPI_TC_SS_BIT_POS			(4)

#define SPI_SOFT_RST	(1U << 31)
#define SPI_TXPAUSE_EN	(1U << 7)
#define SPI_MASTER		(1U << 1)
#define SPI_ENABLE		(1U << 0)
#define SPI_STATUS_IDLE	(1U << 31)

#define SPI_EXCHANGE	(1U << 31)
#define SPI_SAMPLE_MODE	(1U << 13)
#define SPI_LSB_MODE	(1U << 12)
#define SPI_SAMPLE_CTRL	(1U << 11)
#define SPI_RAPIDS_MODE	(1U << 10)
#define SPI_DUMMY_1		(1U << 9)
#define SPI_DHB			(1U << 8)
#define SPI_SET_SS_1	(1U << 7)
#define SPI_SS_MANUAL	(1U << 6)
#define SPI_SEL_SS0		(0U << 4)
#define SPI_SEL_SS1		(1U << 4)
#define SPI_SEL_SS2		(2U << 4)
#define SPI_SEL_SS3		(3U << 4)
#define SPI_SS_N_INBST	(1U << 3)
#define SPI_SS_ACTIVE0	(1U << 2)
#define SPI_MODE0		(0U << 0)
#define SPI_MODE1		(1U << 0)
#define SPI_MODE2		(2U << 0)
#define SPI_MODE3		(3U << 0)

#define SPI_CPHA        (1U << 0)

#define SPI_SS_INT		(1U << 13)
#define SPI_TC_INT		(1U << 12)
#define SPI_TXUR_INT	(1U << 11)
#define SPI_TXOF_INT	(1U << 10)
#define SPI_RXUR_INT	(1U << 9)
#define SPI_RXOF_INT	(1U << 8)
#define SPI_TXFULL_INT	(1U << 6)
#define SPI_TXEMPT_INT	(1U << 5)
#define SPI_TXREQ_INT	(1U << 4)
#define SPI_RXFULL_INT	(1U << 2)
#define SPI_RXEMPT_INT	(1U << 1)
#define SPI_RXREQ_INT	(1U << 0)
#define SPI_ERROR_INT	(SPI_TXUR_INT|SPI_TXOF_INT|SPI_RXUR_INT|SPI_RXOF_INT)
#define SPI_INTEN_MASK		(0x77|(0x3f<<8))


#define SPI_TXFIFO_RST	(1U << 31)
#define SPI_TXFIFO_TST	(1U << 30)
#define SPI_TXDMAREQ_EN	(1U << 24)
#define SPI_RXFIFO_RST	(1U << 15)
#define SPI_RXFIFO_TST	(1U << 14)
#define SPI_RXDMAREQ_EN	(1U << 8)

#define SPI_MASTER_DUAL	(1U << 28)

#define SPI_BIT_EXCHANGE 	(1U << 31)
#define SPI_TBC_INT			(1U	<< 24)
#define	SPI_TBCS			(1U	<< 25)
#define SPI_SET_SS_1_BIT	(1U << 	7)
#define SPI_SS_ACTIVE_BIT	(1U	<<	5)
#define SPI_3W_BIT_EN		(2U <<  0)
#define SPI_STDAD_BIT_EN	(3U <<  0)   //Standard SPI
#define SPI_VAR			(R_SPI_REG_BASE + 0x00)
#define SPI_GCR			(R_SPI_REG_BASE + 0x04)
#define SPI_TCR			(R_SPI_REG_BASE + 0x08)
#define SPI_IER			(R_SPI_REG_BASE + 0x10)
#define SPI_ISR			(R_SPI_REG_BASE + 0x14)
#define SPI_FCR			(R_SPI_REG_BASE + 0x18)
#define SPI_FSR			(R_SPI_REG_BASE + 0x1c)
#define SPI_WCR			(R_SPI_REG_BASE + 0x20)
#define SPI_CCR			(R_SPI_REG_BASE + 0x24)
#define SPI_MBC			(R_SPI_REG_BASE + 0x30)
#define SPI_MTC			(R_SPI_REG_BASE + 0x34)
#define SPI_BCC			(R_SPI_REG_BASE + 0x38)


#define SPI_BATC		(R_SPI_REG_BASE + 0x3c)
#define SPI_BA_CCR		(R_SPI_REG_BASE + 0x40)
#define SPI_TBR			(R_SPI_REG_BASE + 0x44)
#define SPI_RBR			(R_SPI_REG_BASE + 0x48)

#define SPI_TXD			(R_SPI_REG_BASE + 0x200)
#define SPI_RXD			(R_SPI_REG_BASE + 0x300)
static int SpiBus = 0;
#define SPI_GPIO_BASE (0xD300B000)

struct ArSpiState {
    uint8_t bitsPerWord;
    uint8_t xferEnable;

    uint16_t rxWord;
    uint16_t txWord;

    bool rxDone;
    bool txDone;

    struct ChainedIsr isrNss;

    bool nssChange;
};
struct ArSpi {
    u32 VAR;
    u32 GCR;
    u32 TCR;
    u32 IER;
    u32 ISR;
    u32 FCR;
    u32 FSR;
    u32 WCR;
    u32 CCR;
	u32 MBC;
    u32 MTC;
    u32 BCC;
	u32 BATC;
	u32 BA_CCR;
	u32 TBR;
	u32 RBR;
	u32 TXD;
	u32 RXD;
};

struct ArSpiCfg {
	struct ArSpi *regs;
    uint32_t clockBus;
    uint32_t clockUnit;

    IRQn_Type irq;
};

struct ArSpiDev {
    struct SpiDevice *base;
    const struct ArSpiCfg *cfg;
    const struct ArSpiBoardCfg *board;
    struct ArSpiState state;

    struct Gpio *miso;
    struct Gpio *mosi;
    struct Gpio *sck;
    struct Gpio *nss;
};
struct ArSpi spi0 = {
	.VAR = SPI_VAR,
    .GCR = SPI_GCR,
    .TCR = SPI_TCR,
    .IER = SPI_IER,
    .ISR = SPI_ISR,
    .FCR = SPI_FCR,
    .FSR = SPI_FSR,
    .WCR = SPI_WCR,
    .CCR = SPI_CCR,
	.MBC = SPI_MBC,
    .MTC = SPI_MTC,
    .BCC = SPI_BCC,
	.BATC = SPI_BATC,
	.BA_CCR = SPI_BA_CCR,
	.TBR = SPI_TBR,
	.RBR = SPI_RBR,
	.TXD = SPI_TXD,
	.RXD = SPI_RXD,
};
static const struct ArSpiCfg mArSpiCfgs[] = {
    [0] = {
		.regs = &spi0,
        .clockBus = CCU_MOD_CLK_R_SPI,
        .clockUnit = CCU_MOD_CLK_R_SPI,

        .irq = R_SPI_IRQn,
    },
};

static struct ArSpiDev mArSpiDevs[ARRAY_SIZE(mArSpiCfgs)];

static inline struct Gpio *arSpiGpioInit(uint32_t gpioNum, enum ArGpioSpeed speed, enum ArGpioAltFunc func)
{
    struct Gpio *gpio = gpioRequest(gpioNum);

    if (gpio)
        gpioConfigAlt(gpio, speed, GPIO_PULL_NONE, GPIO_OUT_PUSH_PULL, func);

    return gpio;
}
static void spi_enable_bus(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->GCR);
	reg_val |= SPI_ENABLE;
	writel(reg_val, cfg->regs->GCR);
}
/* disbale spi bus */
static void spi_disable_bus(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->GCR);
	reg_val &= ~SPI_ENABLE;
	writel(reg_val, cfg->regs->GCR);
}
static s32 spi_set_cs(u32 chipselect,const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->TCR);

	if (chipselect < 4) {
		reg_val &= ~SPI_TC_SS_MASK;/* SS-chip select, clear two bits */
		reg_val |= chipselect << SPI_TC_SS_BIT_POS;/* set chip select */
		writel(reg_val,cfg->regs->TCR);
		return 0;
	}
	else {
		INF("Chip Select set fail! cs = %d\n", chipselect);
		return -1;
	}
}

static void spi_set_ss_level(const struct ArSpiCfg *cfg, u32 level)
{
	u32 rval = readl(cfg->regs->TCR)&(~(1 << 7));
	rval |= level << 7;
	writel(rval, cfg->regs->TCR);
}

/* set master mode */
static void spi_set_master(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->GCR);
	reg_val |= SPI_MASTER;
	writel(reg_val, cfg->regs->GCR);
}

/* SPI Clock Control Register Bit Fields & Masks,default:0x0000_0002 */
#define SPI_CLK_CTL_CDR2		(0xFF <<  0)	/* Clock Divide Rate 2,master mode only : SPI_CLK = AHB_CLK/(2*(n+1)) */
#define SPI_CLK_CTL_CDR1		(0xF  <<  8)	/* Clock Divide Rate 1,master mode only : SPI_CLK = AHB_CLK/2^n */
#define SPI_CLK_CTL_DRS			(0x1  << 12)	/* Divide rate select,default,0:rate 1;1:rate 2 */
#define SPI_CLK_SCOPE			(SPI_CLK_CTL_CDR2+1)

/* set spi clock */
static void spi_set_clk(const struct ArSpiCfg *cfg, u32 clk)
{
	u32 reg_val;
	u32 mclk = 24000000;
	u32 div_clk;


	ccu_set_mclk_src(CCU_MOD_CLK_R_SPI, CCU_SYS_CLK_16M);
	mclk = ccu_get_sclk_freq(CCU_SYS_CLK_SPI);

	div_clk = mclk / ( clk << 1);

	reg_val = readl(cfg->regs->CCR);

	/* CDR2 */
	if(div_clk <= SPI_CLK_SCOPE) {
		if (div_clk != 0) {
			div_clk--;
		}
		reg_val &= ~SPI_CLK_CTL_CDR2;
		reg_val |= (div_clk | SPI_CLK_CTL_DRS);
	}/* CDR1 */
	else {
		div_clk = 0;
		while(mclk > clk){
			div_clk++;
			mclk >>= 1;
		}
		reg_val &= ~(SPI_CLK_CTL_CDR1 | SPI_CLK_CTL_DRS);
		reg_val |= (div_clk << 8);
	}
	writel(reg_val, cfg->regs->CCR);
}
/* soft reset spi controller */
static void spi_soft_reset(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->GCR);
	reg_val |= SPI_SOFT_RST;
	writel(reg_val, cfg->regs->GCR);
}

/* enable irq type */
static void spi_enable_irq(u32 bitmap, const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->IER);
	bitmap &= SPI_INTEN_MASK;
	reg_val |= bitmap;
	writel(reg_val,cfg->regs->IER);
}

/* disable irq type */
static void spi_disable_irq(u32 bitmap, const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->IER);
	bitmap &= SPI_INTEN_MASK;
	reg_val &= ~bitmap;
	writel(reg_val,cfg->regs->IER);
}

static inline void arSpiDataPullMode(struct ArSpiDev *pdev, enum ArGpioSpeed dataSpeed, enum GpioPullMode dataPull)
{
    gpioConfigAlt(pdev->miso, dataSpeed, dataPull, GPIO_OUT_PUSH_PULL, pdev->board->gpioFunc);
    gpioConfigAlt(pdev->mosi, dataSpeed, dataPull, GPIO_OUT_PUSH_PULL, pdev->board->gpioFunc);
}

static inline void arSpiSckPullMode(struct ArSpiDev *pdev, enum ArGpioSpeed sckSpeed, enum GpioPullMode sckPull)
{
    gpioConfigAlt(pdev->sck, sckSpeed, sckPull, GPIO_OUT_PUSH_PULL, pdev->board->gpioFunc);
}
/* config spi */
static void spi_config_tc(u32 master,const struct SpiMode *mode,const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->TCR);

	/*1. CPOL */
	if (mode->cpol == SPI_CPOL_IDLE_HI)
		reg_val |= SPI_TC_CPOL;/*default POL = 1 */
	else
		reg_val &= ~SPI_TC_CPOL;

	/*2. PHA */
	if (mode->cpha == SPI_CPHA_TRAILING_EDGE)
		reg_val |= SPI_TC_CPHA;/*default PHA = 1 */
	else
		reg_val &= ~SPI_TC_CPHA;

#if 0
	/*3. SPOL,chip select signal polarity */
	if (mode->spol == SPI_SPOL_IDLE_HI)
		reg_val &= ~SPI_TC_SPOL;
	else
		reg_val |= SPI_TC_SPOL;/*default SSPOL = 1,Low level effective */
#endif

	/*4. LMTF--LSB/MSB transfer first select */
	if (mode->format == SPI_FORMAT_LSB_FIRST)
		reg_val |= SPI_TC_FBS;
	else
		reg_val &= ~SPI_TC_FBS;/*default LMTF =0, MSB first */

#if 0
	/*master mode: set DDB,DHB,SMC,SSCTL*/
	if(master == 1) {
		/*5. dummy burst type */
		reg_val &= ~SPI_TC_DDB;/*default DDB =0, ZERO */

		/*6.discard hash burst-DHB */
		reg_val |= SPI_TC_DHB;/*default DHB =1, discard unused burst */

		/*7. set SMC = 1 , SSCTL = 0 ,TPE = 1 */
		reg_val &= ~SPI_TC_SSCTL;
	}
#endif
	writel(reg_val,cfg->regs->TCR);
}
#if 0
static void spi_set_dual_read(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->BCC);
	reg_val |= SPI_BCC_DUAL_MOD_RX_EN;
	writel(reg_val, cfg->regs->BCC);
}
#endif
static inline int arSpiEnable(struct ArSpiDev *pdev,
        const struct SpiMode *mode, bool master)
{
    struct ArSpiState *state = &pdev->state;
	const struct ArSpiCfg *cfg = pdev->cfg;
    if (master) {
        if (!mode->speed)
            return -EINVAL;

        //ccu_set_mclk_onoff(CCU_MOD_CLK_R_SPI, CCU_CLK_ON);
		//ccu_set_mclk_reset(CCU_MOD_CLK_R_SPI, CCU_CLK_NRESET);

    }
	spi_enable_bus(cfg);
	spi_soft_reset(cfg);
    atomicWriteByte(&state->xferEnable, false);

    state->txWord = mode->txWord;
    state->bitsPerWord = mode->bitsPerWord;
   	spi_config_tc(1,mode,cfg);
	spi_enable_irq(SPI_TC_INT|SPI_ERROR_INT,cfg);
    return 0;
}
/* enable transmit pause */
static void spi_enable_tp(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->GCR);
	reg_val |= SPI_TXPAUSE_EN;
	writel(reg_val, cfg->regs->GCR);
}
/* set ss control */
static void spi_ss_ctrl(const struct ArSpiCfg *cfg, u32 on_off)
{
	u32 reg_val = readl(cfg->regs->TCR);
	on_off &= 0x1;
	if(on_off)
		reg_val |= SPI_SS_MANUAL;//owner
	else
		reg_val &= ~SPI_SS_MANUAL;
	writel(reg_val, cfg->regs->TCR);
}

#define SPI_FIFO_CTL_RX_LEVEL	(0xFF <<  0)
#define SPI_FIFO_CTL_TX_LEVEL	(0xFF << 16)

/* reset fifo */
static void spi_reset_fifo(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->FCR);
	reg_val |= (SPI_RXFIFO_RST|SPI_TXFIFO_RST);

	/* Set the trigger level of RxFIFO/TxFIFO. */
	reg_val &= ~(SPI_FIFO_CTL_RX_LEVEL|SPI_FIFO_CTL_TX_LEVEL);
	reg_val |= (0x20<<16) | 0x20;
	writel(reg_val, cfg->regs->FCR);
}

/* set transfer total length BC, transfer length TC and single transmit length STC */
static void spi_set_bc_tc_stc(u32 tx_len, u32 rx_len, u32 stc_len, u32 dummy_cnt, const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->MBC);
	reg_val &= ~SPI_BC_CNT_MASK;
	reg_val |= (SPI_BC_CNT_MASK & (tx_len + rx_len + dummy_cnt));
	writel(reg_val, cfg->regs->MBC);
	//SPI_DBG("\n-- BC = %d --\n", readl(base_addr + SPI_BURST_CNT_REG));

	reg_val = readl(cfg->regs->MTC);
	reg_val &= ~SPI_TC_CNT_MASK;
	reg_val |= (SPI_TC_CNT_MASK & tx_len);
	writel(reg_val, cfg->regs->MTC);
	//SPI_DBG("\n-- TC = %d --\n", readl(base_addr + SPI_TRANSMIT_CNT_REG));

	reg_val = readl(cfg->regs->BCC);
	reg_val &= ~SPI_BCC_STC_MASK;
	reg_val |= (SPI_BCC_STC_MASK & stc_len);
	reg_val &= ~(0xf << 24);
	reg_val |= (dummy_cnt << 24);
	writel(reg_val, cfg->regs->BCC);
	//SPI_DBG("\n-- STC = %d --\n", readl(base_addr + SPI_BCC_REG));
}

static int arSpiMasterStartSync(struct SpiDevice *dev, spi_cs_t cs,
        const struct SpiMode *mode)
{
    struct ArSpiDev *pdev = dev->pdata;
	//const struct ArSpiCfg *cfg = pdev->cfg;
    int err = arSpiEnable(pdev, mode, true);
    if (err < 0)
		return err;

    arSpiDataPullMode(pdev, pdev->board->gpioSpeed, pdev->board->gpioPull);
    arSpiSckPullMode(pdev, pdev->board->gpioSpeed, mode->cpol ? GPIO_PULL_UP : GPIO_PULL_DOWN);

    if (!pdev->nss)
		pdev->nss = gpioRequest(cs);

    if (!pdev->nss)
		return -ENODEV;
    gpioConfigOutput(pdev->nss, pdev->board->gpioSpeed, pdev->board->gpioPull, GPIO_OUT_PUSH_PULL, 1);

    return 0;
}

static int arSpiSlaveStartSync(struct SpiDevice *dev,
        const struct SpiMode *mode)
{
    struct ArSpiDev *pdev = dev->pdata;

    arSpiDataPullMode(pdev, pdev->board->gpioSpeed, GPIO_PULL_NONE);
    arSpiSckPullMode(pdev, pdev->board->gpioSpeed, GPIO_PULL_NONE);

    if (!pdev->nss)
        pdev->nss = arSpiGpioInit(pdev->board->gpioNss, pdev->board->gpioSpeed, pdev->board->gpioFunc);
    if (!pdev->nss)
        return -ENODEV;

    return arSpiEnable(pdev, mode, false);
}

static inline bool arSpiIsMaster(struct ArSpiDev *pdev)
{
	const struct ArSpiCfg *cfg = pdev->cfg;
	return (readl(cfg->regs->GCR)&SPI_MASTER);
}

static void arSpiDone(struct ArSpiDev *pdev, int err)
{
    struct ArSpiState *state = &pdev->state;
	const struct ArSpiCfg *cfg = pdev->cfg;
    if (pdev->board->sleepDev >= 0)
        platReleaseDevInSleepMode(pdev->board->sleepDev);

    if (arSpiIsMaster(pdev)) {
        if (state->nssChange && pdev->nss){
			spi_set_ss_level(cfg,1);
			gpioSet(pdev->nss,1);
        }
        spiMasterRxTxDone(pdev->base, err);
    } else {
        spiSlaveRxTxDone(pdev->base, err);
    }
}
#if 0
static void arSpiRxDone(void *cookie, uint16_t bytesLeft, int err)
{
    struct ArSpiDev *pdev = cookie;
    struct ArSpiState *state = &pdev->state;
	INF("aw==== rx done %d\n",state->txDone);
    state->rxDone = true;

        atomicWriteByte(&state->xferEnable, false);
        arSpiDone(pdev, err);
}

static void arSpiTxDone(void *cookie, uint16_t bytesLeft, int err)
{
    struct ArSpiDev *pdev = cookie;
    struct ArSpiState *state = &pdev->state;
	state->txDone = true;
	INF("aw==== tx done : rxDone : %d\n",state->rxDone);
        atomicWriteByte(&state->xferEnable, false);
        arSpiDone(pdev, err);
}
#endif
/* start spi transfer */
static void spi_start_xfer(const struct ArSpiCfg *cfg)
{
	u32 reg_val = readl(cfg->regs->TCR);
	reg_val |= SPI_EXCHANGE;
	writel(reg_val,cfg->regs->TCR);
}
/* SPI FIFO Status Register Bit Fields & Masks,defualt value:0x0000_0000 */
#define SPI_FIFO_STA_RX_CNT		(0xFF <<  0)	/* rxFIFO counter,how many bytes in rxFIFO */
#define SPI_FIFO_STA_RB_CNT		(0x7  << 12)	/* rxFIFO read buffer counter,how many bytes in rxFIFO read buffer */
#define SPI_FIFO_STA_RB_WR		(0x1  << 15)	/* rxFIFO read buffer write enable */
#define SPI_FIFO_STA_TX_CNT		(0xFF << 16)	/* txFIFO counter,how many bytes in txFIFO */
#define SPI_FIFO_STA_TB_CNT		(0x7  << 28)	/* txFIFO write buffer counter,how many bytes in txFIFO write buffer */
#define SPI_FIFO_STA_TB_WR		(0x1  << 31)	/* txFIFO write buffer write enable */
#define SPI_RXCNT_BIT_POS		(0)
#define SPI_TXCNT_BIT_POS		(16)
#if 1
/* query txfifo bytes */
static u32 spi_query_txfifo(const struct ArSpiCfg *cfg)
{
	u32 reg_val = (SPI_FIFO_STA_TX_CNT & readl(cfg->regs->FSR));
	reg_val >>= SPI_TXCNT_BIT_POS;
	return reg_val;
}
#endif
/* query rxfifo bytes */
static u32 spi_query_rxfifo(const struct ArSpiCfg *cfg)
{
	u32 reg_val = (SPI_FIFO_STA_RX_CNT & readl(cfg->regs->FSR));
	reg_val >>= SPI_RXCNT_BIT_POS;

	return reg_val;
}
#define SPI_INT_STA_MASK		(0x77|(0x3f<<8))

/* clear irq pending */
static void spi_clr_irq_pending(u32 pending_bit, const struct ArSpiCfg *cfg)
{
	pending_bit &= SPI_INT_STA_MASK;
	writel(pending_bit, cfg->regs->ISR);
}

static void arSpiDebugReg(const struct ArSpiCfg *cfg)
{

	ERR("VAR reg: 0x%x,value : 0x%x\n",cfg->regs->VAR,readl(cfg->regs->VAR));
	ERR("GCR reg: 0x%x,value : 0x%x\n",cfg->regs->GCR,readl(cfg->regs->GCR));
	ERR("TCR reg: 0x%x,value : 0x%x\n",cfg->regs->TCR,readl(cfg->regs->TCR));
	ERR("IER reg: 0x%x,value : 0x%x\n",cfg->regs->IER,readl(cfg->regs->IER));
	ERR("ISR reg: 0x%x,value : 0x%x\n",cfg->regs->ISR,readl(cfg->regs->ISR));
	ERR("FCR reg: 0x%x,value : 0x%x\n",cfg->regs->FCR,readl(cfg->regs->FCR));

	ERR("FSR reg: 0x%x,value : 0x%x\n",cfg->regs->FSR,readl(cfg->regs->FSR));
	ERR("WCR reg: 0x%x,value : 0x%x\n",cfg->regs->WCR,readl(cfg->regs->WCR));
	ERR("CCR reg: 0x%x,value : 0x%x\n",cfg->regs->CCR,readl(cfg->regs->CCR));
	ERR("MBC reg: 0x%x,value : 0x%x\n",cfg->regs->MBC,readl(cfg->regs->MBC));
	ERR("MTC reg: 0x%x,value : 0x%x\n",cfg->regs->MTC,readl(cfg->regs->MTC));
	ERR("BCC reg: 0x%x,value : 0x%x\n",cfg->regs->BCC,readl(cfg->regs->BCC));
	ERR("TXD reg: 0x%x,value : 0x%x\n",cfg->regs->TXD,readl(cfg->regs->BCC));
	ERR("1f0 :0x%x\n",readl(0xD70101f0));
	ERR("1fc :0x%x\n",readl(0xD70101fc));
	ERR("gpio :0x%x\n",readl(R_PIO_REG_BASE+0x04));

}

static void arSpiMasterSetup(const struct SpiMode *mode, const struct ArSpiCfg *cfg)
{
	spi_config_tc(1,mode,cfg);
	spi_enable_tp(cfg);
	/* write 1 to clear 0 */
	spi_clr_irq_pending(SPI_INT_STA_MASK, cfg);
	spi_reset_fifo(cfg);
}

static int arSpiRxTx(struct SpiDevice *dev, void *rxBuf, const void *txBuf,
        size_t size, const struct SpiMode *mode)
{
    struct ArSpiDev *pdev = dev->pdata;
    struct ArSpiState *state = &pdev->state;
	const struct ArSpiCfg *cfg = pdev->cfg;
	u32 rcnt;
	char *tmp_ptr;
	u32 i = 0;
	u32 tx_size;
	u32 rx_size;
	u32 dummy_size = 0;
	u32 poll_time = 0xffffffff;
	u32 rw_flag = ((char *)txBuf)[0]>>7;

    if (atomicXchgByte(&state->xferEnable, true) == true){
		INF("spi is busy\n");
        return -EBUSY;
	}
    if (arSpiIsMaster(pdev) && pdev->nss){
		gpioSet(pdev->nss,0);
        spi_set_ss_level(cfg,0);
	}

    state->rxDone = false;
    state->txDone = false;
    state->nssChange = mode->nssChange;

    //read
	if(rw_flag){
		/*step 1: send read addr; single half duplex TX mode*/
		tx_size = 1;
		rx_size = 0;
		arSpiMasterSetup(mode,cfg);
		spi_set_bc_tc_stc(tx_size, rx_size, tx_size, dummy_size, cfg);
		spi_enable_irq(SPI_TC_INT|SPI_ERROR_INT,cfg);
		if(txBuf){
			tmp_ptr = (char *)txBuf;
			rcnt = tx_size;
			spi_start_xfer(cfg);
			while(rcnt--){
				writeb(tmp_ptr[i++], cfg->regs->TXD);
				udelay(20);		//should delay 10 us     checked!
			}
			while(spi_query_txfifo(cfg)&&(--poll_time > 0) );/* txFIFO counter */
			if(poll_time <= 0) {
				ERR("aw==== cpu transfer data time out!,tx fifo:%d, tx data:%x\n",
					spi_query_txfifo(cfg), tmp_ptr[0]);
				arSpiDebugReg(cfg);
				atomicWriteByte(&state->xferEnable, false);
				return -1;
			}
		} else {
			ERR("spi read without address !!\n");
			atomicWriteByte(&state->xferEnable, false);
			return -1;
		}
		/*step 2: read data; single half duplex RX mode*/
		tx_size = 0;
		rx_size = size -1;
		arSpiMasterSetup(mode,cfg);
		spi_set_bc_tc_stc(tx_size, rx_size, tx_size, dummy_size, cfg);
		spi_enable_irq(SPI_TC_INT|SPI_ERROR_INT,cfg);
		if(rxBuf) {
			tmp_ptr = (char *)rxBuf;
			rcnt = rx_size;
			spi_start_xfer(cfg);
			poll_time = 0xffffffff;
			while(rcnt &&(--poll_time > 0)) {
				if(spi_query_rxfifo(cfg) ) {
					tmp_ptr[i++] =  readb(cfg->regs->RXD);//fetch data
					rcnt--;
				}
				if(poll_time <= 0) {
					ERR("cpu transfer data time out!,rx fifo : %d\n",spi_query_rxfifo(cfg));
					arSpiDebugReg(cfg);
					atomicWriteByte(&state->xferEnable, false);
					return -1;
				}
			}
	    } else {
	        state->rxDone = true;
	    }
	}else {
		tx_size = size;
		rx_size = 0;
		arSpiMasterSetup(mode,cfg);
		spi_set_bc_tc_stc(tx_size, rx_size, tx_size, dummy_size, cfg);
		spi_enable_irq(SPI_TC_INT|SPI_ERROR_INT,cfg);
		if(txBuf){
			tmp_ptr = (char *)txBuf;
			rcnt = size;
			spi_start_xfer(cfg);
			while(rcnt--){
				writeb(tmp_ptr[i++], cfg->regs->TXD);
				udelay(20);
			}
			while(spi_query_txfifo(cfg)&&(--poll_time > 0) );/* txFIFO counter */
			if(poll_time <= 0) {
				ERR("cpu transfer data time out!,tx fifo:%d, tx data:%x\n",
					spi_query_txfifo(cfg), tmp_ptr[--i]);
				arSpiDebugReg(cfg);
				atomicWriteByte(&state->xferEnable, false);
				return -1;
			}
		}
	}
	atomicWriteByte(&state->xferEnable, false);
	arSpiDone(pdev, 0);
    return 0;
}


static int arSpiSlaveIdle(struct SpiDevice *dev, const struct SpiMode *mode)
{
    struct ArSpiDev *pdev = dev->pdata;
    struct ArSpiState *state = &pdev->state;

    if (atomicXchgByte(&state->xferEnable, true) == true)
        return -EBUSY;

    atomicXchgByte(&state->xferEnable, false);
    return 0;
}

static inline void arSpiDisable(struct SpiDevice *dev, bool master)
{
    struct ArSpiDev *pdev = dev->pdata;
	const struct ArSpiCfg *cfg = pdev->cfg;
	u32 rval = readl(cfg->regs->GCR)&(~SPI_ENABLE);
	writel(rval, cfg->regs->GCR);
    if (master) {
        arSpiSckPullMode(pdev, pdev->board->gpioSpeed, pdev->board->gpioPull);
    }

}

static int arSpiMasterStopSync(struct SpiDevice *dev)
{
    struct ArSpiDev *pdev = dev->pdata;
	const struct ArSpiCfg *cfg = pdev->cfg;
    if (pdev->nss) {
		spi_set_ss_level(cfg,1);
		gpioSet(pdev->nss,1);
        gpioRelease(pdev->nss);
    }

    arSpiDisable(dev, true);
    pdev->nss = NULL;
    return 0;
}

static int arSpiSlaveStopSync(struct SpiDevice *dev)
{
    struct ArSpiDev *pdev = dev->pdata;

    if (pdev->nss)
        gpioRelease(pdev->nss);

    arSpiDisable(dev, false);
    pdev->nss = NULL;
    return 0;
}

static bool arSpiExtiIsr(struct ChainedIsr *isr)
{
    struct ArSpiState *state = container_of(isr, struct ArSpiState, isrNss);
    struct ArSpiDev *pdev = container_of(state, struct ArSpiDev, state);

    if (pdev->nss)
        return false;

    spiSlaveCsInactive(pdev->base);
    return true;
}

static void arSpiSlaveSetCsInterrupt(struct SpiDevice *dev, bool enabled)
{
	struct ArSpiDev *pdev = dev->pdata;
    struct ChainedIsr *isr = &pdev->state.isrNss;
	if(enabled){
		arSpiExtiIsr(isr);
	}

}

static bool arSpiSlaveCsIsActive(struct SpiDevice *dev)
{
    struct ArSpiDev *pdev = dev->pdata;
    return pdev->nss && !gpioGet(pdev->nss);
}


static s32 spi_int_handler(void *parg){
	int id = *(int *)parg;
	struct ArSpiDev *pdev = &mArSpiDevs[id];
	const struct ArSpiCfg *cfg = pdev->cfg;
	u32 isr = readl(cfg->regs->ISR);
	/* check error */
	if(isr & SPI_TC_INT) {
		//INF("aw==== irq coming\n");
		spi_disable_irq(SPI_TC_INT | SPI_ERROR_INT, cfg);

		return 0;
	} else if (isr & (SPI_ERROR_INT)) {

		spi_disable_irq(SPI_TC_INT | SPI_ERROR_INT, cfg);
		spi_soft_reset(cfg);
		INF("aw==== irq error\n");
		goto out;
	}
out:
	writel(isr, cfg->regs->ISR);
	return 0;
}

static int arSpiRelease(struct SpiDevice *dev)
{
    struct ArSpiDev *pdev = dev->pdata;
	const struct ArSpiCfg *cfg = pdev->cfg;
	uninstall_isr(R_SPI_IRQn,spi_int_handler);
	spi_disable_bus(cfg);
    pdev->base = NULL;
    return 0;
}

const struct SpiDevice_ops mArSpiOps = {
    .masterStartSync = arSpiMasterStartSync,
    .masterRxTx = arSpiRxTx,
    .masterStopSync = arSpiMasterStopSync,

    .slaveStartSync = arSpiSlaveStartSync,
    .slaveIdle = arSpiSlaveIdle,
    .slaveRxTx = arSpiRxTx,
    .slaveStopSync = arSpiSlaveStopSync,

    .slaveSetCsInterrupt = arSpiSlaveSetCsInterrupt,
    .slaveCsIsActive = arSpiSlaveCsIsActive,

    .release = arSpiRelease,
};


static void arSpiInit(struct ArSpiDev *pdev, const struct ArSpiCfg *cfg,
        const struct ArSpiBoardCfg *board, struct SpiDevice *dev)
{
	//struct ArSpi *regs = pdev->cfg->regs;
    pdev->miso = arSpiGpioInit(board->gpioMiso, board->gpioSpeed, board->gpioFunc);
    pdev->mosi = arSpiGpioInit(board->gpioMosi, board->gpioSpeed, board->gpioFunc);
    pdev->sck = arSpiGpioInit(board->gpioSclk, board->gpioSpeed, board->gpioFunc);
	pdev->nss= arSpiGpioInit(board->gpioNss, board->gpioSpeed, board->gpioFunc);
	//FIX ME clk

	//writel(0x10001,0xD70101fc);
	//writel(0x47477744,SPI_GPIO_BASE+0xc0);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_SPI, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_R_SPI, CCU_CLK_NRESET);
	writel(0x80000000,0xD70101f0);
	/* 1. enable the spi module */
	spi_enable_bus(cfg);
	/* 2. set the default chip select */
	spi_set_cs(0,cfg);
	/*
	 * 3. master: set spi module clock;
	 * 4. set the default frequency	6MHz
	*/
	spi_set_master(cfg);
	spi_set_clk(cfg, 6000000);
	/*
	 * 5. master : set POL,PHA,SSOPL,LMTF,DDB,DHB; default: SSCTL=0,SMC=1,TBW=0.
	 * 6. set bit width-default: 8 bits
	*/
	spi_enable_tp(cfg);
	/* 7. manual control the chip select */
	spi_ss_ctrl(cfg, 1);
	/* 8. reset fifo */
	spi_reset_fifo(cfg);
    pdev->base = dev;
    pdev->cfg = cfg;
    pdev->board = board;
}

int spiRequest(struct SpiDevice *dev, uint8_t busId)
{
    if (busId >= ARRAY_SIZE(mArSpiDevs))
        return -ENODEV;

    const struct ArSpiBoardCfg *board = boardArSpiCfg(busId);
    if (!board)
        return -ENODEV;
	SpiBus = busId;
    struct ArSpiDev *pdev = &mArSpiDevs[busId];
    const struct ArSpiCfg *cfg = &mArSpiCfgs[busId];
    if (!pdev->base)
        arSpiInit(pdev, cfg, board, dev);

	install_isr(board->irqNss, spi_int_handler, &SpiBus);
	interrupt_enable(board->irqNss);
    memset(&pdev->state, 0, sizeof(pdev->state));
    dev->ops = &mArSpiOps;
    dev->pdata = pdev;
    return 0;
}
