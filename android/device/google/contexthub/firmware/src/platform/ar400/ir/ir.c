/*
 * drivers/ir/ir.c
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */
#include "ir_i.h"

#if IR_USED
//#define IR_TEST

//ir configure information backup
static u32 ir_ctrl;
static u32 ir_rxcfg;
static u32 ir_rxinte;
static u32 ir_rxints;
static u32 ir_cfg;
static u32 ir_clk_src;
static u32 ir_clk_div;

static ir_key_t ir_key_depot = {0};

static u32 ir_valid_value = 0;
static u32 ir_valid_code = 0;
static u32 ir_valid_addr = 0;

//ir rx raw data buffer
static ir_raw_buffer_t raw_buffer;

static s32 ir_clk_cfg(void)
{
	u32 value;

	/* enable ir mode */
	value = 0x3 << 4;
	writel(value, IR_CTRL_REG);

	/* config ir configure register */
	value = 0;
#ifdef FPGA_PLATFORM
	value |= 3<<0;
	value |= ((16 & 0x3f) << 2);
	value |= ((5  & 0xff) << 8);
	value |= ((1  & 0xff) << 23);
#else
#if IR_24M_USED
	value |= IR_SAMPLE_128;
	value |= IR_RXFILT_VAL;
	value |= IR_RXIDLE_VAL;
	value |= IR_ACTIVE_T;
	value |= IR_ACTIVE_T_C;
#else
	value  = ((0x1 << 24) | (0x0 << 0));    //sample_freq = 32768Hz / 1 = 32768Hz (30.5us)
	value |= ((IR_RXFILT_VAL & 0x3f) << 2); //set filter threshold
	value |= ((IR_RXIDLE_VAL & 0xff) << 8); //set idle threshold
	value |= (ATHC_UNIT << 23);
	value |= (ATHC_THRE & 0x7f)<<16;
#endif
#endif
	writel(value, IR_CFG_REG);

	//invert input signal
	writel((0x1<<2), IR_RXCFG_REG);

	//clear all rx interrupt status
	writel(0xff, IR_RXINTS_REG);

	//set rx interrupt enable
	value = 0;
	value |=  (IR_RXINTS_RXDA);  /* enable fifo available interrupt */
	value |=  (IR_RXINTS_RXPE);  /* enable package end interrupt */
	value |=  (IR_RXINTS_RXOF);  /* enable fifo full interrupt */
	value |= ((IR_FIFO_TRIGER - 1) << 8);  /* rx fifo threshold = fifo-size / 2 */
	writel(value, IR_RXINTE_REG);

	//enable ir module
	value = readl(IR_CTRL_REG);
	value |= 0x3;
	writel(value, IR_CTRL_REG);

	return OK;
}

static s32 ir_read_fifo_raw_data(struct ir_raw_buffer *raw_buf)
{
	u32  i;
	u32  count;
	u8   data;
	u32  offset;

	/* read fifo byte count */
	count = (readl(IR_RXINTS_REG) >> 8) & 0x7f;

	/* read fifo */
	offset = raw_buf->count;
	if ((offset + count) >= IR_RAW_BUFFER_SIZE) {
		ERR("ir data full\n");
		return -ENOSPC;
	}
	for (i = 0; i < count; i++) {
		/* read fifo data */
		data = (u8)(readl(IR_RXDAT_REG) & 0xff);
		raw_buf->data[offset + i] = data;
	}
	raw_buf->count += i;

	return OK;
}

static u32 ir_decode_raw_data(u8 *buffer, u32 count)
{
	u32 len;
	u32 val = 0, last = 1;
	u32 code = 0;
	u32 bitCnt = 0;
	u32 i = 0;

	/* find lead '1' */
	len = 0;
	for (i = 0; i < count; i++) {
		val = buffer[i];
		if (val & 0x80)
			len += (val & 0x7f);
		else {
			if (len > IR_L1_MIN)
				break;
			len = 0;
		}
	}
	if ((val & 0x80) || (len <= IR_L1_MIN)) {
		INF("lead1\n");
		goto error_code; /* Invalid Code */
	}

	/* find lead '0' */
	len = 0;
	for (; i < count; i++) {
		val = buffer[i];
		if (val & 0x80) {
			if (len > IR_L0_MIN)
				break;
			len = 0;
		} else
			len += (val & 0x7f);
	}

	if ((!(val & 0x80)) || (len <= IR_L0_MIN)) {
		INF("lead0\n");
		goto error_code; /* Invalid Code */
	}

	/* go decoding */
	code = 0;  /* 0 for repeat code */
	bitCnt = 0;
	last = 1;
	len = 0;
	for (; i < count; i++) {
		val = buffer[i];
		if (last) {
			if (val & 0x80)
				len += (val & 0x7f);
			else {
				if (len > IR_PMAX) {  /* error pulse */
					INF("Pulse %d\n", i);
					goto error_code; /* Invalid Code */
				}
				last = 0;
				len = val & 0x7f;
			}
		} else {
			if (val & 0x80) {
				if (len > IR_DMAX) { /* error distant */
					INF("Distant %d\n", i);
					goto error_code; /* Invalid Code */
				} else {
					if (len > IR_DMID) /* data '1' */
						code |= 1<<bitCnt;
					bitCnt ++;
					if (bitCnt == 32)
						break; /* decode over */
				}
				last = 1;
				len = val & 0x7f;
			} else
				len += (val & 0x7f);
		}
	}
	return code;

error_code:
	for (i = 0; i < count; i++)
		INF("%2x ", buffer[i]);
	INF("\n");
	return IR_ERROR_CODE;
}

static u32 ir_code_is_valid(u32 add_code, u32 code)
{
	u32 tmp1, tmp2;

#ifdef IR_CHECK_ADDR_CODE
	/* check address value */
	ir_valid_addr = code & 0xffff;
	if(ir_valid_addr != (add_code & 0xffff))
		return FALSE; /* address error */
#endif
	tmp1 = code & 0x00ff0000;
	tmp2 = (code & 0xff000000)>>8;

	if ((tmp1^tmp2) == 0x00ff0000) {
		ir_valid_code = tmp1 >> 16;
		INF("detect valid ir code :%x\n", ir_valid_code);
		return TRUE;
	} else {
		ir_valid_code = 0;
		return FALSE;
	}
}

#if (defined BOOT_USED) && (defined EVB_PLATFORM)
static int ir_int_handler(void *parg)
{
	interrupt_clear_pending(INTC_R_CIR_IRQ);

	INF("ir detected\n");
	if (ir_is_power_key()) {
		tvbox_save_rtc_flag(0x0f);
		LOG("reset system now\n");
		power_reset_system();
	}

	return TRUE;
}
#endif

s32 ir_init(void)
{
	//backup ir registers
	ir_ctrl   = readl(IR_CTRL_REG);
	ir_rxcfg  = readl(IR_RXCFG_REG);
	ir_rxinte = readl(IR_RXINTE_REG);
	ir_rxints = readl(IR_RXINTS_REG);
	ir_cfg    = readl(IR_CFG_REG);

	//backup ir clock configure
	ir_clk_src = ccu_get_mclk_src(CCU_MOD_CLK_R_CIR);
	ir_clk_div = ccu_get_mclk_div(CCU_MOD_CLK_R_CIR);

	/* config R_CIR pin */
#if (defined CONFIG_ARCH_SUN8IW6P1)
	pin_set_multi_sel(PIN_GRP_PL, 12, 2);             //PL12 config as R_CIR
	pin_set_pull(PIN_GRP_PL, 12, PIN_PULL_UP);        //R_CIR pull-up
	pin_set_drive(PIN_GRP_PL, 12, PIN_MULTI_DRIVE_0); //R_CIR drive level 0

	/* standby ir clock source=32k, div=1 or cource=24M, div=8 or
	 * IOSC/16/2/16=31250Hz=~32768Hz, 31250Hz can use 32768Hz paras, div = 1.
	 */
#if IR_24M_USED
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_HOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 8);
#else
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_LOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 32);
#endif
#elif (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1)
	pin_set_multi_sel(PIN_GRP_PL, 11, 2);             //PL11 config as R_CIR
	pin_set_pull(PIN_GRP_PL, 11, PIN_PULL_UP);        //R_CIR pull-up
	pin_set_drive(PIN_GRP_PL, 11, PIN_MULTI_DRIVE_0); //R_CIR drive level 0

	/* standby ir clock source=32k, div=1 or cource=24M, div=8 or
	 * IOSC/16/2/16=31250Hz=~32768Hz, 31250Hz can use 32768Hz paras, div = 1.
	 */
#if IR_24M_USED
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_HOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 8);
#else
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_LOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 1);
#endif
#elif (defined CONFIG_ARCH_SUN50IW6P1)
	pin_set_multi_sel(PIN_GRP_PL, 9, 2);             //PL9 config as R_CIR
	pin_set_pull(PIN_GRP_PL, 9, PIN_PULL_UP);        //R_CIR pull-up
	pin_set_drive(PIN_GRP_PL, 9, PIN_MULTI_DRIVE_0); //R_CIR drive level 0

	/* standby ir clock source=32k, div=1 or cource=24M, div=8 or
	 * IOSC/16/2/16=31250Hz=~32768Hz, 31250Hz can use 32768Hz paras, div = 1.
	 */
#if IR_24M_USED
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_HOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 8);
#else
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_LOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 1);
#endif
#else

	pin_set_multi_sel(PIN_GRP_PL, 4, 2);		 //PL4 config as R_CIR
	pin_set_pull(PIN_GRP_PL, 4, PIN_PULL_UP);	 //R_CIR pull-up
	pin_set_drive(PIN_GRP_PL, 4, PIN_MULTI_DRIVE_0); //R_CIR drive level 0

	/* standby ir clock source=32k, div=1 or cource=24M, div=8 */
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, CCU_SYS_CLK_LOSC);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, 1);
#endif
	//reset ir module
	ccu_reset_module(CCU_MOD_CLK_R_CIR);
	ccu_set_mclk_onoff(CCU_MOD_CLK_R_CIR, CCU_CLK_ON);

	//ir standby setup
	ir_clk_cfg();
	raw_buffer.count = 0;

#if (defined BOOT_USED) && (defined EVB_PLATFORM)
	/* register ir interrupt handler */
	writel(0xff, IR_RXINTS_REG);
	install_isr(INTC_R_CIR_IRQ, ir_int_handler, NULL);
	interrupt_clear_pending(INTC_R_CIR_IRQ);
	interrupt_enable(INTC_R_CIR_IRQ);
#elif (defined TF_USED) && (defined EVB_PLATFORM)
	interrupt_clear_pending(INTC_R_CIR_IRQ);
	interrupt_enable(INTC_R_CIR_IRQ);
#endif

	return OK;
}

#if ((defined KERNEL_USED) || (defined TF_USED))
s32 ir_exit(void)
{
	//backup ir clock configure
	ccu_set_mclk_src(CCU_MOD_CLK_R_CIR, ir_clk_src);
	ccu_set_mclk_div(CCU_MOD_CLK_R_CIR, ir_clk_div);

	//reset ir module
	ccu_reset_module(CCU_MOD_CLK_R_CIR);

	//restore ir registers
	writel(ir_ctrl   , IR_CTRL_REG);
	writel(ir_rxcfg  , IR_RXCFG_REG);
	writel(ir_rxinte , IR_RXINTE_REG);
	writel(ir_rxints , IR_RXINTS_REG);
	writel(ir_cfg    , IR_CFG_REG);

	return OK;
}
#endif

static s32 ir_verify_addr(void)
{
	u32 status;
	u32 i;

	/* read out interrupt pending status register */
	status = readl(IR_RXINTS_REG);
	/* clear interrupt pending */
	writel(status, IR_RXINTS_REG);

	INF("status:%x\n", status);
	if (status & (0x1 << 4)) {
		/* rx fifo available interrupt */
		if (ir_read_fifo_raw_data(&raw_buffer) != OK) {
			//discard all received data
			WRN("read ir fifo raw data failed\n");
			raw_buffer.count = 0;
			return FALSE;
		}
	}
	if (status & (0x1 << 1)) {
		/* ir rx package end interrupt */
		if (ir_read_fifo_raw_data(&raw_buffer) != OK) {
			WRN("read ir fifo raw data failed\n");
			raw_buffer.count = 0;
			return FALSE;
		}

		/* decode raw data */
		ir_valid_value = ir_decode_raw_data(raw_buffer.data, raw_buffer.count);
		INF("ir_valid_value:%x\n", ir_valid_value);
		raw_buffer.count = 0;

		/* check the code valid or not */
		for (i = 0; i < ir_key_depot.num; i++) {
			if (ir_code_is_valid(ir_key_depot.ir_code_depot[i].addr_code, ir_valid_value)) {
				INF("detect ir raw code :%x\n", ir_valid_addr);
				return TRUE;
			}
		}
		/* invalid code detect */
		return FALSE;
	}
	if (status & (0x1 << 0)) {
		/* ir rx fifo full interrupt */
		WRN("ir rx fifo full\n");
		raw_buffer.count = 0;
		return FALSE;
	}

	/* invalid ir rx interrupt detect */
	ERR("inv ir rx irq det, st:0x%x\n", status);

	return FALSE;
}

#if (defined KERNEL_USED)
s32 ir_set_paras(struct message *pmessage)
{
	ir_key_depot.ir_code_depot[0].key_code = pmessage->paras[0];
	ir_key_depot.ir_code_depot[0].addr_code = pmessage->paras[1];
	ir_key_depot.num = 1;
	LOG("ir key:%x, addr:%x\n", ir_key_depot.ir_code_depot[0].key_code, \
	    ir_key_depot.ir_code_depot[0].addr_code);

	return OK;
}
#elif (defined TF_USED)
s32 ir_set_paras(ir_key_t *ir_key)
{
	u32 i;

	memcpy(&ir_key_depot, ir_key, sizeof(ir_key_t));
	for (i = 0; i < ir_key_depot.num; i++) {
		INF("ir key:%x, addr:%x\n", ir_key_depot.ir_code_depot[i].key_code, \
	    		ir_key_depot.ir_code_depot[i].addr_code);
	}

	return OK;
}

u32 ir_is_used(void)
{
	return arisc_para.start_os.irkey_used;
}

void ir_sysconfig_cfg(void)
{
	//LOG("irkey_used:%d %d\n", arisc_para.start_os.irkey_used);
	if (!arisc_para.start_os.irkey_used)
		return;

	ir_init();
	INF("ir driver ok\n");
}
#else /* !((defined KERNEL_USED) || (defined TF_USED)) */
static int irkey_used = 0;

s32 ir_set_paras(ir_code_t *ir_code, int index)
{
	int i;

	/* check repeat configuration */
	for (i = 0; i < ir_key_depot.num; i++)
		if ((ir_key_depot.ir_code_depot[i].key_code == ir_code->key_code) && \
		    (ir_key_depot.ir_code_depot[i].addr_code == ir_code->addr_code))
			return OK;

	/* just cover the last ir config */
	if (ir_key_depot.num >= IR_NUM_KEY_SUP)
		ir_key_depot.num--;

	ir_key_depot.ir_code_depot[ir_key_depot.num].key_code = ir_code->key_code;
	ir_key_depot.ir_code_depot[ir_key_depot.num].addr_code = ir_code->addr_code;
	LOG("ir num:%d, key:%x, add:%x\n", ir_key_depot.num, ir_code->key_code, ir_code->addr_code);
	ir_key_depot.num++;

	return OK;
}

void ir_sysconfig_cfg(void)
{
	ir_code_t ir_code;
	int count;
	int state;
	int i, val;

	val = script_parser_fetch("box_start_os", "irkey_used", &irkey_used, 1);
	LOG("irkey_used:%d %d\n", irkey_used, val);
	if (!irkey_used)
		return;

	count = script_parser_subkey_count("s_cir0");
	LOG("s_cir0 count:%d\n", count);
	script_parser_fetch("s_cir0", "ir_used", &val, 1);
	LOG("used:%d\n", val);
	count >>= 1; /* count/2 */
	count = (count < IR_NUM_KEY_SUP) ? count : IR_NUM_KEY_SUP;
	LOG("coun:%d\n", count);
	if (val) {
		char key_buf[32] = "ir_power_key_code";
		char add_buf[32] = "ir_addr_code";

		for (i = 0; i < count; i++) {
			sprintf(key_buf + 17, "%d", i);
			INF("%s\n", key_buf);
			sprintf(add_buf + 12, "%d", i);
			INF("%s\n", add_buf);

			state = script_parser_fetch("s_cir0", key_buf, &val, 1);
			if (SCRIPT_PARSER_OK != state)
				continue;

			ir_code.key_code = val;
			LOG("key:%x state:%d\n", val, state);

			state = script_parser_fetch("s_cir0", add_buf, &val, 1);
			if (SCRIPT_PARSER_OK != state)
				continue;
			ir_code.addr_code = val;
			LOG("add:%x state:%d\n", val, state);
			ir_set_paras(&ir_code, i);
		}
	}

	ir_init();
	LOG("ir driver ok\n");
}

u32 ir_is_used(void)
{
	return irkey_used;
}
#endif /* ((defined KERNEL_USED) || (defined TF_USED)) */

u32 ir_is_power_key(void)
{
	int i;
	int state = FALSE;

/* step.1 get ir key/addr and verify addr */
	if (!ir_verify_addr())
		return state;

/* step.2 check key and addr */
	for (i = 0; i < ir_key_depot.num; i++) {
		if ((ir_valid_code == ir_key_depot.ir_code_depot[i].key_code) && \
		    (ir_valid_addr == ir_key_depot.ir_code_depot[i].addr_code)) {
			state = TRUE;
			break;
		}
	}

	return state;
}

#ifdef IR_TEST
s32 ir_test(void)
{
	u32 value = 0;

	ir_init();
	while (1) {
#ifdef FPGA_PLATFORM
		value = readl(IR_RXINTS_REG);
		if (value & 0x13) {
#else
		if (interrupt_query_pending(INTC_R_CIR_IRQ)) {
#endif
			LOG("ir detected\n");
			if (ir_is_power_key()) {
				LOG("cir ok\n");
				tvbox_save_rtc_flag(0x0f);
				power_reset_system();
			}
			writel(value, IR_RXINTS_REG);
		}
		time_mdelay(5);
	}

	ir_exit();
}
#endif
#endif /* IR_USED */
