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
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <i2c.h>
#include <seos.h>
#include <util.h>
#include <gpio.h>
#include <atomicBitset.h>
#include <atomic.h>
#include <platform.h>

#include <plat/inc/cmsis.h>
#include <plat/inc/gpio.h>
#include <plat/inc/i2c.h>
#include <plat/inc/plat.h>
#include <plat/inc/include.h>
#include <cpu.h>


#include <cpu/inc/barrier.h>
#if SM_VERBOSE_DEBUG
#define sm_log_debug(x) osLog(LOG_DEBUG, x "\n")
#else
#define sm_log_debug(x) do {} while(0)
#endif


#define I2C_VERBOSE_DEBUG       0
#define I2C_MAX_QUEUE_DEPTH     8

#define I2C_OK	0
#define I2C_FAIL	-1

#if I2C_VERBOSE_DEBUG
#define i2c_log_debug(x) osLog(LOG_DEBUG, x "\n")
#else
#define i2c_log_debug(x) do {} while(0)
#endif

#define TEST_TWI_NO		(4)

#define TWIC_USE_INT	0

//#define TWI3_PB56

#define TWIC_NUM    	7	//twi01234+r_twi01
#define TWI_MASTER  	1

#define TWIC_OS      0x400
//#define INT_TWI      (twic_no == 5 ? NVIC_SRC_TWI0 : (twic_no == 6 ?NVIC_SRC_TWI1:(NVIC_SRC_TWI0 + twic_no)))
//#define TWI_BASE     (twic_no == 5 ? R_TWI0_BASE : (twic_no == 6 ?R_TWI1_BASE:(TWI0_BASE + TWIC_OS * twic_no)))

/* slave address register bit field */
#define _10BITADDR_FIX  0x11110000
#define GCALL_ENB         1
#define I2C_EFR_MASK        (0x3<<0)/* 00:no,01: 1byte, 10:2 bytes, 11: 3bytes */

/* control register bit field */
#define I2C_INT_ENB     (1<<7)
#define I2C_BUS_ENB     (1<<6)
#define I2C_MSTART      (1<<5)
#define I2C_MSTOP       (1<<4)
#define I2C_INTFLAG     (1<<3)
#define I2C_AACK        (1<<2)
#define I2C_LCR_WMASK	(I2C_MSTART|I2C_MSTOP|I2C_INTFLAG)
#define I2C_SADDR_ADD7(x)    (((x) & I2C_SADDR_ADD7_MASK) << 1)
#define I2C_SADDR_ADD7_MASK  0x7F


/* TWI Status Register Bit Fields & Masks  */
#define TWI_STAT_MASK                   (0xff)
/* 7:0 bits use only,default is 0xF8 */
#define TWI_STAT_BUS_ERR				(0x00) 	/* BUS ERROR */
/* Master mode use only */
#define I2C_STAT_TX_STA					(0x08) 	/* START condition transmitted */
#define I2C_STAT_TX_RESTA				(0x10) 	/* Repeated START condition transmitted */
#define I2C_STAT_TX_AW_ACK				(0x18) 	/* Address+Write bit transmitted, ACK received */
#define I2C_STAT_TX_AW_NAK				(0x20) 	/* Address+Write bit transmitted, ACK not received */
#define I2C_STAT_TXD_ACK				(0x28) 	/* data byte transmitted in master mode,ack received */
#define I2C_STAT_TXD_NAK				(0x30) 	/* data byte transmitted in master mode ,ack not received */
#define I2C_STAT_ARBLOST				(0x38) 	/* arbitration lost in address or data byte */
#define I2C_STAT_TX_AR_ACK				(0x40) 	/* Address+Read bit transmitted, ACK received */
#define I2C_STAT_TX_AR_NAK				(0x48) 	/* Address+Read bit transmitted, ACK not received */
#define I2C_STAT_RXD_ACK				(0x50) 	/* data byte received in master mode ,ack transmitted */
#define I2C_STAT_RXD_NAK				(0x58) 	/* date byte received in master mode,not ack transmitted */
#define I2C_STAT_IDLE					(0xF8) 	/* No relevant status infomation,INT_FLAG = 0 */

#define I2C_LCR_IDLE_STATUS				(0x3A)
#define I2C_LCR_NORM_STATUS				(0x30) /* normal status */


//static u8 iic_mem[TWI_S_MEM_LEN];
static u32 slave_reg_addr;
static int bus0_id = 0;
static int bus1_id = 0;
static u8 mLock = 0;


struct ArI2c {
    u32 SAR;     //2-Wire Slave Address Register
    u32 EAR;  	  //2-Wire Extend Address Register
    u32 DTR;      //2-Wire Data Register
    u32 CTR;      //2-Wire Control Register
    u32 STR;      //2-Wire Status Register
	u32 CKR;       //2-Wire Clock Register
    u32 SRR;      //2-Wire Soft-Reset Register
    u32 EFR;       //2-Wire Enhance Feature Register
    u32 LCR;       //2-Wire line control register
    u32 DVFSCR;   //2-Wire DVFS control register
};

enum ArI2cMasterState
{
    AR_I2C_MASTER_IDLE,
    AR_I2C_MASTER_START,
    AR_I2C_MASTER_RUNNING,
};

struct I2cArState {
    struct {
        union {
            uint8_t *buf;
            const uint8_t *cbuf;
            uint8_t byte;
        };
        size_t size;
        size_t offset;
        bool preamble;

        I2cCallbackF callback;
        void *cookie;
    } rx, tx;

    enum {
        AR_I2C_DISABLED,
        AR_I2C_SLAVE,
        AR_I2C_MASTER,
    } mode;

    enum {
		AR_I2C_S_IDLE,
		AR_I2C_S_SETADDR,
		AR_I2C_S_RCV_SADDR,
		AR_I2C_S_BADDR,
		AR_I2C_S_WDATA,
		AR_I2C_S_RDATA
    } slaveState;

    // ArI2cSpiMasterState
    uint8_t masterState;
    uint16_t tid;
};

struct ArI2cCfg {
    struct ArI2c *regs;
	IRQn_Type irqEv;
    uint32_t clock;
};

struct ArI2cDev {
    const struct ArI2cCfg *cfg;
    const struct ArI2cBoardCfg *board;
    struct I2cArState state;

    uint32_t next;
    uint32_t last;

    struct Gpio *scl;
    struct Gpio *sda;

    uint8_t addr;
};


struct ArI2c i2c0 = {
	.SAR = R_TWI1_REG_BASE,
	.EAR = R_TWI1_REG_BASE + 0x04,
    .DTR = R_TWI1_REG_BASE + 0x08,
    .CTR = R_TWI1_REG_BASE + 0x0c,
    .STR = R_TWI1_REG_BASE + 0x10,
	.CKR = R_TWI1_REG_BASE + 0x14,
    .SRR = R_TWI1_REG_BASE + 0x18,
    .EFR = R_TWI1_REG_BASE + 0x1c,
    .LCR = R_TWI1_REG_BASE + 0x20,
    .DVFSCR = R_TWI2_REG_BASE + 0xFC,
};
struct ArI2c i2c1 = {
	.SAR = R_TWI2_REG_BASE,
	.EAR = R_TWI2_REG_BASE + 0x04,
    .DTR = R_TWI2_REG_BASE + 0x08,
    .CTR = R_TWI2_REG_BASE + 0x0c,
    .STR = R_TWI2_REG_BASE + 0x10,
	.CKR = R_TWI2_REG_BASE + 0x14,
    .SRR = R_TWI2_REG_BASE + 0x18,
    .EFR = R_TWI2_REG_BASE + 0x1c,
    .LCR = R_TWI2_REG_BASE + 0x20,
    .DVFSCR = R_TWI2_REG_BASE + 0xFC,
};


static const struct ArI2cCfg mArI2cCfgs[] = {
    [0] = {
        .regs = &i2c0,

        .clock = CCU_MOD_CLK_R_TWI,

        .irqEv = R_I2C1_IRQn,
    },
    [1] = {
        .regs = &i2c1,

        .clock = CCU_MOD_CLK_R_TWI,

        .irqEv = R_I2C2_IRQn,
    },
};

static struct ArI2cDev mArI2cDevs[ARRAY_SIZE(mArI2cCfgs)];

struct ArI2cXfer
{
    uint32_t        id;
    const void     *txBuf;
    size_t          txSize;
    void           *rxBuf;
    size_t          rxSize;
    I2cCallbackF    callback;
    void           *cookie;
    uint8_t         busId; /* for us these are both fine in a uint 8 */
    uint8_t         addr;
};

ATOMIC_BITSET_DECL(mXfersValid, I2C_MAX_QUEUE_DEPTH, static);
static struct ArI2cXfer mXfers[I2C_MAX_QUEUE_DEPTH] = { };
static inline struct ArI2cXfer *ArI2cGetXfer(void)
{
    int32_t idx = atomicBitsetFindClearAndSet(mXfersValid);

    if (idx < 0)
        return NULL;
    else
        return mXfers + idx;
}

static inline struct Gpio* arI2cGpioInit(const struct ArI2cBoardCfg *board, const struct ArI2cGpioCfg *cfg)
{
    struct Gpio* gpio = gpioRequest(cfg->gpioNum);
    gpioConfigAlt(gpio, board->gpioSpeed, board->gpioPull, GPIO_OUT_OPEN_DRAIN,
            cfg->func);

    return gpio;
}
/* clear the interrupt flag */
static inline void i2c_disable_irq(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val &= ~I2C_INT_ENB;
	reg_val &= ~(I2C_MSTART|I2C_MSTOP|I2C_INTFLAG);
	writel(reg_val, cfg->regs->CTR);
}

/* enable the interrupt flag */
static inline void i2c_enable_irq(const const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);

	/*
	 * 1 when enable irq for next operation, set intflag to 0 to prevent to clear it by a mistake
	 *   (intflag bit is write-1-to-clear bit)
	 * 2 Similarly, mask START bit and STOP bit to prevent to set it twice by a mistake
	 *   (START bit and STOP bit are self-clear-to-0 bits)
	 */
	reg_val |= I2C_INT_ENB;
	reg_val &= ~(I2C_MSTART|I2C_MSTOP|I2C_INTFLAG);
	writel(reg_val, cfg->regs->CTR);
}
static __inline void arI2cClearInt(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
    /* start and stop bit should be 0 */
	reg_val |= I2C_INTFLAG;
	reg_val &= ~(I2C_MSTOP | I2C_MSTART);
	writel(reg_val ,cfg->regs->CTR);
	/* read two more times to make sure that interrupt flag does really be cleared */
	{
		unsigned int temp;
		temp = readl(cfg->regs->CTR);
		temp |= readl(cfg->regs->CTR);
	}

}

static inline void arI2cEnable(const struct ArI2cCfg *cfg){
		writel(readl(cfg->regs->CTR)| (I2C_BUS_ENB), cfg->regs->CTR);
}
static inline void arI2cDisable(const struct ArI2cCfg *cfg){
	u32 rval = readl(cfg->regs->CTR);
	writel(rval&(~I2C_BUS_ENB), cfg->regs->CTR);
}
/* get start bit status, poll if start signal is sent */
static inline unsigned int I2cGetStart(const struct ArI2cCfg *cfg)
{
	u32 rval = readl(cfg->regs->CTR);
	rval >>= 5;
	return rval & 1;
}
/* get start bit status, poll if start signal is sent */
static inline void I2cSetStart(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val |= I2C_MSTART;
	reg_val &= ~I2C_INTFLAG;
	writel(reg_val, cfg->regs->CTR);
}
static inline int arI2cStart(const struct ArI2cCfg *cfg){
	u32 timeout = 65536;
	I2cSetStart(cfg);
	while((1 == I2cGetStart(cfg))&&(--timeout));
	if (timeout == 0) {
		osLog(LOG_ERROR,"START can't sendout!\n");
		return I2C_FAIL;
	}
	return I2C_OK;
}
static int arI2cRestart(const struct ArI2cCfg *cfg)
{
	u32 timeout = 65536;
	I2cSetStart(cfg);
	arI2cClearInt(cfg);
	while((1 == I2cGetStart(cfg))&&(--timeout));
	if (timeout == 0) {
		osLog(LOG_ERROR," Restart can't sendout!\n");
		return I2C_FAIL;
	}
	return I2C_OK;
}
/* get stop bit status, poll if stop signal is sent */
static inline unsigned int i2cGetStop(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val >>= 4;
	return reg_val & 1;
}
static inline void i2cSetStop(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val |= I2C_MSTOP;
	reg_val &= ~I2C_INTFLAG;
	writel(reg_val, cfg->regs->CTR);
}

static inline int arI2cStop(const struct ArI2cCfg *cfg) {
	u32 timeout = 65536;
	i2cSetStop(cfg);
	arI2cClearInt(cfg);

	i2cGetStop(cfg);/* it must delay 1 nop to check stop bit */
	while(( 1 == i2cGetStop(cfg))&& (--timeout));
	if (timeout == 0) {
		osLog(LOG_ERROR,"[i2c] STOP can't sendout!\n");
		return I2C_FAIL;
	}

	timeout = 65536;
	while((I2C_STAT_IDLE != readl(cfg->regs->STR)) && (--timeout));
	if (timeout == 0) {
		osLog(LOG_ERROR,"[i2c] state 0x%x isn't idle\n",readl(cfg->regs->STR));
		return I2C_FAIL;
	}

	timeout = 65536;
	while((I2C_LCR_IDLE_STATUS != readl(cfg->regs->LCR)
		&& I2C_LCR_NORM_STATUS != readl(cfg->regs->LCR))
		&& (--timeout));
	if (timeout == 0) {
		osLog(LOG_ERROR,"[i2c] LCR 0x%x isn't idle\n",readl(cfg->regs->LCR));
		return I2C_FAIL;
	}

	return I2C_OK;
}


static inline void arI2cReset(const struct ArI2cCfg *cfg){
	/* reset twi comtroller */
	writel(0x1,cfg->regs->SRR);
	/* wait twi reset completing */
	while (readl(cfg->regs->SRR));
}

static inline void ArI2cAckEnable(const struct ArI2cCfg *cfg)
{
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val |= I2C_AACK;
	reg_val &= ~I2C_INTFLAG;
	writel(reg_val, cfg->regs->CTR);
}
static inline void ArI2cAckDisable(const struct ArI2cCfg *cfg)
{	
	unsigned int reg_val = readl(cfg->regs->CTR);
	reg_val &= ~I2C_AACK;
	reg_val &= ~I2C_INTFLAG;
	writel(reg_val,cfg->regs->CTR);

}
/* Enhanced Feature Register */
static inline void ArI2cSetEfr(const struct ArI2cCfg *cfg, u32 efr)
{
	u32 reg_val = readl(cfg->regs->EFR);

	reg_val &= ~I2C_EFR_MASK;
	efr     &= I2C_EFR_MASK;
	reg_val |= efr;
	writel(reg_val, cfg->regs->EFR);
}

static __inline void i2c_write_byte(const struct ArI2cCfg *cfg, u8 value)
{
	writel(value, cfg->regs->DTR);
	arI2cClearInt(cfg);
}

static __inline u8 i2c_read_byte(const struct ArI2cCfg *cfg)
{
	u8 buffer ;
	buffer = readl(cfg->regs->DTR);
	arI2cClearInt(cfg);
	return buffer;
}

static __inline u8 i2c_read_last_byte(const struct ArI2cCfg *cfg)
{
	u8 buffer ;
	buffer = readl(cfg->regs->DTR);
	return buffer;
}

static inline struct ArI2cXfer *arI2cGetXfer(void)
{
    int32_t idx = atomicBitsetFindClearAndSet(mXfersValid);

    if (idx < 0)
        return NULL;
    else
        return mXfers + idx;
}
/* twi line control register -default value: 0x0000_003a */
#define TWI_LCR_SDA_EN          (0x01<<0) 	/* SDA line state control enable ,1:enable;0:disable */
#define TWI_LCR_SDA_CTL         (0x01<<1) 	/* SDA line state control bit, 1:high level;0:low level */
#define TWI_LCR_SCL_EN          (0x01<<2) 	/* SCL line state control enable ,1:enable;0:disable */
#define TWI_LCR_SCL_CTL         (0x01<<3) 	/* SCL line state control bit, 1:high level;0:low level */
#define TWI_LCR_SDA_STATE_MASK  (0x01<<4)   /* current state of SDA,readonly bit */
#define TWI_LCR_SCL_STATE_MASK  (0x01<<5)   /* current state of SCL,readonly bit */

/* enable SDA or SCL */
static void twi_enable_lcr(const struct ArI2cCfg *cfg, unsigned int sda_scl)
{
	unsigned int reg_val = readl(cfg->regs->LCR);
	sda_scl &= 0x01;
	if (sda_scl)
		reg_val |= TWI_LCR_SCL_EN;/* enable scl line control */
	else
		reg_val |= TWI_LCR_SDA_EN;/* enable sda line control */

	writel(reg_val, cfg->regs->LCR);
}

/* get SDA state */
static unsigned int twi_get_sda(const struct ArI2cCfg *cfg)
{
	unsigned int status = 0;
	status = TWI_LCR_SDA_STATE_MASK & readl(cfg->regs->LCR);
	status >>= 4;
	return (status&0x1);
}

/* set SCL level(high/low), only when SCL enable */
static void twi_set_scl(const struct ArI2cCfg *cfg, unsigned int hi_lo)
{
	unsigned int reg_val = readl(cfg->regs->LCR);
	reg_val &= ~TWI_LCR_SCL_CTL;
	hi_lo   &= 0x01;
	reg_val |= (hi_lo<<3);
	writel(reg_val, cfg->regs->LCR);
}


/* disable SDA or SCL */
static void twi_disable_lcr(const struct ArI2cCfg *cfg, unsigned int sda_scl)
{
	unsigned int reg_val = readl(cfg->regs->LCR);
	sda_scl &= 0x01;
	if (sda_scl)
		reg_val &= ~TWI_LCR_SCL_EN;/* disable scl line control */
	else
		reg_val &= ~TWI_LCR_SDA_EN;/* disable sda line control */

	writel(reg_val, cfg->regs->LCR);
}

/* send 9 clock to release sda */
static int twi_send_clk_9pulse(const struct ArI2cCfg *cfg, int bus_num)
{
	int twi_scl = 1;
	int low = 0;
	int high = 1;
	int cycle = 0;

	/* enable scl control */
	twi_enable_lcr(cfg, twi_scl);
	INF("scl : 0x%x\n",readl(cfg->regs->LCR));

	while (cycle < 9){
		if (twi_get_sda(cfg)
		    && twi_get_sda(cfg)
		    && twi_get_sda(cfg)) {
			break;
		}

		/* twi_scl -> low */
		twi_set_scl(cfg, low);
		udelay(1000);

		/* twi_scl -> high */
		twi_set_scl(cfg, high);
		udelay(1000);
		cycle++;
	}

	if (twi_get_sda(cfg)) {
		twi_disable_lcr(cfg, twi_scl);
		INF("scl : 0x%x\n",readl(cfg->regs->LCR));
		return I2C_OK;
	} else {
		INF("[i2c%d] SDA is still Stuck Low, failed. \n", bus_num);
		twi_disable_lcr(cfg, twi_scl);
		return I2C_FAIL;
	}
}

static inline void arI2cPutXfer(struct ArI2cXfer *xfer)
{
    if (xfer)
        atomicBitsetClearBit(mXfersValid, xfer - mXfers);
}

static inline void ArI2cInvokeTxCallback(struct I2cArState *state, size_t tx, size_t rx, int err)
{
    uint16_t oldTid = osSetCurrentTid(state->tid);
	if(state->tx.callback)
		state->tx.callback(state->tx.cookie, tx, rx, err);
    osSetCurrentTid(oldTid);
}
static inline void ArI2cSlaveTxDone(struct ArI2cDev *pdev)
{
    struct I2cArState *state = &pdev->state;
    size_t txOffst = state->tx.offset;

    //ArI2cSlaveIdle(pdev);
    ArI2cInvokeTxCallback(state, txOffst, 0, 0);
}
static inline void ArI2cMasterTxRxDone(struct ArI2cDev *pdev, int err)
{
    struct I2cArState *state = &pdev->state;
	//const struct ArI2cCfg *cfg = pdev->cfg;
    size_t txOffst = state->tx.offset;
    size_t rxOffst = state->rx.offset;
    uint32_t id;
    int i;
    struct ArI2cXfer *xfer;

    state->tx.offset = 0;
    state->rx.offset = 0;
	ArI2cInvokeTxCallback(state, txOffst, rxOffst, err);
    do {
        id = atomicAdd32bits(&pdev->next, 1);
    } while (!id);
    for (i=0; i<I2C_MAX_QUEUE_DEPTH; i++) {
        xfer = &mXfers[i];

        if (xfer->busId == (pdev - mArI2cDevs) &&
                atomicCmpXchg32bits(&xfer->id, id, 0)) {
            pdev->addr = xfer->addr;
            state->tx.cbuf = xfer->txBuf;
            state->tx.offset = 0;
            state->tx.size = xfer->txSize;
            state->tx.callback = xfer->callback;
            state->tx.cookie = xfer->cookie;
            state->rx.buf = xfer->rxBuf;
            state->rx.offset = 0;
            state->rx.size = xfer->rxSize;
            state->rx.callback = NULL;
            state->rx.cookie = NULL;
            atomicWriteByte(&state->masterState, AR_I2C_MASTER_START);
            arI2cPutXfer(xfer);
			arI2cRestart(pdev->cfg);
			//osLog(LOG_ERROR,"restart\n");
            return;
        }
    }
	arI2cStop(pdev->cfg);
    atomicWriteByte(&state->masterState, AR_I2C_MASTER_IDLE);

}


static inline void arI2cSpeedSet(const struct ArI2cCfg *cfg,
       uint32_t speed){
    u8 clk_m = 0;
	u8 clk_n = 0;

	u32 sclk_real = 0;
	u32 src_clk = 0;
	u8  pow_clk_n = 1;
	u32 divider = 0;;
	src_clk = ccu_get_sclk_freq(CCU_SYS_CLK_APBS1) / 10;
	divider = src_clk / speed;
	if (divider == 0) {
		clk_m = 1;
		goto out;
	}

	while (clk_n < 8) {
		clk_m = (divider / pow_clk_n) - 1;
		while (clk_m < 16) {
			sclk_real = src_clk / (clk_m + 1) / pow_clk_n;
			if (sclk_real <= speed)
				goto out;
			else
				clk_m++;
		}
		clk_n++;
		pow_clk_n *= 2;
	}
out:
	writel((clk_m << 3) | clk_n, cfg->regs->CKR);

}

static u8 i2c_get_status(const struct ArI2c *regs){
	return  (0xff & readl(regs->STR));
}
static void i2c_process_master_status(void *paras)
{	
	struct ArI2cDev *pdev = paras;
	struct I2cArState *state = &pdev->state;
	const struct ArI2cCfg *cfg = pdev->cfg;
	u8 status = i2c_get_status(cfg->regs);
	//static u8 last_status = 0xf8;

	int  err_code   = I2C_OK;

	//++debug_cnt;
		//if(last_status != status)
		//	osLog(LOG_ERROR,"state->masterState: %d,status : 0x%x,%u\n",state->masterState,status,debug_cnt);
		//last_status = status;
	if(atomicCmpXchgByte((uint8_t *)&mLock,0, 1)) {
	switch (status) {
		case 0xf8: /* On reset or stop the bus is idle, use only at poll method */
			err_code = 0xf8;
			break;
		case 0x08: /* A START condition has been transmitted */
		case 0x10: /* A repeated start condition has been transmitted */
			i2c_write_byte(cfg,pdev->addr);
			//osLog(LOG_ERROR,"read addr %x\n",pdev->addr);
			break;
		case 0xd8:
		case 0x20:
			err_code = 0x20;
			break;
		case 0x18:
		case 0x28:
			if(state->tx.offset < state->tx.size){
				i2c_write_byte(cfg,state->tx.cbuf[state->tx.offset]);
		//		osLog(LOG_ERROR,"write %d\n",state->tx.cbuf[state->tx.offset]);
				state->tx.offset++;
			} else {				// transmit complete
				ArI2cMasterTxRxDone(pdev,0);
			}
			break;
		case 0x40:
			if(state->rx.size > 1) {
				ArI2cAckEnable(cfg);
				arI2cClearInt(cfg);
			} else if (state->rx.size == 1) {
				arI2cClearInt(cfg);
			}
			break;
		case 0x50:
			if(state->rx.offset < state->rx.size){
				if(state->rx.offset + 2 == state->rx.size)
					ArI2cAckDisable(cfg);
				state->rx.buf[state->rx.offset] = i2c_read_byte(cfg);
		//		osLog(LOG_ERROR,"read  %d\n",state->rx.buf[state->rx.offset]);
				state->rx.offset++;
				break;
			}
			err_code = I2C_FAIL;
			break;
		case 0x58:
			if(state->rx.offset == state->rx.size -1) {
				state->rx.buf[state->rx.offset] = i2c_read_last_byte(cfg);
				state->rx.offset++;
		//		osLog(LOG_ERROR,"read done %d\n",state->rx.buf[state->rx.offset]);
				ArI2cMasterTxRxDone(pdev,0);
			}
			break;
		default :
			err_code = status;
			break;
		}
	if(err_code != I2C_OK){};
	//	osLog(LOG_ERROR,"i2c state error code %d\n",err_code);
	atomicWriteByte((uint8_t *)&mLock,0);
	}

	if(AR_I2C_MASTER_IDLE != atomicReadByte(&state->masterState))
		i2c_enable_irq(pdev->cfg);

	return ;
}

static void i2c_process_slave_status(struct ArI2cDev *pdev, u8 status)
{
	struct I2cArState *state = &pdev->state;
	const struct ArI2cCfg *cfg = pdev->cfg;
	struct ArI2c *regs = pdev->cfg->regs;
	u8 temp;
	switch (state->slaveState) {
		case AR_I2C_S_IDLE:
			if (status == 0x70) { //general call
				 state->slaveState = AR_I2C_S_SETADDR;
			}
			else if (status == 0x60)    //slave address + w
			{
				state->slaveState = AR_I2C_S_BADDR;
			}
			else if (status == 0xa8)    //slave address + r
			{
				state->slaveState = AR_I2C_S_RCV_SADDR;
				//ctrller->slave_rw = 1;
				writel(pdev->addr, regs->DTR);
			}
			else if (status == 0xa0)    //stop or restart
			{
				state->slaveState = AR_I2C_S_IDLE;
			}
			else
			{
			}
			break;
		case AR_I2C_S_SETADDR:
			if (status == 0x90)         //receive data and ack send
			{
				temp = readl(cfg->regs->DTR);
				if(temp == 0x18){
					sm_log_debug("Set SA status\n");
				} else {
					sm_log_debug("Unrecognized command for General Call\n");
				}
				state->slaveState = AR_I2C_S_RCV_SADDR;
			}
			break;
		case AR_I2C_S_RCV_SADDR:
			if(status == 0x90){
				temp = readl (cfg->regs->DTR);
				writel((temp<<1)|0x01,cfg->regs->SAR);
			}
			state->slaveState = AR_I2C_S_IDLE;
			break;
		case AR_I2C_S_BADDR:
			if(status == 0x80){
				slave_reg_addr = readl(cfg->regs->DTR);
				state->slaveState = AR_I2C_S_WDATA;
			}else {
				sm_log_debug("AR_I2C_S_BADDR Error\n");
			}
			break;
		case AR_I2C_S_WDATA:	
			if(status == 0xa0){
				state->slaveState = AR_I2C_S_IDLE;
			}else if(status == 0x80){
				if (state->rx.offset < state->rx.size) {
					state->rx.buf[state->rx.offset] = readl(cfg->regs->DTR);
					state->rx.offset++;
				} else {
				ArI2cAckDisable(cfg);
				/* TODO: error on overflow */
				}
			}else {
				sm_log_debug("AR_I2C_S_WDATA Error\n");
			}
			break;
		case AR_I2C_S_RDATA:
			if(status == 0xa0){
				state->slaveState = AR_I2C_S_IDLE;
			}else if(status == 0xb8){
				if (state->tx.preamble) {
					writel(state->tx.byte,cfg->regs->DTR);
					state->tx.offset++;
				} else if (state->tx.offset < state->tx.size) {
					writel(state->tx.byte,cfg->regs->DTR);
					state->tx.offset++;
				} else {
					state->slaveState = AR_I2C_S_RDATA;
					ArI2cInvokeTxCallback(state, state->tx.offset, 0, 0);
				}
			}else if(status == 0xc0){
				state->tx.offset--;
				ArI2cSlaveTxDone(pdev);
				state->slaveState = AR_I2C_S_IDLE;
			}else {
				state->slaveState = AR_I2C_S_RDATA;
			}
			break;
		default:
			break;
	}
}



static s32 i2c0_int_handler(void *parg){
	int id = *(int *)parg;
	struct ArI2cDev *pdev = &mArI2cDevs[id];
	//struct I2cArState *state = &pdev->state;
	u8 status = 0;//fix me
	i2c_disable_irq(pdev->cfg);

	//osLog(LOG_ERROR,"i2c_int_handler\n");
	if(pdev->state.mode == AR_I2C_SLAVE){
		i2c_process_slave_status(pdev,status);
	}else if(pdev->state.mode == AR_I2C_MASTER) {
	//	osDefer(i2c_process_master_status,(void *)pdev,false);
		i2c_process_master_status((void *)pdev);
	}
//	if(AR_I2C_MASTER_IDLE != atomicReadByte(&state->masterState))
//		i2c_enable_irq(pdev->cfg);
	return 0;

}

static s32 i2c1_int_handler(void *parg){
	int id = *(int *)parg;
	struct ArI2cDev *pdev = &mArI2cDevs[id];
	//struct I2cArState *state = &pdev->state;
	u8 status = 0;//fix me
	i2c_disable_irq(pdev->cfg);

	//osLog(LOG_ERROR,"i2c_int_handler\n");
	if(pdev->state.mode == AR_I2C_SLAVE){
		i2c_process_slave_status(pdev,status);
	}else if(pdev->state.mode == AR_I2C_MASTER) {
//		osDefer(i2c_process_master_status,(void *)pdev,false);
		i2c_process_master_status((void *)pdev);
	}
//	if(AR_I2C_MASTER_IDLE != atomicReadByte(&state->masterState))
//		i2c_enable_irq(pdev->cfg);
	return 0;

}


int i2cMasterRequest(uint32_t busId, uint32_t speedInHz){

	if (busId >= ARRAY_SIZE(mArI2cDevs))
        return -EINVAL;

    const struct ArI2cBoardCfg *board = boardArI2cCfg(busId);
    if (!board)
        return -EINVAL;
	if(0 == busId){
		bus0_id = busId;
	}else if(1 == busId){
		bus1_id = busId;
	}
    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    struct I2cArState *state = &pdev->state;
    const struct ArI2cCfg *cfg = &mArI2cCfgs[busId];
	//uint32_t value;
    if (state->mode == AR_I2C_DISABLED) {
        state->mode = AR_I2C_MASTER;

        pdev->cfg = cfg;
        pdev->board = board;
        pdev->next = 2;
        pdev->last = 1;
        atomicBitsetInit(mXfersValid, I2C_MAX_QUEUE_DEPTH);
        pdev->scl = arI2cGpioInit(board, &board->gpioScl);
        pdev->sda = arI2cGpioInit(board, &board->gpioSda);

		ccu_set_mclk_onoff(CCU_MOD_CLK_R_TWI, CCU_CLK_ON);
		ccu_set_mclk_reset(CCU_MOD_CLK_R_TWI, CCU_CLK_NRESET);
		// gating
		if(busId == 0){
			writel(0x0,0xD7010010);
			writel(readl(0xD701019c)|0x20002,0xD701019c);
		}else if(busId == 1){
			writel(0x0,0xD7010010);
			writel(readl(0xD701019c)|0x40004,0xD701019c);
		}
		arI2cSpeedSet(cfg,speedInHz);
		writel(0x58, cfg->regs->CKR);

		arI2cReset(cfg);
	if ((readl(cfg->regs->LCR)&0xff) != 0x3a)
		LOG("init twi failed!,status : ,reg : 0x%x ,value :0x%x\n",cfg->regs->LCR,readl(cfg->regs->SAR));
		ArI2cAckEnable(cfg);
		arI2cEnable(cfg);
        atomicWriteByte(&state->masterState, AR_I2C_MASTER_IDLE);
		if(0 == busId){
	        install_isr(cfg->irqEv, i2c0_int_handler, &bus0_id);
		}else if(1 == busId){
			install_isr(cfg->irqEv, i2c1_int_handler, &bus1_id);
		}
		interrupt_enable(cfg->irqEv);
        return 0;
    } else {
        return -EBUSY;
    }
}
int i2cMasterRelease(uint32_t busId){
	 if (busId >= ARRAY_SIZE(mArI2cDevs))
        return -EINVAL;

    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    struct I2cArState *state = &pdev->state;
    const struct ArI2cCfg *cfg = pdev->cfg;

    if (state->mode == AR_I2C_MASTER) {
        if (atomicReadByte(&state->masterState) == AR_I2C_MASTER_IDLE) {
            state->mode = AR_I2C_DISABLED;
			i2c_disable_irq(cfg);
			interrupt_disable(cfg->irqEv);
			if(0 == busId){
				uninstall_isr(cfg->irqEv,i2c0_int_handler);
			}else if(1 == busId){
				uninstall_isr(cfg->irqEv,i2c1_int_handler);
			}
            arI2cDisable(cfg);
		    ccu_set_mclk_src(CCU_MOD_CLK_R_TWI, cfg->clock);
            gpioRelease(pdev->scl);
            gpioRelease(pdev->sda);
            return 0;
        } else {
            return -EBUSY;
        }
    } else {
        return -EINVAL;
    }
}

int i2cMasterTxRx(uint32_t busId, uint32_t addr, const void *txBuf, size_t txSize,
        void *rxBuf, size_t rxSize, I2cCallbackF callback, void *cookie){

	uint32_t id = 0;
	uint32_t id_r = 0;
	uint32_t retry_cnt = 0;
	struct ArI2cXfer *xfer;
	struct ArI2cXfer *xfer_r;
    if (busId >= ARRAY_SIZE(mArI2cDevs))
        return -EINVAL;
    else if (addr & 0x80)
        return -ENXIO;

    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    struct I2cArState *state = &pdev->state;
	const struct ArI2cCfg *cfg = pdev->cfg;
    if (state->mode != AR_I2C_MASTER)
        return -EINVAL;

	if (rxSize > 0) { //read :request  2 xfer
		xfer = ArI2cGetXfer();
		if (xfer) {     // write read reg addr
			xfer->busId = busId;
			xfer->addr = ((uint8_t)addr & 0xff)<< 1;
			xfer->txBuf = txBuf;
			xfer->txSize = txSize;
			xfer->rxBuf = NULL;
			xfer->rxSize = 0;
			xfer->callback = NULL;
			xfer->cookie = NULL;
			do {
				id = atomicAdd32bits(&pdev->last, 1);
			} while (!id);
			atomicWrite32bits(&xfer->id, id);
		}

		xfer_r = ArI2cGetXfer();
		if (xfer_r) {         // read
			xfer_r->busId = busId;
			xfer_r->addr = ((uint8_t)addr & 0xff)<< 1;
			xfer_r->addr |= 0x01;
			xfer_r->txBuf = NULL;
			xfer_r->txSize = 0;
			xfer_r->rxBuf = rxBuf;
			xfer_r->rxSize = rxSize;
			xfer_r->callback = callback;
			xfer_r->cookie = cookie;
			do {
				id_r = atomicAdd32bits(&pdev->last, 1);
			} while (!id_r);
			atomicWrite32bits(&xfer_r->id, id_r);
		}
	}else {
		xfer = ArI2cGetXfer();
		if (xfer) {
			xfer->busId = busId;
			xfer->addr = ((uint8_t)addr & 0xff)<< 1;
			xfer->txBuf = txBuf;
			xfer->txSize = txSize;
			xfer->rxBuf = rxBuf;
			xfer->rxSize = rxSize;
			xfer->callback = callback;
			xfer->cookie = cookie;
			do {
				id = atomicAdd32bits(&pdev->last, 1);
			} while (!id);
			atomicWrite32bits(&xfer->id, id);
		}
	}
        // only initiate transfer here if we are in IDLE. Otherwise the transfer
        // completion interrupt will start the next transfer (not necessarily
        // this one)
   if(xfer) {
        if (atomicCmpXchgByte((uint8_t *)&state->masterState,
                AR_I2C_MASTER_IDLE, AR_I2C_MASTER_START)) {
  //              osLog(LOG_ERROR,"%u\n",__LINE__);
            // it is possible for this transfer to already be complete by the
            // time we get here. if so, transfer->id will have been set to 0.
            if (atomicCmpXchg32bits(&xfer->id, id, 0)) {
	//			osLog(LOG_ERROR,"%u\n",__LINE__);
                pdev->addr = xfer->addr;
                state->tx.cbuf = xfer->txBuf;
                state->tx.offset = 0;
                state->tx.size = xfer->txSize;
                state->tx.callback = xfer->callback;
                state->tx.cookie = xfer->cookie;
                state->rx.buf = xfer->rxBuf;
                state->rx.offset = 0;
                state->rx.size = xfer->rxSize;
                state->rx.callback = NULL;
                state->rx.cookie = NULL;
                state->tid = osGetCurrentTid();
                if (pdev->board->sleepDev >= 0)
                    platRequestDevInSleepMode(pdev->board->sleepDev, 12);
                arI2cPutXfer(xfer);

		RETRY:
				udelay(100);
				if(++retry_cnt > 10){
					atomicWriteByte(&state->masterState, AR_I2C_MASTER_IDLE);
					osLog(LOG_ERROR,"i2c start fail after retry %ld times\n",retry_cnt);
					return -EBUSY;
				}
				arI2cReset(cfg);
				udelay(100);
				if(0xf8 != readl(cfg->regs->STR)&&
					0x00 != readl(cfg->regs->STR)&& 0xB0 != readl(cfg->regs->STR)){
					if (I2C_OK == twi_send_clk_9pulse(cfg, busId))
						INF("LCR is ok\n");
					else
						goto RETRY;
				}
				i2c_enable_irq(cfg);
				ArI2cAckDisable(cfg);
				ArI2cSetEfr(cfg,0x0);
				int ret = arI2cStart(cfg);
				if (ret == I2C_FAIL) {
						osLog(LOG_ERROR,"i2c start fail at retry cnt %ld\n",retry_cnt);
						goto RETRY;
				}
				atomicWriteByte(&state->masterState, AR_I2C_MASTER_RUNNING);
            }
        }
        return 0;
	} else return -1;
}

int i2cSlaveRequest(uint32_t busId, uint32_t addr){
	if (busId >= ARRAY_SIZE(mArI2cDevs))
		return -EINVAL;

    const struct ArI2cBoardCfg *board = boardArI2cCfg(busId);
    if (!board)
        return -EINVAL;

    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    const struct ArI2cCfg *cfg = &mArI2cCfgs[busId];

    if (pdev->state.mode == AR_I2C_DISABLED) {
        pdev->state.mode = AR_I2C_SLAVE;

        pdev->addr = addr;
        pdev->cfg = cfg;
        pdev->board = board;

        pdev->scl = arI2cGpioInit(board, &board->gpioScl);
        pdev->sda = arI2cGpioInit(board, &board->gpioSda);

        return 0;
    } else {
        return -EBUSY;
    }
}
int i2cSlaveRelease(uint32_t busId){
	if (busId >= ARRAY_SIZE(mArI2cDevs))
		return -EINVAL;

    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    const struct ArI2cCfg *cfg = pdev->cfg;

    if (pdev->state.mode == AR_I2C_SLAVE) {
        pdev->state.mode = AR_I2C_DISABLED;
        arI2cStop(cfg);
        ccu_set_mclk_src(CCU_MOD_CLK_R_TWI, cfg->clock);

        gpioRelease(pdev->scl);
        gpioRelease(pdev->sda);

        return 0;
    } else {
        return -EBUSY;
    }
}

void i2cSlaveEnableRx(uint32_t busId, void *rxBuf, size_t rxSize,
        I2cCallbackF callback, void *cookie){
	struct ArI2cDev *pdev = &mArI2cDevs[busId];
	const struct ArI2cCfg *cfg = pdev->cfg;
	struct I2cArState *state = &pdev->state;

    if (pdev->state.mode == AR_I2C_SLAVE) {
        state->rx.buf = rxBuf;
        state->rx.offset = 0;
        state->rx.size = rxSize;
        state->rx.callback = callback;
        state->rx.cookie = cookie;
        //state->slaveState = STM_I2C_SLAVE_RX_ARMED;
        state->tid = osGetCurrentTid();

        ccu_set_mclk_src(CCU_MOD_CLK_R_TWI, cfg->clock);
        ccu_reset_module(CCU_MOD_CLK_R_TWI);

        arI2cStart(cfg);
        cfg->regs->SAR = I2C_SADDR_ADD7(pdev->addr);
        ArI2cAckEnable(cfg);
    }
}
static int i2cSlaveTx(uint32_t busId, const void *txBuf, uint8_t byte,
        size_t txSize, I2cCallbackF callback, void *cookie)
{
    struct ArI2cDev *pdev = &mArI2cDevs[busId];
    struct I2cArState *state = &pdev->state;

    if (pdev->state.mode == AR_I2C_SLAVE) {
        if (state->slaveState == AR_I2C_S_RCV_SADDR)
            return -EBUSY;

        if (txBuf) {
            state->tx.cbuf = txBuf;
            state->tx.preamble = false;
        } else {
            state->tx.byte = byte;
            state->tx.preamble = true;
        }
        state->tx.offset = 0;
        state->tx.size = txSize;
        state->tx.callback = callback;
        state->tx.cookie = cookie;


        state->slaveState = AR_I2C_S_WDATA;

        return 0;
    } else {
        return -EBUSY;
    }
}

int i2cSlaveTxPreamble(uint32_t busId, uint8_t byte,
        I2cCallbackF callback, void *cookie){
	return i2cSlaveTx(busId, NULL, byte, 0, callback, cookie);
}
int i2cSlaveTxPacket(uint32_t busId, const void *txBuf, size_t txSize,
        I2cCallbackF callback, void *cookie){
	return i2cSlaveTx(busId, txBuf, 0, txSize, callback, cookie);
}


