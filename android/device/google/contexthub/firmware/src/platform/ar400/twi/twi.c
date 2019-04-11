/*
 * drivers/twi/twi.c
 *
 * Copyright (C) 2014-2016 AllWinnertech Ltd.
 * Author: Xiafeng <Xiafeng@allwinnertech.com>
 *
 */

#include "twi_i.h"

#if TWI_USED

//#define TWI_DEBUG

extern struct arisc_para arisc_para;
volatile bool twi_lock = FALSE;

#ifdef TWI_DEBUG
static void printreg(void)
{
	int i;

	for (i = 0; i < 9; i++)
		LOG("reg[%d]:%x\n", i, readl(TWI_SAR + ( i<< 2)));
}
#endif

/* clear the interrupt flag */
static inline void twi_clear_irq_flag(void)
{
	unsigned int reg_val;

	reg_val = readl(TWI_CTR);
	reg_val |= TWI_INTFLAG;
	//reg_val &= ~(TWI_MSTART | TWI_MSTOP); /* start and stop bit should be 0 */
	writel(reg_val, TWI_CTR);

	/* make sure that interrupt flag does really be cleared */
	readl(TWI_CTR); /* read twice for delay */
	while (readl(TWI_CTR) & TWI_INTFLAG)
		;
}

static inline void twi_enable_ack(void)
{
	unsigned int reg_val;

	reg_val = readl(TWI_CTR);
	reg_val |= TWI_AACK;
	reg_val &= ~TWI_INTFLAG;
	writel(reg_val, TWI_CTR);
	readl(TWI_CTR);
	readl(TWI_CTR);
}

static inline void twi_disable_ack(void)
{
	unsigned int reg_val;

	reg_val = readl(TWI_CTR) & 0x0ff;
	reg_val &= ~TWI_AACK;
	reg_val &= ~TWI_INTFLAG;
	writel(reg_val, TWI_CTR);
}

static void twic_send_dummy_clock(void)
{
	unsigned int i = 10, timeout;

	INF("restore bus status to 0x3a\n");
	while (i--) {
		writel(0xa, TWI_LCR);
		for (timeout = 1000; timeout; timeout--)
			;
	}
	writel(0xa, TWI_LCR);
	if (readl(TWI_LCR) != 0x3a)
		INF("bus status err: 0x%x\n", readl(TWI_LCR));

	writel(1, TWI_SRR);
}

static void twic_reset(void)
{
	/* reset twi comtroller */
	writel(1, TWI_SRR);
	INF("reset\n");
	/* wait twi reset completing */
	while (readl(TWI_SRR))
		;
}

static void twi_start(void)
{
	unsigned int value;

	value = readl(TWI_CTR);
	value |= TWI_MSTART;
	value &= ~TWI_INTFLAG;
	writel(value, TWI_CTR);
}

/*
 * twi_stop() - stop current twi transfer.
 *
 */
static u32 twi_stop(void)
{
	unsigned int timeout;
	unsigned int value;

	/* step1. send stop signal */
	value = readl(TWI_CTR);
	value |= TWI_MSTOP;
	value &= ~TWI_INTFLAG;
	writel(value, TWI_CTR);
	timeout = TWI_CHECK_TIMEOUT;
	while ((readl(TWI_CTR) & TWI_MSTOP) && (--timeout))
		;

	/* step2. clear the interrupt flag */
	twi_clear_irq_flag();

	value = readl(TWI_CTR); /* read delay */
	value = readl(TWI_CTR); /* read delay */
	timeout = TWI_CHECK_TIMEOUT;
	while ((readl(TWI_CTR) & TWI_MSTOP) && (--timeout))
		;

	/* step3. check twi fsm is idle(0xf8) */
	timeout = TWI_CHECK_TIMEOUT;
	while ((0xf8 != readl(TWI_STR)) && (--timeout))
		;
	if (timeout == 0) {
		ERR("state err:%x\n", readl(TWI_STR));
		twic_reset();
		return -EFAIL;
	}

	/* step4. check twi scl & sda must high level */
	timeout = TWI_CHECK_TIMEOUT;
	while ((0x3a != readl(TWI_LCR)) && (--timeout))
		;
	if (timeout == 0) {
		ERR("lcr err:%x\n", readl(TWI_LCR));
		return -EFAIL;
	}
	INF("stop send\n");

	return OK;
}

/*
 * twic_set_sclk() - set twi clock to clk.
 *
 * @clk: the clock want to set.
 */
static void twic_set_sclk(u32 clk)
{
	u8 clk_m = 0;
	u8 clk_n = 0;
#ifdef FPGA_PLATFORM
	/* 24MHz clock source: 400KHz, clk_m = 5, clk_n = 0, 400KHz */
	clk_m = 5;
	clk_n = 0;
	goto out;
#else
	u32 sclk_real = 0;
	u32 src_clk = 0;
	u8  pow_clk_n = 1;
	u32 divider = 0;

	src_clk = ccu_get_sclk_freq(CCU_SYS_CLK_APB0) / 10;
	INF("apb0 clk:%x\n", src_clk);

	divider = src_clk / clk;
	if (divider == 0) {
		clk_m = 1;
		goto out;
	}

	while (clk_n < 8) {
		clk_m = (divider / pow_clk_n) - 1;
		while (clk_m < 16) {
			sclk_real = src_clk / (clk_m + 1) / pow_clk_n;
			if (sclk_real <= clk)
				goto out;
			else
				clk_m++;
		}
		clk_n++;
		pow_clk_n *= 2;
	}
#endif

out:
	/* set clock control register */
	writel((clk_m << 3) | clk_n, TWI_CKR);
	//INF("m = %x, n = %x, req_sclk = %d, real_clk = %d\n", clk_m, clk_n, clk, sclk_real);
}

static s32 twi_clkchangecb(u32 command, u32 freq)
{
	switch(command) {
		case CCU_CLK_CLKCHG_REQ:
		{
			/* check twi is busy now
			 * ...
			 * clock will be change, lock twi interface
			 */
			twi_lock = 1;
			INF("twi clk change request\n");
			return OK;
		}
		case CCU_CLK_CLKCHG_DONE:
		{
			/*
			 * clock change finish, re-config twi clock,
			 * maybe re-config rsb clock should do somethings?
			 */
			twic_set_sclk(TWI_CLOCK_FREQ);

			/* unlock twi interface */
			twi_lock = 0;
			INF("twi clk change done\n");
			return OK;
		}
		default:
		{
			break;
		}
	}

	return -EFAIL;
}

s32 twi_init(void)
{
#ifdef  FPGA_PLATFORM
	/* PG[24][25] used as twi SCK and SDA under FPGA platform */
	//writel(0x22, 0x01c20800 + 0xE4);
	writel(0x00000033, 0x01c20824);//cfg
	writel(0x00000005, 0x01c20840);//pull
	writel(0x0000000a, 0x01c20838);//drv
#endif

	/* initialize twi sck and sda pins */
#if PIN_INIT_BY_CPUS
	//judge pmu whether exist
	if (arisc_para.power_mode != POWER_MODE_AXP) {
		twi_lock = TRUE;
		return OK;
	}
	pin_set_multi_sel(PIN_GRP_PL, 0, 2); /* PL0 config as TWI_SCK */
	pin_set_pull(PIN_GRP_PL, 0, PIN_PULL_DISABLE); /* TWI_SCK pull-up */
	pin_set_drive(PIN_GRP_PL, 0, PIN_MULTI_DRIVE_2); /* TWI_SCK drive level 2 */
	pin_set_multi_sel(PIN_GRP_PL, 1, 2); /* PL1 config as TWI_SDA */
	pin_set_pull(PIN_GRP_PL, 1, PIN_PULL_DISABLE); /* TWI_SDA pull-up */
	pin_set_drive(PIN_GRP_PL, 1, PIN_MULTI_DRIVE_2); /* TWI_SDA drive level 2 */
#endif
	/* initialize twi clock */
	//ccu_set_mclk_reset(CCU_MOD_CLK_R_TWI_1, CCU_CLK_RESET);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_TWI, CCU_CLK_ON);
	ccu_set_mclk_reset(CCU_MOD_CLK_R_TWI, CCU_CLK_NRESET);
	twic_set_sclk(TWI_CLOCK_FREQ);
	writel(TWI_BUS_ENB | TWI_AACK, TWI_CTR);
	twic_reset();

	/* twi bus state is not reset value, try to restore it */
	if (readl(TWI_LCR) != 0x3a)
		twic_send_dummy_clock();

	/* register uart clock change notifier */
	ccu_reg_mclk_cb(CCU_MOD_CLK_R_TWI, twi_clkchangecb);

	twi_lock = 0;

#ifdef TWI_DEBUG
	printreg();
#endif
	if (readl(TWI_LCR) == 0x3a)
		LOG("init twi succeeded\n");
	else
		LOG("init twi failed!\n");

	return OK;
}

/*
 * twi_exit() - exit twi transfer.
 *
 * @return: 0 always
 */
s32 twi_exit(void)
{
	/* softreset twi module  */

	return OK;
}

/*
 * twi_byte_rw() - twi byte read and write.
 *
 * @op: operation read or write
 * @saddr: slave address
 * @baddr: byte address
 * @data: pointer to the data to be read or write
 * @return: EPDK_OK,      byte read or write successed;
 *          EPDK_FAIL,    btye read or write failed!
 */
static s32 twi_byte_rw(twi_rw_type_e op, u8 saddr, u8 baddr, u8 *data, u32 len)
{
	volatile unsigned int state;
	unsigned int timeout;
	int ret = -1;
	s32 cpsr;

	if(twi_lock)
		return -EACCES;

	if(len > 4)
		return -EINVAL;

	twi_enable_ack(); /* enable ACK first */

	cpsr = cpuIntsOff();
	writel(0, TWI_EFR);
	state = (readl(TWI_STR) & 0x0ff);
	if (state != 0xf8) {
		ERR("e1:%x\n", state);
		goto stop_out;
	}

	/*
	 * control registser bitmap
	 *   7      6       5     4       3       2    1    0
	 * INT_EN  BUS_EN  START  STOP  INT_FLAG  ACK  NOT  NOT
	 */

	/* step1. Send Start */
	twi_start();

	timeout = TWI_CHECK_TIMEOUT;
	while ((!(readl(TWI_CTR) & TWI_INTFLAG)) && (--timeout)) /* wait for flg set */
		;
	if (timeout == 0) {
		ERR("to0\n");
		goto stop_out;
	}

	state = readl(TWI_STR);
	if (state != 0x08) {
		ERR("e3:%x\n", state);
		goto stop_out;
	}

	/* step2. Send Slave Address */
	writel(((saddr << 1) & 0x0fe), TWI_DTR); /* slave address + write */
	twi_clear_irq_flag(); /* clear int flag to send saddr */

	timeout = TWI_CHECK_TIMEOUT;
	while ((!(readl(TWI_CTR) & TWI_INTFLAG)) && (--timeout))
		;
	if (timeout == 0) {
		ERR("to1\n");
		goto stop_out;
	}

	state = readl(TWI_STR);
	while (state != 0x18) {
		ERR("e5:%x\n", state);
		goto stop_out;
	}

	/* step3. Send Byte Address */
	writel(baddr, TWI_DTR); /* slave address + write */
	twi_clear_irq_flag(); /* clear int flag to send regaddr */

	timeout = TWI_CHECK_TIMEOUT;
	while ((!(readl(TWI_CTR) & TWI_INTFLAG)) && (--timeout))
		;
	if (timeout == 0) {
		ERR("to2\n");
		goto stop_out;
	}

	state = readl(TWI_STR);
	if (state != 0x28) {
		ERR("e7:%x\n", state);
		goto stop_out;
	}

	if (op == TWI_WRITE) {
		/* step4. Send Data to be write */
		while (len--) {
			writel(*data++, TWI_DTR); /* slave address + write */
			twi_clear_irq_flag(); /* clear int flag */

			timeout = TWI_CHECK_TIMEOUT;
			while((!(readl(TWI_CTR) & TWI_INTFLAG)) && (timeout--));
			if (timeout == -1) {
				ERR("to3\n");
				goto stop_out;
			}

			state = readl(TWI_STR);
			if (state != 0x28) {
				ERR("e9:%x\n", state);
				goto stop_out;
			}
		}
	} else {
		/* step4. Send restart for read */
		twi_start();
		twi_clear_irq_flag(); /* clear int flag to send saddr */

		timeout = TWI_CHECK_TIMEOUT;
		while ((!(readl(TWI_CTR) & TWI_INTFLAG)) && (--timeout))
			;
		if (timeout == 0) {
			ERR("to4\n");
			goto stop_out;
		}

		state = readl(TWI_STR);
		if (state != 0x10) {
			ERR("e11:%x\n", state);
			goto stop_out;
		}

		/* step5. Send Slave Address */
		writel(((saddr << 1) | 1), TWI_DTR); /* slave address + write */
		twi_clear_irq_flag(); /* clear int flag to send saddr */

		timeout = TWI_CHECK_TIMEOUT;
		while ((!(readl(TWI_CTR) & TWI_INTFLAG)) && (--timeout))
			;
		if (timeout == 0) {
			ERR("to5\n");
			goto stop_out;
		}

		state = readl(TWI_STR);
		if (state != 0x40) {
			ERR("e13:%x\n", state);
			goto stop_out;
		}

		/* step6. Get data */
		while (len--) {
			if(len == 0)
				twi_disable_ack();

			twi_clear_irq_flag(); /* clear int flag then data come in */

			timeout = TWI_CHECK_TIMEOUT;
			while((!(readl(TWI_CTR) & TWI_INTFLAG)) && (timeout--));
			if (timeout == -1) {
				ERR("to6\n");
				goto stop_out;
			}

			*data++ = readl(TWI_DTR);
			state = readl(TWI_STR);
			if (len > 0) {
				if (state != 0x50) {
					ERR("e14:%x\n", state);
					goto stop_out;
				}
			} else {
				if (state != 0x58) {
					ERR("e15:%x\n", state);
					goto stop_out;
				}
			}
		}
	}
	ret = 0;

stop_out:
	/* WRITE: step 5; READ: step 7 */
	twi_stop(); /* Send Stop */
#ifdef TWI_DEBUG
	//INF("twi_%x r:%x d:%x\n", op, baddr, *data);
	//printreg(); /* only for twi time sequence debuger */
#endif

	cpuIntsRestore(cpsr);
	//printk("%u,%u,%u,%u,%d\n", op, saddr, baddr, *data, ret) ;

	return ret;
}

s32 twi_read(u8 *addr, u8 *data, u32 len)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < len; i++) {
		ret |= twi_byte_rw(TWI_READ, PMU_IIC_ADDR, *addr++, data++, 1);
	}

	return ret;
}

s32 twi_write(u8 *addr, u8 *data, u32 len)
{
	u32 i;
	s32 ret = 0;

	for (i = 0; i < len; i++) {
		ret |= twi_byte_rw(TWI_WRITE, PMU_IIC_ADDR, *addr++, data++, 1);
	}

	return ret;
}


s32 twi_read_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u8  addr[TWI_TRANS_BYTE_MAX];
	u8  data[TWI_TRANS_BYTE_MAX];
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

	result = twi_read(addr, data, len);

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

s32 twi_write_block_data(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u8  addr[TWI_TRANS_BYTE_MAX];
	u8  data[TWI_TRANS_BYTE_MAX];
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


	result = twi_write(addr, data, len);

	return result;
}

s32 twi_bits_ops_sync(struct message *pmessage)
{
	u32 i = 0;
	u32 len = 0;
	u32 ops = 0;
	u8  addr[TWI_TRANS_BYTE_MAX];
	u8  mask[TWI_TRANS_BYTE_MAX];
	u8  delay[TWI_TRANS_BYTE_MAX];
	u8  data[TWI_TRANS_BYTE_MAX];
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
		twi_read(&addr[i], &data[i], 1);
		if (ops == TWI_CLR_BITS)
		{
			//clear target bits
			data[i] = data[i] & (~mask[i]);
		}
		else if (ops == TWI_CLR_BITS)
		{
			//set target bits
			data[i] = data[i] | mask[i];
		}

		//writeback register value
		result = twi_write(addr, data, len);
		if (delay[i])
		{
			//delay some time
			time_udelay(delay[i] * 1000);
		}
	}

	return result;
}

bool is_twi_lock(void)
{
	return twi_lock;
}
#endif /* TWI_USED */
