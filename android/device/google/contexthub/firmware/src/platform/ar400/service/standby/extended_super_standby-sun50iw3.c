/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                               standby module
*
*                                    (c) Copyright 2012-2016, superm China
*                                             All Rights Reserved
*
* File    : extended_super_standby.c
* By      : superm
* Version : v1.0
* Date    : 2013-1-16
* Descript: extended super-standby module public header.
* Update  : date                auther      ver     notes
*           2013-1-16 17:04:28  superm       1.0     Create this file.
*********************************************************************************************************
*/

#include "standby_i.h"
#if defined CONFIG_ARCH_SUN50IW3P1
#define DEBUG_DM 0
#if EST_USED
extern u32 dram_crc_enable;
extern u32 dram_crc_src;
extern u32 dram_crc_len;
static s32 result = 0;
static u32 cpus_src = 0;
static u32 pll_periph0, mbus;
static u32 gwake_event = 0;
static u32 dm_on;
static u32 dm_off;
static unsigned short volt_bak[DM_MAX];
static bool output_cnt[AW1660_POWER_MAX];
static u32 pl_debounce_cfg_bak;
static u32 pm_debounce_cfg_bak;
static struct extended_standby extended_config;
static struct extended_standby *pextended_config = &extended_config;

/* power domain */
#define IS_DM_ON(dm)  ((dm_on >> dm) & 0x1)
#define IS_SYS_ON     ((dm_on >> DM_SYS) & 0x1)

#define IS_DM_OFF(dm)   (!((dm_off >> dm) & 0x1))
#define IS_CPUA_OFF     (!((dm_off >> DM_CPUA) & 0x1))
#define IS_CPUB_OFF     (!((dm_off >> DM_CPUB) & 0x1))
#define IS_DRAM_OFF     (!((dm_off >> DM_DRAM) & 0x1))
#define IS_GPU_OFF      (!((dm_off >> DM_GPU) & 0x1))
#define IS_SYS_OFF      (!((dm_off >> DM_SYS) & 0x1))
#define IS_VPU_OFF      (!((dm_off >> DM_VPU) & 0x1))
#define IS_CPUS_OFF     (!((dm_off >> DM_CPUS) & 0x1))
#define IS_DRAMPLL_OFF  (!((dm_off >> DM_DRAMPLL) & 0x1))
#define IS_ADC_OFF      (!((dm_off >> DM_ADC) & 0x1))
#define IS_PL_OFF       (!((dm_off >> DM_PL) & 0x1))
#define IS_PM_OFF       (!((dm_off >> DM_PM) & 0x1))
#define IS_CPVDD_OFF    (!((dm_off >> DM_CPVDD) & 0x1))
#define IS_LDOIN_OFF    (!((dm_off >> DM_LDOIN) & 0x1))

/*
 * clock
 * n is PLLn
 */
#define IS_DIS_PLL(n)     (!(pextended_config->cpux_clk_state.init_pll_dis & (0x1 << n)))
#define IS_EN_PLL(n)      (pextended_config->cpux_clk_state.exit_pll_en & (0x1 << n))
#define IS_CHANGE_PLL(n)  (pextended_config->cpux_clk_state.pll_change & (0x1 << n))
#define PLLCFG(n)         (pextended_config->cpux_clk_state.pll_factor[n])

#define IS_CHANGE_BUS(n)  (pextended_config->cpux_clk_state.bus_change & (0x1 << n))
#define BUSCFG(n)         (pextended_config->cpux_clk_state.bus_factor[n])

#define IS_DIS_OSC(n)     (!(pextended_config->cpux_clk_state.osc_en & (0x1 << n)))

/* gpio */
#define IOCFG(n)          (pextended_config->soc_io_state.io_state[n])
#define IS_GPIO_HOLD      (pextended_config->soc_io_state.hold_flag)

/* dram */
#define IS_DRAM_SUSPEND   (pextended_config->soc_dram_state.selfresh_flag)
#define DRAM_CRC_EN       (pextended_config->soc_dram_state.crc_en)
#define DRAM_CRC_START    (pextended_config->soc_dram_state.crc_start)
#define DRAM_CRC_LEN      (pextended_config->soc_dram_state.crc_len)
/*
void reg_debug(const char *name, const unsigned int *start, unsigned int len)
{
	int i;
	char buf[16];

	ERR("%s:", name);
	for (i = 0; i < len; i++) {
		if ((i & 3) == 0)
			ERR("\n[%8x]:", start + i);
		ERR("%8x", *(start + i));
	}
	ERR("\n");
}
*/
static void set_wakeup_src(struct super_standby_para *para)
{
	interrupt_standby_enter();

	if ((para->event & CPUS_WAKEUP_NMI) ||
		(para->event & CPUS_WAKEUP_GPIO))
	{
		//enable exteral intterrupt wakeup
		//long_jump((long_jump_fn)mem_pmu_standby_init, (void *)para);
		mem_pmu_standby_init(para);
		interrupt_enable(EXT_NMI_IRQn);
		//INF("NMI/GPIO\n");
	}
	//long_jump((long_jump_fn)mem_int_suspend_cfg, (void *)para);
	mem_int_suspend_cfg(para);
}

static void get_wakeup_src(struct super_standby_para *para)
{
	u32 timeout = 0;
#if PMU_CHRCUR_CRTL_USED
	u32    bat_delay = 0;
#endif
	gwake_event = 0;
	led_bln_suspend(0);
	while (1)
	{
		if ((para->event & CPUS_WAKEUP_NMI) ||
			(para->event & CPUS_WAKEUP_GPIO))
		{
			if (interrupt_query_pending(EXT_NMI_IRQn))
			{
				interrupt_clear_pending(EXT_NMI_IRQn);
				//INF("pmu wakeup detect\n");
				if(pmu_query_event(&gwake_event) == OK)
				{
					break;
				}
			}
		}
		if (para->event & CPUS_WAKEUP_GPIO) {
			if (interrupt_query_pending(R_GPIOL_IRQn) ||
				interrupt_query_pending(R_GPIOM_IRQn)   ||
				interrupt_query_pending(GPIOB_IRQn)     ||
				interrupt_query_pending(GPIOF_IRQn)     ||
				interrupt_query_pending(GPIOG_IRQn)     ||
				interrupt_query_pending(GPIOH_IRQn)) {
				//FIXME:gpio_enable_bitmap not enough bitwide to support over 12 PL pin
				//A63 has 20 PL pin
				if (is_wanted_gpio_int(PIN_GRP_PL, (para->gpio_enable_bitmap) & GPIO_PL_MASK) ||
					(is_wanted_gpio_int(PIN_GRP_PM, ((para->gpio_enable_bitmap) & GPIO_PM_MASK) >> 12))) {
					INF("pin wakeup detect\n");
					gwake_event = CPUS_WAKEUP_GPIO;
					break;
				}
			}
		}
#if  IR_USED
		if (para->event & CPUS_WAKEUP_IR)
		{
			if (interrupt_query_pending(INTC_R_CIR_IRQ)) {
				interrupt_clear_pending(INTC_R_CIR_IRQ);
				if (ir_is_power_key() == TRUE) {
					INF("cir wakeup detect\n");
					gwake_event = CPUS_WAKEUP_IR;
					break;
				}
			}
		}
#endif // IR_USED
		if (para->event & CPUS_WAKEUP_ALM0) {
			if (interrupt_query_pending(R_ALARM0_IRQn)) {
				INF("alarm0 wakeup detect\n");
				gwake_event = CPUS_WAKEUP_ALM0;
				break;
			}
		}
		if (para->event & CPUS_WAKEUP_ALM1) {
			if (interrupt_query_pending(R_ALARM1_IRQn)) {
				INF("alarm1 wakeup detect\n");
				gwake_event = CPUS_WAKEUP_ALM1;
				break;
			}
		}
		if (para->event & CPUS_WAKEUP_TIMEOUT)
		{
			if (timeout < para->timeout)
			{
				time_mdelay(1);
				timeout++;
			}
			else
			{
				//INF("timeout wakeup detect\n");
				gwake_event = CPUS_WAKEUP_TIMEOUT;
				break;
			}
			//WRN("sstandby not support timeout wakeup\n");
		}
		if (para->event & CPUS_WAKEUP_USBMOUSE)
		{
			if (interrupt_query_pending(USB_DRD_DEV_IRQn) || \
				interrupt_query_pending(USB_DRD_EHCI_IRQn) || \
				interrupt_query_pending(USB_DRD_OHCI_IRQn) || \
				interrupt_query_pending(USB_HOST_EHCI_IRQn) || \
				interrupt_query_pending(USB_HOST_OHCI_IRQn)) {
				INF("USB mouse wakeup detect\n");
				gwake_event = CPUS_WAKEUP_USBMOUSE;
				break;
			}
		}
		if (para->event & CPUS_WAKEUP_CODEC)
		{
			if (interrupt_query_pending(ACDET_IRQn) || interrupt_query_pending(AC_IRQn))
			{
				//INF("CODEC wakeup detect\n");
				gwake_event = CPUS_WAKEUP_CODEC;
				break;
			}
		}
		if (para->event & CPUS_WAKEUP_LRADC)
		{
			if (interrupt_query_pending(LRADC_IRQn))
			{
				//INF("LRADC wakeup detect\n");
				gwake_event = CPUS_WAKEUP_LRADC;
				break;
			}
		}
		led_bln_adjust(0);
#if PMU_CHRCUR_CRTL_USED
		bat_delay++;
		if (bat_delay == 43750*CHRCUR_CRTL_PER_SECONDS) {
			pmu_contrl_batchrcur();
			bat_delay = 0;
		}
#endif
	}
	led_bln_resume(0);
}

static void wait_cpu0_resume(void)
{
#if !(AR100_TEST_EN)
	//start system watchdog to avoid cpu0 reset fail
	//watchdog_enable();

	printk("wait ac327 resume...\n");
	//wait cpu0 restore finished.
	while (1) {
		//INF("0x01f00104 = %x\n", readl(0x01f00104));
		//ar100 cpu interrupt is disable now,
		//we should query message by hand.
		struct message *pmessage = hwmsgbox_query_message();
		if (pmessage == NULL)
			continue; //no message, query again

		//query valid message
		if (pmessage->type == SSTANDBY_RESTORE_NOTIFY) {
			//cpu0 restore, feedback wakeup event.
			//LOG("cpu0 restore finished\n");
			pmessage->paras[0] = gwake_event;
			pmessage->paras[1] = ((after_crc != before_crc) ? 1 : 0);
			pmessage->state    = MESSAGE_PROCESSED;
		} else {
			/* invalid message detected, ignore it, by sunny at 2012-6-28 11:33:13. */
			ERR("esstandby ignore message [%p, %x]\n", pmessage, pmessage->type);
		}
		if (pmessage->attr & (MESSAGE_ATTR_SOFTSYN | MESSAGE_ATTR_HARDSYN))
			hwmsgbox_feedback_message(pmessage, SEND_MSG_TIMEOUT); //synchronous message, need feedback.
		else
			message_free(pmessage); //asyn message, free message directly.

		if (pmessage->state == MESSAGE_PROCESSED)
			break;
		//we need waiting continue.
	}
	//disable wacthdog
	//watchdog_disable();
#endif
}

#if DEBUG_DM
u8 AW1660_OUTPUT_PWR_CTRL0_BAK, AW1660_OUTPUT_PWR_CTRL1_BAK, AW1660_ALDO123_OP_MODE_BAK;
static void print_output_state(void)
{
	u8 devaddr = RSB_RTSADDR_AW1660;
	u8 regaddr = AW1660_OUTPUT_PWR_CTRL0;

	regaddr = AW1660_OUTPUT_PWR_CTRL0;
	pmu_reg_read(&devaddr, &regaddr, &AW1660_OUTPUT_PWR_CTRL0_BAK, 1);
	LOG("reg%x:%x\n", AW1660_OUTPUT_PWR_CTRL0, AW1660_OUTPUT_PWR_CTRL0_BAK);
	regaddr = AW1660_OUTPUT_PWR_CTRL1;
	pmu_reg_read(&devaddr, &regaddr, &AW1660_OUTPUT_PWR_CTRL1_BAK, 1);
	LOG("reg%x:%x\n", AW1660_OUTPUT_PWR_CTRL1, AW1660_OUTPUT_PWR_CTRL1_BAK);

	regaddr = AW1660_ALDO123_OP_MODE;
	pmu_reg_read(&devaddr, &regaddr, &AW1660_ALDO123_OP_MODE_BAK, 1);
	LOG("reg%x:%x\n", AW1660_ALDO123_OP_MODE, AW1660_ALDO123_OP_MODE_BAK);
}
#endif

static void dm_suspend(void)
{
	/* one dm maybe have some output */
	s32 dm, output;

	pmu_set_mode(AW1660_POWER_DCDC4, REGU_MODE_AUTO); //set vdd-gpu to auto switch mode to save consumption
	pmu_set_mode(AW1660_POWER_DCDC5, REGU_MODE_AUTO); //set vcc-dram to auto switch mode to save consumption

	//print_output_state();
	//hexdump("powertree", (char *)arisc_para.power_regu_tree, sizeof(arisc_para.power_regu_tree));
	memset(output_cnt, 0, sizeof(output_cnt));
	for (dm = DM_MAX - 1; dm >= 0; dm--) {
		if (IS_DM_ON(dm)) {
			for (output = 0; output < AW1660_POWER_MAX; output++) {
				if ((arisc_para.power_regu_tree[dm] >> output) & 0x1) {
					output_cnt[output] = 1;
					if (pextended_config->soc_pwr_dm_state.volt[dm] != 0) {
						volt_bak[dm] = pmu_get_voltage(output);
						pmu_set_voltage(output, pextended_config->soc_pwr_dm_state.volt[dm]);
						//printk(" DM:%d, volt_bak[dm] :%d,pextended_config->soc_pwr_dm_state.volt[dm]:%d\n", dm, volt_bak[dm],pextended_config->soc_pwr_dm_state.volt[dm]);

					}
				}
			}
		}
	}
	for (dm = DM_MAX - 1; dm >= 0; dm--) {
		if (IS_DM_OFF(dm)) {
			for (output = 0; output < AW1660_POWER_MAX; output++) {
				if ((arisc_para.power_regu_tree[dm] >> output) & 0x1) {
					if (output_cnt[output] == 0) {
						pmu_set_voltage_state(output, POWER_VOL_OFF);
					}
				}
			}
		}
	}

	//print_output_state();
}

static void dm_resume(void)
{
	s32 dm, output;

	for (dm = 0; dm < DM_MAX; dm++) {
		if (IS_DM_ON(dm)) {
			if (pextended_config->soc_pwr_dm_state.volt[dm] != 0) {
				for (output = 0; output < AW1660_POWER_MAX; output++) {
					if ((arisc_para.power_regu_tree[dm] >> output) & 0x1) {
						pmu_set_voltage(output, volt_bak[dm]);
					}
				}
			}
		}
	}
	for (dm = 0; dm < DM_MAX; dm++) {
		if (IS_DM_OFF(dm)) {
			for (output = 0; output < AW1660_POWER_MAX; output++) {
				if ((arisc_para.power_regu_tree[dm] >> output) & 0x1) {
					if (output_cnt[output] == 0) {
						pmu_set_voltage_state(output, POWER_VOL_ON);
					}
				}
			}
		}
	}

	pmu_set_mode(AW1660_POWER_DCDC4, REGU_MODE_PWM); //set vdd-gpu to pwm mode
	pmu_set_mode(AW1660_POWER_DCDC5, REGU_MODE_PWM); //set vcc-dram to pwm mode

	//print_output_state();
}

static void dram_suspend(struct super_standby_para *para)
{
	dram_disable_all_master();
	if (IS_DRAM_SUSPEND) {
		dram_crc_enable = DRAM_CRC_EN;
		dram_crc_src    = DRAM_CRC_START;
		dram_crc_len    = DRAM_CRC_LEN;
		//calc dram checksum
		if (standby_dram_crc_enable())
		{
			before_crc = standby_dram_crc();
		}

		pll_periph0 = readl(CCU_PLL_PERIPH0_REG);
		mbus = readl(CCU_MBUS_CLK_REG);
		dram_power_save_process();
	}
}

static void dram_resume(struct super_standby_para *para)
{
	if (IS_DRAM_SUSPEND) {
		//restore dram controller and transing area.
		//INF("power-up dram\n");
		writel(readl(CCU_PLL_PERIPH0_REG) & (~(1 << 31)), CCU_PLL_PERIPH0_REG); //disable pll_periph0 firstly
		writel((pll_periph0&(~(0x1 << 31))), CCU_PLL_PERIPH0_REG); //set pll factor but not enable pll
		time_udelay(100); //delay 100us for factor to be effective
		writel((readl(CCU_PLL_PERIPH0_REG) | (0x1 << 31)), CCU_PLL_PERIPH0_REG); //enable pll
		time_udelay(20);
		//mbus default clk src is 24MHz, switch to pll_periph0(x2),
		//so before increase mbus freq, should set div firstly.
		//by Superm Wu at 2015-09-18
#ifndef FPGA_PLATFORM
		writel(mbus&0x7, CCU_MBUS_CLK_REG);
		time_udelay(200);
		writel(mbus&((0x3 << 24) | 0x7), CCU_MBUS_CLK_REG);
		time_udelay(20);
		writel((readl(CCU_MBUS_CLK_REG) | (0x1 << 31)), CCU_MBUS_CLK_REG);
		//hexdump("dram_paras", (char *)&arisc_para.dram_para, 0x4 * 24);
		time_udelay(10000);
		dram_power_up_process();
#endif

		//calc dram checksum
		if (standby_dram_crc_enable())
		{
			dram_master_enable_cpus();
			after_crc = standby_dram_crc();
			if(after_crc != before_crc)
			{
				save_state_flag(REC_SSTANDBY | REC_DRAM_DBG | 0xf);//RECORD
				printk("dram crc error...\n");
				//ERR("---->>>>LOOP<<<<----\n");
				while(1);
			}
		}
	}
	dram_enable_all_master();
}

static void clk_restore(void)
{
		//resotre cpu and bus ccu register to reset value
		//cpu/axi
		ccu_set_mclk_src(CCU_MOD_CLK_C0, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_CPU_APB, 1);
		ccu_set_mclk_div(CCU_MOD_CLK_AXI, 2);

		//psi/ahb1/ahb2
		//in the same config, only set once.
		ccu_set_mclk_src(CCU_MOD_CLK_AHB1, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB1, 1);
		time_mdelay(1);

		//ahb3
		ccu_set_mclk_src(CCU_MOD_CLK_AHB3, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB3, 1);
		time_mdelay(1);

		//apb1
		ccu_set_mclk_src(CCU_MOD_CLK_APB1, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB1, 1);
		time_mdelay(1);

		//apb2
		ccu_set_mclk_src(CCU_MOD_CLK_APB2, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB2, 1);
		time_mdelay(1);
}

static s32 spi_clk_src_bk;
static void clk_suspend(void)
{
	//LOG("init_pll_dis:%x\n", pextended_config->cpux_clk_state.init_pll_dis);
	//LOG("exit_pll_en:%x\n", pextended_config->cpux_clk_state.exit_pll_en);
	//LOG("pll_change:%x\n", pextended_config->cpux_clk_state.pll_change);
	//LOG("bus_change:%x\n", pextended_config->cpux_clk_state.bus_change);
	//LOG("osc_en:%x\n", pextended_config->cpux_clk_state.osc_en);
	spi_clk_src_bk = ccu_get_mclk_src(CCU_MOD_CLK_R_SPI);
	ccu_set_mclk_src(CCU_MOD_CLK_R_SPI, CCU_SYS_CLK_16M);

	if (IS_SYS_ON)
		clk_restore();

	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x91);

	//switch AHB1 and APB2 clock source to 23K LOSC
	//because the ccu is on the apb1, and the ahb1/apb2
	//mabye use the pll_periph0 clk src, but the pll_periph0 mabye be
	//closed depend on the extended standby config table.
	ccu_set_mclk_src(CCU_MOD_CLK_PSI, CCU_SYS_CLK_LOSC);  //psi/ahb1/ahb2
	ccu_set_mclk_src(CCU_MOD_CLK_AHB3, CCU_SYS_CLK_LOSC); //ahb3
	ccu_set_mclk_src(CCU_MOD_CLK_APB1, CCU_SYS_CLK_LOSC); //apb1
	ccu_set_mclk_src(CCU_MOD_CLK_APB2, CCU_SYS_CLK_LOSC); //apb2
	ccu_set_mclk_src(CCU_MOD_CLK_C0, CCU_SYS_CLK_LOSC); //cpu+axi

	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x92);

	//disable PLLs
	static volatile u32 pll_value;

	//c0
	if (IS_DIS_PLL(PLL_C0))
	{
		writel(((readl(CCU_PLL_C0_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_C0_REG);
	}
	if (IS_CHANGE_PLL(PLL_C0))
	{
		pll_value = (readl(CCU_PLL_C0_REG) & (~0x3ffff));
		pll_value = (pll_value | (PLLCFG(PLL_C0).factor3 << 16) |
			(PLLCFG(PLL_C0).factor2 << 8) | (PLLCFG(PLL_C0).factor1));
		writel(pll_value, CCU_PLL_C0_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x921);

	//ddr0
	if (IS_DIS_PLL(PLL_DRAM))
	{
		//SDRPLL configration updata,
		//after enable pll_ddr0, this bit should be set to 1 to validate PLL6.
		writel(((readl(CCU_PLL_DDR0_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_DDR0_REG);
	}
	if (IS_CHANGE_PLL(PLL_DRAM))
	{
		pll_value = (readl(CCU_PLL_DDR0_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_DRAM).factor3 << 8) | (PLLCFG(PLL_DRAM).factor2 << 1) |
			(PLLCFG(PLL_DRAM).factor1));
		writel(pll_value, CCU_PLL_DDR0_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x922);

	//ddr1
	if (IS_DIS_PLL(PLL_DRAM1))
	{
		//SDRPLL configration updata,
		//after enable PLL6, this bit should be set to 1 to validate PLL6.
		writel(((readl(CCU_PLL_DDR1_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_DDR1_REG);
		writel((readl(CCU_PLL_DDR1_REG) | (0x1 << 30)), CCU_PLL_DDR1_REG);
		while((readl(CCU_PLL_DDR1_REG) >> 30) & 0x1);
	}
	if (IS_CHANGE_PLL(PLL_DRAM1))
	{
		pll_value = (readl(CCU_PLL_DDR1_REG) & (~0x7fff));
		pll_value = (pll_value | (PLLCFG(PLL_DRAM1).factor3 << 8)  | (PLLCFG(PLL_DRAM1).factor2 << 1) | (PLLCFG(PLL_DRAM1).factor1));
		writel(pll_value, CCU_PLL_DDR1_REG);
		writel((readl(CCU_PLL_DDR1_REG) | (0x1 << 30)), CCU_PLL_DDR1_REG);
		while((readl(CCU_PLL_DDR1_REG) >> 30) & 0x1);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x923);

	//periph0
	if (IS_DIS_PLL(PLL_PERIPH))
	{
		writel(((readl(CCU_PLL_PERIPH0_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_PERIPH0_REG);
	}
	if (IS_CHANGE_PLL(PLL_PERIPH))
	{
		pll_value = (readl(CCU_PLL_PERIPH0_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_PERIPH).factor3 << 8) |
			(PLLCFG(PLL_PERIPH).factor2 << 1) | (PLLCFG(PLL_PERIPH).factor1 << 0));
		writel(pll_value, CCU_PLL_PERIPH0_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x924);


	//periph1
	if (IS_DIS_PLL(PLL_PERIPH1))
	{
		writel(((readl(CCU_PLL_PERIPH1_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_PERIPH1_REG);
	}
	if (IS_CHANGE_PLL(PLL_PERIPH1))
	{
		pll_value = (readl(CCU_PLL_PERIPH1_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_PERIPH1).factor3 << 8) |
			(PLLCFG(PLL_PERIPH1).factor2 << 1) | (PLLCFG(PLL_PERIPH1).factor1 << 0));
		writel(pll_value, CCU_PLL_PERIPH1_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x925);

	//gpu
	if (IS_DIS_PLL(PLL_GPU))
	{
		writel(((readl(CCU_PLL_GPU0_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_GPU0_REG);
	}
	if (IS_CHANGE_PLL(PLL_GPU))
	{
		pll_value = (readl(CCU_PLL_GPU0_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_GPU).factor3 << 8) | (PLLCFG(PLL_GPU).factor2 << 1) | (PLLCFG(PLL_GPU).factor1));
		writel(pll_value, CCU_PLL_GPU0_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x927);

	//video0
	if (IS_DIS_PLL(PLL_VIDEO0))
	{
		writel(((readl(CCU_PLL_VIDEO0_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_VIDEO0_REG);
	}
	if (IS_CHANGE_PLL(PLL_VIDEO0))
	{
		pll_value = (readl(CCU_PLL_VIDEO0_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_VIDEO0).factor3 << 8) | (PLLCFG(PLL_VIDEO0).factor2 << 1) | (PLLCFG(PLL_VIDEO0).factor1));
		writel(pll_value, CCU_PLL_VIDEO0_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x928);

	//video1
	if (IS_DIS_PLL(PLL_VIDEO1))
	{
		writel(((readl(CCU_PLL_VIDEO1_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_VIDEO1_REG);
	}
	if (IS_CHANGE_PLL(PLL_VIDEO1))
	{
		pll_value = (readl(CCU_PLL_VIDEO1_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_VIDEO1).factor3 << 8) | (PLLCFG(PLL_VIDEO1).factor2 << 1) | (PLLCFG(PLL_VIDEO1).factor1));
		writel(pll_value, CCU_PLL_VIDEO1_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x929);

	//ve
	if (IS_DIS_PLL(PLL_VE))
	{
		writel(((readl(CCU_PLL_VE_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_VE_REG);
	}
	if (IS_CHANGE_PLL(PLL_VE))
	{
		pll_value = (readl(CCU_PLL_VE_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_VE).factor3 << 8) | (PLLCFG(PLL_VE).factor2 << 1) | (PLLCFG(PLL_VE).factor1));
		writel(pll_value, CCU_PLL_VE_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x930);

	//de
	if (IS_DIS_PLL(PLL_DE))
	{
		writel(((readl(CCU_PLL_DE_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_DE_REG);
	}
	if (IS_CHANGE_PLL(PLL_DE))
	{
		pll_value = (readl(CCU_PLL_DE_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_DE).factor3 << 8) | (PLLCFG(PLL_DE).factor2 << 1) | (PLLCFG(PLL_DE).factor1));
		writel(pll_value, CCU_PLL_DE_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x931);

	//hsic
	if (IS_DIS_PLL(PLL_HSIC))
	{
		writel(((readl(CCU_PLL_HSIC_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_HSIC_REG);
	}
	if (IS_CHANGE_PLL(PLL_HSIC))
	{
		pll_value = (readl(CCU_PLL_HSIC_REG) & (~0xffff));
		pll_value = (pll_value | (PLLCFG(PLL_HSIC).factor3 << 8) | (PLLCFG(PLL_HSIC).factor2 << 1) | (PLLCFG(PLL_HSIC).factor1));
		writel(pll_value, CCU_PLL_HSIC_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x932);

	//audio
	if (IS_DIS_PLL(PLL_AUDIO))
	{
		writel(((readl(CCU_PLL_AUDIO_REG) & (~(0x1 << 31))) | (0x0 << 31)), CCU_PLL_AUDIO_REG);
	}
	if (IS_CHANGE_PLL(PLL_AUDIO))
	{
		pll_value = (readl(CCU_PLL_AUDIO_REG) & (~0x3fffff));
		pll_value = (pll_value | (PLLCFG(PLL_AUDIO).factor4 << 16) | (PLLCFG(PLL_AUDIO).factor3 << 8) |
			(PLLCFG(PLL_AUDIO).factor2 << 1) | (PLLCFG(PLL_AUDIO).factor1));
		writel(pll_value, CCU_PLL_AUDIO_REG);
	}
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x933);

	//set bus src and src factor/div
//	static u32 bus_value;
	if (IS_CHANGE_BUS(BUS_C0))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_C0, BUSCFG(BUS_C0).src);
	}

	if (IS_CHANGE_BUS(BUS_AXI0))
	{
		ccu_set_mclk_div(CCU_MOD_CLK_AXI, BUSCFG(BUS_AXI0).div_ratio);
	}

	if (IS_CHANGE_BUS(BUS_PSI) || IS_CHANGE_BUS(BUS_AHB1) || IS_CHANGE_BUS(BUS_AHB2))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_AHB1, BUSCFG(BUS_AHB1).src);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB1, BUSCFG(BUS_AHB1).div_ratio);
	}

	if (IS_CHANGE_BUS(BUS_AHB3))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_AHB3, BUSCFG(BUS_AHB3).src);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB3, BUSCFG(BUS_AHB3).div_ratio);
	}

	if (IS_CHANGE_BUS(BUS_APB1))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_APB1, BUSCFG(BUS_APB1).src);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB1, BUSCFG(BUS_APB1).div_ratio);
	}

	if (IS_CHANGE_BUS(BUS_APB2))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_APB2, BUSCFG(BUS_APB2).src);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB2, BUSCFG(BUS_APB2).div_ratio);
	}

	//disable oscs
	if (IS_DIS_OSC(OSC_HOSC))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_IOSC);
		ccu_set_mclk_src(CCU_MOD_CLK_APBS2, CCU_SYS_CLK_IOSC);
		ccu_set_mclk_src(CCU_MOD_CLK_SYSTICK, CCU_SYS_CLK_LOSC);
		//hexdump("prcm", (char *)R_PRCM_REG_BASE, 0x3f0);
		time_mdelay(1);
		ccu_24mhosc_disable();
	}
	else if (IS_DIS_OSC(OSC_LDO0) && IS_DIS_OSC(OSC_LDO1))
	{
		volatile u32 value;
		//disable ldo/ldo1 if all plls disable
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		writel(value, CCU_PLL_CTRL1);
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		value &= (~(0x3 << 0));
		writel(value, CCU_PLL_CTRL1);
	}
}

static void clk_resume(void)
{
	if (IS_DIS_OSC(OSC_HOSC))
	{
		ccu_24mhosc_enable();
		time_mdelay(1);
		ccu_set_mclk_src(CCU_MOD_CLK_SYSTICK, CCU_SYS_CLK_HOSC);
		ccu_set_mclk_src(CCU_MOD_CLK_APBS2, CCU_SYS_CLK_HOSC);
		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_HOSC);
	}
	else if (IS_DIS_OSC(OSC_LDO0) && IS_DIS_OSC(OSC_LDO1))
	{
		volatile u32 value;

		//power-on pll ldo
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		writel(value, CCU_PLL_CTRL1);
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		value |= (0x1 << 0);
		writel(value, CCU_PLL_CTRL1);

		//wait 2ms for power-on ready
		time_mdelay(2);

		//adjust pll voltage to 1.45v
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		writel(value, CCU_PLL_CTRL1);
		value = (readl(CCU_PLL_CTRL1) | (0xa7 << 24));
		value &= (~(0x7 << 16));
		value |= (0x4 << 16);
		writel(value, CCU_PLL_CTRL1);

		//wait 2ms for voltage ready
		time_mdelay(2);
	}

	if (IS_CHANGE_BUS(BUS_C0) || IS_CHANGE_BUS(BUS_AXI0))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_C0, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AXI, 2);
	}

	//restore ahb1 and apb2 to defult value if they are changed when initialization
	if (IS_CHANGE_BUS(BUS_PSI) || IS_CHANGE_BUS(BUS_AHB1) || IS_CHANGE_BUS(BUS_AHB2))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_AHB1, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB1, 1);
		time_mdelay(1);
	}

	if (IS_CHANGE_BUS(BUS_AHB3))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_AHB3, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_AHB3, 1);
		time_mdelay(1);
	}

	if (IS_CHANGE_BUS(BUS_APB1))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_APB1, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB1, 1);
		time_mdelay(1);
	}

	if (IS_CHANGE_BUS(BUS_APB2))
	{
		ccu_set_mclk_src(CCU_MOD_CLK_APB2, CCU_SYS_CLK_HOSC);
		time_mdelay(1);
		ccu_set_mclk_div(CCU_MOD_CLK_APB2, 1);
		time_mdelay(1);
	}

	//C0
	if (IS_CHANGE_PLL(PLL_C0))
	{
		writel(0x02001000, CCU_PLL_C0_REG);
	}
	if (IS_EN_PLL(PLL_C0))
	{
		writel(((readl(CCU_PLL_C0_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_C0_REG);
	}

	//DRAM
	if (IS_CHANGE_PLL(PLL_DRAM))
	{
		writel(0x00002301, CCU_PLL_DDR0_REG);

	}
	if (IS_EN_PLL(PLL_DRAM))
	{
		//SDRPLL configration updata,
		//after enable PLL6, this bit(bit 30) should be set to 1 to validate PLL6.
		writel(((readl(CCU_PLL_DDR0_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_DDR0_REG);
	}

	//DRAM1
	if (IS_CHANGE_PLL(PLL_DRAM1))
	{
		writel(0x00002301, CCU_PLL_DDR1_REG);
		writel(((readl(CCU_PLL_DDR1_REG) & (~(0x1 << 30))) | (0x1 << 30)), CCU_PLL_DDR1_REG);
		while((readl(CCU_PLL_DDR1_REG) >> 30) & 0x1);
	}
	if (IS_EN_PLL(PLL_DRAM1))
	{
		//SDRPLL configration updata,
		//after enable PLL6, this bit(bit 30) should be set to 1 to validate PLL6.
		writel(((readl(CCU_PLL_DDR1_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_DDR1_REG);
		writel(((readl(CCU_PLL_DDR1_REG) & (~(0x1 << 30))) | (0x1 << 30)), CCU_PLL_DDR1_REG);
		while((readl(CCU_PLL_DDR1_REG) >> 30) & 0x1);
	}

	//PERIPH
	if (IS_CHANGE_PLL(PLL_PERIPH))
	{
		writel(0x00003100, CCU_PLL_PERIPH0_REG);
	}
	if (IS_EN_PLL(PLL_PERIPH))
	{
		writel(((readl(CCU_PLL_PERIPH0_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_PERIPH0_REG);
	}

	//PERIPH1
	if (IS_CHANGE_PLL(PLL_PERIPH1))
	{

		writel(0x00003100, CCU_PLL_PERIPH1_REG);
	}
	if (IS_EN_PLL(PLL_PERIPH1))
	{
		writel(((readl(CCU_PLL_PERIPH1_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_PERIPH1_REG);
	}

	//GPU
	if (IS_CHANGE_PLL(PLL_GPU))
	{
		writel(0x00002301, CCU_PLL_GPU0_REG);
	}
	if (IS_EN_PLL(PLL_GPU))
	{
		writel(((readl(CCU_PLL_GPU0_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_GPU0_REG);
	}

	//VIDEO0
	if (IS_CHANGE_PLL(PLL_VIDEO0))
	{
		writel(0x00006203, CCU_PLL_VIDEO0_REG);
	}
	if (IS_EN_PLL(PLL_VIDEO0))
	{
		writel(((readl(CCU_PLL_VIDEO0_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_VIDEO0_REG);
	}

	//VIDEO1
	if (IS_CHANGE_PLL(PLL_VIDEO1))
	{

		writel(0x00006203, CCU_PLL_VIDEO1_REG);
	}
	if (IS_EN_PLL(PLL_VIDEO1))
	{
		writel(((readl(CCU_PLL_VIDEO1_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_VIDEO1_REG);
	}

	//VE
	if (IS_CHANGE_PLL(PLL_VE))
	{
		writel(0x00002301, CCU_PLL_VE_REG);
	}
	if (IS_EN_PLL(PLL_VE))
	{
		writel(((readl(CCU_PLL_VE_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_VE_REG);
	}

	//DE
	if (IS_CHANGE_PLL(PLL_DE))
	{
		writel(0x00002301, CCU_PLL_DE_REG);
	}
	if (IS_EN_PLL(PLL_DE))
	{
		writel(((readl(CCU_PLL_DE_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_DE_REG);
	}

	//HSIC
	if (IS_CHANGE_PLL(PLL_HSIC))
	{
		writel(0x00002701, CCU_PLL_HSIC_REG);
	}
	if (IS_EN_PLL(PLL_HSIC))
	{
		writel(((readl(CCU_PLL_HSIC_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_HSIC_REG);
	}

	//AUDIO
	if (IS_CHANGE_PLL(PLL_AUDIO))
	{
		writel(0x00142a01, CCU_PLL_AUDIO_REG);
	}
	if (IS_EN_PLL(PLL_AUDIO))
	{
		writel(((readl(CCU_PLL_AUDIO_REG) & (~(0x1 << 31))) | (0x1 << 31)), CCU_PLL_AUDIO_REG);
	}

#ifdef FPGA_PLATFORM
	clk_restore();
#else
	if (IS_SYS_ON)
		clk_restore();
#endif

	ccu_set_mclk_src(CCU_MOD_CLK_R_SPI, spi_clk_src_bk);
}

static void gpio_suspend(void)
{
	pl_debounce_cfg_bak = readl(R_PIO_REG_BASE + (1-1)*0x20 + 0x218);
	pm_debounce_cfg_bak = readl(R_PIO_REG_BASE + (2-1)*0x20 + 0x218);
	writel(0x0, R_PIO_REG_BASE + (1-1)*0x20 + 0x218);
	writel(0x0, R_PIO_REG_BASE + (2-1)*0x20 + 0x218);

	time_mdelay(1);

	if (IS_GPIO_HOLD)
	{
		//FIXME: where are cpux pad hold reg?
		//ccu_set_poweroff_gating_state(PWRCTL_VDD_CPUX_GPIO_PAD_HOLD, CCU_POWEROFF_GATING_VALID);
	}
}

static void gpio_resume(void)
{
	writel(pl_debounce_cfg_bak, R_PIO_REG_BASE + (1-1)*0x20 + 0x218);
	writel(pm_debounce_cfg_bak, R_PIO_REG_BASE + (2-1)*0x20 + 0x218);

	if (IS_GPIO_HOLD)
	{
		//ccu_set_poweroff_gating_state(PWRCTL_VDD_CPUX_GPIO_PAD_HOLD, CCU_POWEROFF_GATING_INVALID);
	}
	time_mdelay(1);
}

static void system_suspend(void)
{
	if (IS_SYS_OFF)
	{
		ccu_set_poweroff_gating_state(PWRCTL_VDD_CPUS, CCU_POWEROFF_GATING_VALID);
		ccu_set_poweroff_gating_state(PWRCTL_VDD_AVCC_A, CCU_POWEROFF_GATING_VALID);
		//writel(0xe, 0x1F01510);
	}

	if (IS_GPU_OFF)
	{
		ccu_set_poweroff_gating_state(PWRCTL_GPU, CCU_POWEROFF_GATING_VALID);
	}


	if (IS_SYS_OFF)
	{
		ccu_set_mclk_reset(CCU_MOD_CLK_VDD_SYS, CCU_CLK_RESET);
	}
}

static void system_resume(void)
{
	if (IS_SYS_OFF)
	{
		ccu_set_poweroff_gating_state(PWRCTL_VDD_CPUS, CCU_POWEROFF_GATING_INVALID);
		ccu_set_poweroff_gating_state(PWRCTL_VDD_AVCC_A, CCU_POWEROFF_GATING_INVALID);
	}
	time_mdelay(1);

	if (IS_SYS_OFF)
	{
		ccu_set_mclk_reset(CCU_MOD_CLK_VDD_SYS, CCU_CLK_NRESET);
		time_mdelay(1);
	}

}

static s32 esstandby_process_init(struct message *request, struct super_standby_para *para, struct extended_standby *config)
{
	cpuidle_exit();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x1);
	set_wakeup_src(para);
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x2);

	/* communication suspend */
	//notify hwmsgbox and hwspinlock will enter super-standby,
	//hwmsgbox and hwspinlock locate in VDD_SYS power-domain.
	hwmsgbox_super_standby_init();
	hwspinlock_super_standby_init();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x3);

	cpucfg_cpu_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x4);
	//reg_debug(__func__, 0x01c20800, GPIO_REG_LENGTH);

	//gpio pad hold -> save gpio regs
	gpio_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x5);

	//long_jump((long_jump_fn)mem_linux_save, (void *)config);
	mem_linux_save(config);
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x6);

	dram_suspend(para);
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x7);

	cpucfg_cpu_suspend_late();

	//print_output_state();

#ifndef FPGA_PLATFORM
	ccu_set_mclk_src(CCU_MOD_CLK_CPUS, CCU_SYS_CLK_IOSC);

	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x8);

	time_cdelay(1600);
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0x9);
#endif //FPGA_PLATFORM

	clk_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0xa);

	system_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0xb);

	dm_suspend();
	save_state_flag(REC_ESTANDBY | REC_ENTER_INIT | 0xc);

	return OK;
}

static s32 esstandby_process_exit(struct message *request, struct super_standby_para *para, struct extended_standby *config, u32 wake_event)
{
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x1);
	dm_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x2);

	system_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x3);

	clk_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x4);

	cpucfg_cpu_resume_early(para->resume_entry);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x5);

	dram_resume(para);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x6);

	//save gpio regs -> gpio pad hold
	//long_jump((long_jump_fn)mem_linux_restore, (void *)config);
	mem_linux_restore(config);
	//reg_debug(__func__, 0x01c20800, GPIO_REG_LENGTH);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x7);

	gpio_resume();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x8);

	//memcpy((void *)((u32)(para->resume_entry) + 0x40000), (void *)(para->resume_code_src), para->resume_code_length);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0x9);

	/* communication resume */
	//enable msgbox clock and set reset as de-assert state.
	hwmsgbox_super_standby_exit();

	//enable spinlock clock and set reset as de-assert state.
	hwspinlock_super_standby_exit();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xa);

	//cpucfg_set_super_standby_flag();
	//writel(0x14000000, 0x40006FC8);
	cpucfg_cpu_resume(para->resume_entry);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xb);

	wait_cpu0_resume();
	//cpucfg_clear_super_standby_flag();
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xc);

	//long_jump((long_jump_fn)mem_int_resume_cfg, (void *)para);
	mem_int_resume_cfg(para);
	save_state_flag(REC_ESTANDBY | REC_ENTER_EXIT | 0xd);

	cpuidle_init();

	return OK;
}

/*
*********************************************************************************************************
*                                       ENTEY OF TALK-STANDBY
*
* Description:  the entry of extended super-standby.
*
* Arguments  :  request:request command message.
*
* Returns    :  OK if enter extended super-standby succeeded, others if failed.
*********************************************************************************************************
*/
s32 extended_super_standby_entry(struct message *request)
{
	struct super_standby_para super_standby_para;
	struct super_standby_para *para = &super_standby_para;
	u32 standby_base, standby_size;

	//RTC domain general reg, record esstandby progress
	save_state_flag(REC_ESTANDBY | REC_ENTER);//RECORD

	//get super standby para from request message
	/* base        |-------------------|
	 *             |                   |
	 *             |super_standby_para |
	 *             |                   |
	 * base+size/2 |-------------------|
	 *             |                   |
	 *             |extended_standby   |
	 *             |                   |
	 * base+size   |-------------------|
	 */
	arisc_para_get_standby_para_info(&standby_base, &standby_size);
	memcpy((void *)para, (void *)(standby_base), sizeof(super_standby_para));
	if (standby_size/2 >= sizeof(struct extended_standby)) {
		memcpy((void *)pextended_config, (void *)(standby_base + standby_size/2), sizeof(struct extended_standby));
	} else {
		ERR("extended_standby > standby_size/2\n");
		return -ENOMEM;
	}
	para->resume_entry = request->paras[1];
	save_state_flag(REC_ESTANDBY | REC_ENTER | 0x1); //RECORD:ar100 get estandby config
#ifdef FPGA_PLATFORM
	pextended_config->soc_pwr_dm_state.sys_mask = 0xca55;
#endif
	dm_on = pextended_config->soc_pwr_dm_state.sys_mask & pextended_config->soc_pwr_dm_state.state;
	dm_off = (~(pextended_config->soc_pwr_dm_state.sys_mask) | pextended_config->soc_pwr_dm_state.state) | dm_on;

	//hexdump("or para", (char *)standby_base, sizeof(struct super_standby_para));
	//hexdump("or config", (char *)(standby_base + standby_size/2), sizeof(struct extended_standby));

	//hexdump("para", (char *)para, sizeof(struct super_standby_para));
	//hexdump("config", (char *)pextended_config, sizeof(struct extended_standby));
	//hexdump("prcm", (char *)PRCM_REG_BASE, 0x1d4);
	//hexdump("cpuscfg", (char *)R_CPUCFG_REG_BASE, 0x28c);
	//hexdump("cpuxcfg", (char *)CPUCFG_REG_BASE, 0xc0);
	//hexdump("ccu", (char *)CCU_REG_BASE, 0x324);

	//printk("state:%x\n", pextended_config->soc_pwr_dm_state.state);
	//printk("mask:%x\n", pextended_config->soc_pwr_dm_state.sys_mask);
	//printk("dm_on:%x\n", dm_on);
	//printk("dm_off:%x\n", dm_off);

	//backup cpus source clock
	osc_freq_init();
	cpus_src = ccu_get_mclk_src(CCU_MOD_CLK_CPUS);


	/* wait for cpu0 enter WFI status */
	while (!((readl(0xd9010000 + 0x80) >> 16) & (0x1 << 0)));

	//--------------------------------------------------------------------------
	//
	//              initialize enter super-standby porcess
	//
	//--------------------------------------------------------------------------
	printk("%x enter\n", pextended_config->id);
	save_state_flag(REC_ESTANDBY | REC_BEFORE_INIT); //RECORD:ar100 will enter esstandby process init
	result = esstandby_process_init(request, para, pextended_config);
	save_state_flag(REC_ESTANDBY | REC_AFTER_INIT); //RECORD:ar100 finish esstandby process init
#if SUSPEND_POWER_CHECK
	if (result != OK)
	{
		//restore cpus source clock
		ccu_set_mclk_src(CCU_MOD_CLK_CPUS, cpus_src);

		//notify ac327 super-standby enter failed
		ERR("estif\n");
		request->result = result;
		request->state  = MESSAGE_PROCESSED;
		hwmsgbox_feedback_message(request, SEND_MSG_TIMEOUT);
		return -EFAIL;
	}

	/* check system power state & consumption */
	if (sst_info.power_state.enable & CPUS_ENABLE_POWER_EXP) {
		/* update system power onoff state */
		sst_info.power_state.power_reg = pmu_get_powerstate(power_chk_back.power_reg);
		/* update system power consumption */
		sst_info.power_state.system_power = pmu_get_batconsum();

		if (sst_info.power_state.power_reg != power_chk_back.power_reg) {
			/*INF("power state exception, expect:%x, real:%x\n", \
				power_chk_back.power_reg, sst_info.power_state.power_reg);*/
			if (sst_info.power_state.enable & CPUS_WAKEUP_POWER_STA) {
				gwake_event = CPUS_WAKEUP_POWER_EXP;
				goto wakeup_system;
			}
		}
		if (sst_info.power_state.system_power > power_chk_back.system_power) {
			/*INF("power consume exception, expect:%d, real:%d\n", \
				power_chk_back.system_power, sst_info.power_state.system_power);*/
			if (sst_info.power_state.enable & CPUS_WAKEUP_POWER_CSM) {
				gwake_event = CPUS_WAKEUP_POWER_EXP;
				goto wakeup_system;
			}
		}
	}
#endif
	//--------------------------------------------------------------------------
	//
	//              wait valid wakeup source porcess
	//
	//--------------------------------------------------------------------------
	//wait wakeup event coming, cpus not support WFI.
	printk("wait\n");
	//hexdump("ccu", (char *)CCU_REG_BASE, 0x300);
	//hexdump("rtc", (char *)0x01f00000, 0x100);
	save_state_flag(REC_ESTANDBY | REC_WAIT_WAKEUP); //RECORD:ar100 wait for wakeup source
	//printk("%x\n", para->event);
	get_wakeup_src(para);
	//--------------------------------------------------------------------------
	//
	//              exit super-standby wakeup porcess
	//
	//--------------------------------------------------------------------------
#if SUSPEND_POWER_CHECK
wakeup_system:
#endif
	save_state_flag(REC_ESTANDBY | REC_BEFORE_EXIT); //RECORD:ar100 will enter esstandby process exit
	//INF("%x wakeup\n", pextended_config->id);
	esstandby_process_exit(request, para, pextended_config, gwake_event);
	save_state_flag(REC_ESTANDBY | REC_AFTER_EXIT); //RECORD:ar100 finish esstandby process exit

	//restore cpus source clock
	ccu_set_mclk_src(CCU_MOD_CLK_CPUS, cpus_src);

	//INF("power_domain_record:0x%x\n", power_domain_record);

#if DEBUG_DM
	LOG("reg%x:%x\n", AW1660_OUTPUT_PWR_CTRL0, AW1660_OUTPUT_PWR_CTRL0_BAK);
	LOG("reg%x:%x\n", AW1660_OUTPUT_PWR_CTRL1, AW1660_OUTPUT_PWR_CTRL1_BAK);
	LOG("reg%x:%x\n", AW1660_ALDO123_OP_MODE, AW1660_ALDO123_OP_MODE_BAK);
#endif
	printk("%x return\n", pextended_config->id);

	return OK;
}

#endif /* EST_USED */

#if CPUOP_USED
#if 0
int cpu_op(struct message *pmessage)
{
	u32 mpidr = pmessage->paras[0];
	u32 entrypoint = pmessage->paras[1];
	u32 cpu_state = pmessage->paras[2];
	//u32 cluster_state = pmessage->paras[3];

	INF("mpidr:%x, entrypoint:%x; cpu_state:%x\n", mpidr, entrypoint, cpu_state);

	if (cpu_state == arisc_power_on) {
		set_secondary_entry(entrypoint, mpidr);
		cpu_power_up(0, mpidr);
	} else if (cpu_state == arisc_power_off) {
		if (mpidr == 0) {
			extended_super_standby_entry(pmessage);
		} else {
			cpu_power_down(0, mpidr);
		}
	}

	return 0;
}
#else
int cpu_op(struct message *pmessage)
{
	u32 mpidr = pmessage->paras[0];
	u32 entrypoint = pmessage->paras[1];
	u32 cpu_state = pmessage->paras[2];
	u32 cluster_state = pmessage->paras[3]; //unused variable
	u32 system_state = pmessage->paras[4];

	INF("mpidr:%x, entrypoint:%x; cpu_state:%x, cluster_state:%x, system_state:%x\n", mpidr, entrypoint, cpu_state, cluster_state, system_state);
	if (cpu_state == arisc_power_on) {
		set_secondary_entry(entrypoint, mpidr);
		cpu_power_up(0, mpidr);
	} else if (cpu_state == arisc_power_off) {
		if (entrypoint) {
			if (system_state == arisc_power_off) {
				extended_super_standby_entry(pmessage);
			} else if (cluster_state == arisc_power_off){
				cpuidle_enter(entrypoint, mpidr, 2);
			} else {
				cpuidle_enter(entrypoint, mpidr, 1);
			}
		} else {
			cpu_power_down(0, mpidr);
		}
	}

	return 0;
}
#endif
#endif //CPUOP_USED

#if SYSOP_USED
static void shutdown()
{
	u8 val;
	u8 devaddr = RSB_RTSADDR_AW1660;
	u8 regaddr;
	u8 temp;

	if(arisc_para.pmu_pwroff_vol >= 2600 && arisc_para.pmu_pwroff_vol <= 3300){
		if (arisc_para.pmu_pwroff_vol > 3200){
			val = 0x7;
		}else if (arisc_para.pmu_pwroff_vol > 3100){
			val = 0x6;
		}else if (arisc_para.pmu_pwroff_vol > 3000){
			val = 0x5;
		}else if (arisc_para.pmu_pwroff_vol > 2900){
			val = 0x4;
		}else if (arisc_para.pmu_pwroff_vol > 2800){
			val = 0x3;
		}else if (arisc_para.pmu_pwroff_vol > 2700){
			val = 0x2;
		}else if (arisc_para.pmu_pwroff_vol > 2600){
			val = 0x1;
		}else
			val = 0x0;

		regaddr = AW1660_PWR_WAKEUP_CTRL;
		pmu_reg_read(&devaddr, &regaddr, &temp, 1);
		temp &= ~0x7;
		temp |= val & 0x7;
		pmu_reg_write(&devaddr, &regaddr, &temp, 1);
	}
	val = 0xff;
	INF("[axp] send power-off command!\n");

	mdelay(20);

	if(arisc_para.power_start != 1) {
		/* when system is in charging, reboot system*/
		regaddr = AW1660_PWR_SRC_STA;
		pmu_reg_read(&devaddr, &regaddr, &val, 1);
		if(val & 0xF0){
			regaddr = AW1660_PWRM_CHGR_STA;
			pmu_reg_read(&devaddr, &regaddr, &val, 1);
			if(val & 0x20) {
				INF("[axp] set flag!\n");
				/* AW1660_DATA_BUFFC is 0x0d, system is in out_factory_mode*/
				regaddr = AW1660_DATA_BUFFC;
				pmu_reg_read(&devaddr, &regaddr, &val, 1);
				if (0x0d != val) {
					val = 0xf;
					regaddr = AW1660_DATA_BUFFC;
					pmu_reg_write(&devaddr, &regaddr, &val, 1);
				}
				mdelay(20);
				INF("[axp] reboot!\n");
				pmu_reset();//FIXME
				INF("[axp] warning!!! arch can't ,reboot, maybe some error happend!\n");
			}
		}
	}

	regaddr = AW1660_DATA_BUFFC;
	pmu_reg_read(&devaddr, &regaddr, &val, 1);
	if (0x0d != val) {
		val = 0x00;
		regaddr = AW1660_DATA_BUFFC;
		pmu_reg_write(&devaddr, &regaddr, &val, 1);
	}

	mdelay(20);
	pmu_shutdown();
	mdelay(20);
	INF("[axp] warning!!! axp can't power-off, maybe some error happend!\n");
}

int sys_op(struct message *pmessage)
{
	u32 state = pmessage->paras[0];

	INF("state:%x\n", state);

	switch (state)
	{
		case arisc_system_shutdown:
		{
			save_state_flag(REC_SHUTDOWN | 0x101);
			shutdown();
			break;
		}
		case arisc_system_reset:
		case arisc_system_reboot:
		{
			save_state_flag(REC_SHUTDOWN | 0x102);
			pmu_reset();
			break;
		}
		default:
		{
			WRN("invaid system power state (%d)\n", state);
			return -EINVAL;
		}
	}

	return 0;
}
#endif //CPUOP_USED

/*
*********************************************************************************************************
*                                       ENTEY OF FAKE POWER OFF
*
* Description:  the entry of FAKE POWER OFF.
*
* Arguments  :  request:request command message.
*
* Returns    :  OK if enter fake power off succeeded, others if failed.
*********************************************************************************************************
*/
#if FAKE_POWOFF_USED
s32 fake_power_off_entry(struct message *request)
{
	u32 event;

	/* power key config */
	pmu_sysconfig_cfg();

	/* ir config */
	ir_sysconfig_cfg();

	/* set system enter low power consumption */
	write_rtc_domain_reg(START_OS_REG, 0x0f);
	pmu_set_lowpcons();

	while (1) {
		if (interrupt_query_pending(EXT_NMI_IRQn)) {
			interrupt_clear_pending(EXT_NMI_IRQn);
			pmu_query_event(&event);
			if (event & (CPUS_WAKEUP_DESCEND | CPUS_WAKEUP_ASCEND | \
				CPUS_WAKEUP_SHORT_KEY | CPUS_WAKEUP_LONG_KEY)) {
				pmu_clear_pendings();
				INF("pmu powerkey wakeup detect\n");
				break;
			}
			pmu_clear_pendings();
		}

		if (interrupt_query_pending(INTC_R_CIR_IRQ)) {
			interrupt_clear_pending(INTC_R_CIR_IRQ);
			if (ir_is_power_key() == TRUE) {
				INF("cir wakeup detect\n");
				break;
			}
		}

#if 0
		//hexdump("pin", (char *)R_PIO_REG_BASE, 0x218);
		//hexdump("intc", (char *)R_INTC_REG_BASE, 0xcc);
		//printk("%x\n", readl(0x01f01400 + 0x28));
		if (interrupt_query_pending(R_GPIOL_IRQn)) {
			interrupt_clear_pending(R_GPIOL_IRQn);
			INF("pl wakeup detect\n");
			save_state_flag(REC_CPUS_WAKEUP | (readl(R_PIO_REG_BASE + 0x214) & 0xfff));
			break;
		}
#endif
		if (pmu_pin_detect() == TRUE) {
			INF("pl wakeup detect\n");
			break;
		}
	}

	printk("reset system now\n");
	/* enable rtc gating, after that operation, cpus will can't access rtc registers */
	writel((readl(0x01f01400 + 0x2c) & (~0x1)), 0x01f01400 + 0x2c);
	pmu_reset_system();
	while(1);

	return 0;
}
#endif // FAKE_POWOFF_USED
#endif // sun50iw3p1
