/*
 * drivers/uart/uart.c
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */
#include "uart_i.h"

volatile u32 uart_pin_not_used = 0;
volatile u32 uart_lock = 1;
volatile static u32 uart_rate;

#define IS_TX_FIFO_EMPTY  (readl(UART_REG_USR) & (0x1 << 2))
static int uart_clkchangecb(u32 command, u32 freq)
{
	if (uart_pin_not_used)
		return -EACCES;

	switch(command) {
		case CCU_CLK_CLKCHG_REQ:
		{
			//clock will be change
			INF("uart source clock change request\n");
			//wait uart transmit fifo empty
			while (!IS_TX_FIFO_EMPTY)
				continue;
			//lock uart
			uart_lock = 1;
			return OK;
		}
		case CCU_CLK_CLKCHG_DONE:
		{
			//reconfig uart current baudrate
			uart_set_baudrate(uart_rate);
			uart_lock = 0;
			INF("uart buadrate change done\n");

			return OK;
		}
	}

	return -ESRCH;
}

/*
 * uart_init() - initialize uart.
 *
 * @return: OK if initialize uart succeeded, others if failed.
 * @note: initialize uart.
 */
int uart_init(void)
{
	u32 div;
	u32 lcr;
#ifndef FPGA_PLATFORM
	u32 apbs2_clk;
#endif

	//uart_pin_not_used = !arisc_para_get_uart_pin_info();
	uart_pin_not_used = 0;
	if (uart_pin_not_used)
		return -EACCES;

	uart_rate = UART_BAUDRATE;
#ifndef FPGA_PLATFORM
#if PIN_INIT_BY_CPUS
	pin_set_multi_sel(PIN_GRP_PL, 2, 2); //PL2, select as uart_tx
	pin_set_multi_sel(PIN_GRP_PL, 3, 2); //PL3, select as uart_rx
#endif
	//set reset as assert state.
	ccu_set_mclk_reset(CCU_MOD_CLK_R_UART, CCU_CLK_NRESET),

	//set uart clock, open apb0 uart gating.
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_UART, CCU_CLK_ON);

	//the baud rate divisor
	apbs2_clk = ccu_get_sclk_freq(CCU_SYS_CLK_APBS2);
	div = (apbs2_clk + (uart_rate<<3)) / (uart_rate<<4);
#else

	pin_set_multi_sel(PIN_GRP_PL, 2, 2); //PL2, select as uart_tx
	pin_set_multi_sel(PIN_GRP_PL, 3, 2); //PL3, select as uart_rx

	//set reset as assert state.
	ccu_set_mclk_reset(CCU_MOD_CLK_R_UART, CCU_CLK_NRESET),

	//set uart clock, open apb0 uart gating.
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_UART, CCU_CLK_ON);

	//writel(0x1 << 16, R_PRCM_REG_BASE + 0x18c);
	//writel((0x1 << 16) | 0x1, R_PRCM_REG_BASE + 0x18c);
	//the baud rate divisor, baud_rate=24M/(16*13)=115384.6
	//the baud rate divisor, baud_rate=24M/(16*26)=57692.3
	if (uart_rate == 57600) /* 57692.3 -> 57600 */
		div = 0x1A;
	else                    /* 115384.6 -> 115200 */
		div = 0xd;
#endif

	//initialize uart controller
	lcr = readl(UART_REG_LCR);
	//writel(1, UART_REG_HALT);
	writel(lcr | 0x80, UART_REG_LCR);  //select the Divsor Latch Low Register and Divsor Latch High Register
	writel(div & 0xff, UART_REG_DLL);
	writel((div >> 8) & 0xff, UART_REG_DLH);
#if SHELL_USED
	writel(lcr & (~0x80), UART_REG_LCR);
#endif
	writel(0, UART_REG_HALT);
	writel(3, UART_REG_LCR);           //set mode, 8bit charset
	writel(7, UART_REG_FCR);           //enable and reset transmit/receive fifo
#if SHELL_USED
	writel(1, UART_REG_IER);       //enable receiver interrupt
#endif
	//register uart clock change notifier
	ccu_reg_mclk_cb(CCU_MOD_CLK_R_UART, uart_clkchangecb);

	//ensure uart is unlock
	uart_lock = 0;

	//uart initialize succeeded
	return OK;
}

/*
 * uart_putc() - exit uart.
 *
 * @return: OK if exit uart succeeded, others if failed.
 * @note: exit uart.
 */
int uart_exit(void)
{
	uart_lock = 1;
	//uart_pin_not_used = !arisc_para_get_uart_pin_info();
	uart_pin_not_used = 0;

	ccu_unreg_mclk_cb(CCU_MOD_CLK_R_UART, uart_clkchangecb);

	pin_set_multi_sel(PIN_GRP_PL, 2, 7); //PL2, select as uart_tx
	pin_set_multi_sel(PIN_GRP_PL, 3, 7); //PL3, select as uart_rx

	//set uart clock, open apb0 uart gating.
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_UART, CCU_CLK_OFF);

	//set reset as assert state.
	ccu_set_mclk_reset(CCU_MOD_CLK_R_UART, CCU_CLK_RESET);

	return OK;
}

/*
 * uart_putc() - put out a charset.
 *
 * @ch: the charset which we want to put out.
 * @return: OK if put out charset succeeded, others if failed.
 * @note: put out a charset.
 */
int uart_putc(char ch)
{
	if (uart_lock || uart_pin_not_used)
		return -EACCES;

	while (!(readl(UART_REG_USR) & 2)) /* fifo is full, check again */
		;

	/* write out charset to transmit fifo */
	writel(ch, UART_REG_THR);

	return OK;
}

/*
 * uart_getc() - get a charset.
 *
 * @return: the charset we read from uart.
 * @note: get a charset.
 */
u32 uart_get(char *buf)
{
	u32 count = 0;

	if (uart_lock || uart_pin_not_used)
		return -EACCES;

	while(readl(UART_REG_RFL)) {
		*buf++ = (char)(readl(UART_REG_RBR) & 0xff);
		count++;
	}

	return count;
}

/*
 * uart_puts() - put out a string.
 *
 * @string: the string which we want to put out.
 * @return: OK if put out string succeeded, others if failed.
 * @note: put out a string.
 */
int uart_puts(const char *string)
{
	if (uart_lock || uart_pin_not_used)
		return -EACCES;

	ASSERT(string != NULL);

	while (*string != '\0') {
		if (*string == '\n') /* if current character is '\n', insert output with '\r' */
			uart_putc('\r');
		uart_putc(*string++);
	}

	return OK;
}

int uart_set_baudrate(u32 rate)
{
	u32 freq;
	u32 div;
	u32 lcr;
	u32 halt;

	if (uart_pin_not_used)
		return -EACCES;

	//update current uart baudrate
	LOG("uart buadrate change from [%d] to [%d]\n", uart_rate, rate);
	uart_rate = rate;

	//wait uart transmit fifo empty
	while (readl(UART_REG_TFL))
		continue;

	//reconfig uart baudrate
	freq = ccu_get_sclk_freq(CCU_SYS_CLK_APBS2);
	div  = (freq + (uart_rate<<3)) / (uart_rate<<4);
	lcr  = readl(UART_REG_LCR);

	if (div == 0) /* avoid config invalid value */
		div = 1;

	//enable change when busy
	halt = readl(UART_REG_HALT) | 0x2;
	writel(halt, UART_REG_HALT);

	//select the divsor latch low register and divsor latch high register
	writel(lcr | 0x80, UART_REG_LCR);

	//set divsor of uart
	writel(div & 0xff, UART_REG_DLL);
	writel((div >> 8) & 0xff, UART_REG_DLH);

	//unselect the divsor latch low register and divsor latch high register
	writel(lcr & (~0x80), UART_REG_LCR);

	//update baudrate
	halt = readl(UART_REG_HALT) | 0x4;
	writel(halt, UART_REG_HALT);

	//waiting update
	while (readl(UART_REG_HALT) & 0x4)
		continue;

	//disable change when busy
	halt = readl(UART_REG_HALT) | 0x4;
	writel(halt & (~0x2), UART_REG_HALT);

	return OK;
}

u32 uart_get_baudrate(void)
{
	return uart_rate;
}
