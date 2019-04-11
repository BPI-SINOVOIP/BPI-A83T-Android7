/*
 * drivers/led/led.c
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: xiafeng <xiafeng@allwinnertech.com>
 *
 */
#include <plat/inc/include.h>

#ifdef BOOT_USED
static volatile unsigned int led_power = 0;
static volatile unsigned int led_state = 0;

void led_sysconfig_cfg(void)
{
	int val;
	int status;

	status = script_parser_fetch("box_start_os", "led_power", &val, 1);
	if(SCRIPT_PARSER_OK != status)
		return;
	led_power = val;
	LOG("power_led:%d\n", led_power);
	/* boad p1 conneted led_power to 3.3V, which is pmu DCDC1,
	 * so config at power_set_sys_lowcons
	 */

	status = script_parser_fetch("box_start_os", "led_state", &val, 1);
	if(SCRIPT_PARSER_OK != status)
		return;
	led_state = val;
	LOG("led_state:%d\n", led_state);
	/* controlled by pmu giio1/ldo */
	pmu_set_gpio(0, led_state);

	LOG("led set ok\n");
}

unsigned int led_get_powercfg(void)
{
	return led_power;
}

unsigned int led_get_statecfg(void)
{
	return led_state;
}
#endif

#if LED_BLN_USED
#define R_PWM_CTRL_REG  R_PWM_REG_BASE + 0x0
#define R_PWM_CH_PERIOD R_PWM_REG_BASE + 0x4

#define PWM_CH_PERSCAL 0x0 //120
#define PWM_CH_ENTIRE_CYS 0x9e5
#define SCAL_DELAY 10  //ms

static u32 led_stop = 1;
static u32 led_suspend = 0;
static u32 led_rgb = 0;
static u32 led_onms = 1000;
static u32 led_offms = 1000;
static u32 led_dark = 100;
static bool led_trend = 1;
static u32 dark_flg = 0;
static s32 x = 0; /* Unit:10ms */

static void led_bln_on(void)
{
	u32 value;

	led_trend = 1;
	x = 0;

	pin_write_data(PIN_GRP_PL, 10, 1);
	value = (PWM_CH_ENTIRE_CYS << 16) | 0;
	writel(value, R_PWM_CH_PERIOD);
	LOG("bln on\n");
}

static void led_bln_close(void)
{
	u32 value;

	led_trend = 1;
	x = 0;

	pin_write_data(PIN_GRP_PL, 10, 0);
	value = (PWM_CH_ENTIRE_CYS << 16) | PWM_CH_ENTIRE_CYS;
	writel(value, R_PWM_CH_PERIOD);
	LOG("bln off\n");
}

static void led_blnpwm_init(void)
{
	volatile u32 value;

	/* init pin, select as s_pwm */
	pin_set_multi_sel(PIN_GRP_PL, 10, 2);

	/* init ctrl */
	value = readl(R_PWM_CTRL_REG);
	value &= ~(0x1 << 9); /* disable pwm bypass */
	value &= ~(0x1 << 7); /* set pwm mode as cycle mode */
	value &= ~(0x1 << 5); /* set pwm active state as low level */
	value &= ~(0xf << 0); /* mask prescale */
	value |= ((PWM_CH_PERSCAL & 0xf) << 0); /* set perscale */
	value |= (0x1 << 6); /* pass special clock gating */
	writel(value, R_PWM_CTRL_REG);

	/* init period */
	value = (PWM_CH_ENTIRE_CYS << 16) | PWM_CH_ENTIRE_CYS;
	writel(value, R_PWM_CH_PERIOD);

	/* enable pwm output */
	value = readl(R_PWM_CTRL_REG);
	value |=  (0x1 << 4); /* enable pwm output */
	writel(value, R_PWM_CTRL_REG);
}

int led_bln_cfg(unsigned int rgb, unsigned int onms,  unsigned int offms, \
                unsigned int darkms)
{
	led_rgb = rgb;
	led_onms = onms;
	led_offms = offms;
	led_dark = darkms/10;
	dark_flg = 0;
	led_trend = 1;
	x = 0;

	LOG("led cfg:%x %x %x %x\n", led_rgb, led_onms, led_offms, led_dark);

	if (!led_onms) {
		led_stop = 1;
		led_bln_close();
	} else if (!led_offms) {
		led_stop = 1;
		led_bln_on();
	} else
		led_stop = 0;
	led_blnpwm_init();

	return OK;
}

s32 led_bln_adjust(void *arg)
{
	u32 value;
	s32 y = 0;
	volatile u32 i;

	if (led_stop)
		return OK;

	if (dark_flg) {
		if(dark_flg++ > led_dark)
			dark_flg = 0;
		else {
			if (led_suspend) {
				for (i = 0; i < (101 - y) * 128; i++)
					asm volatile ("nop");
			}
			return OK;
		}
	}

	if (led_trend) {
		x++;
		y = 1000 * x / led_onms;
	} else {
		x--;
		y = 1000 * x / led_offms;
	}
	if (y < 1) {
		led_trend = 1;
		y = 0;
		dark_flg = 1;
	}
	if (y > 99) {
		led_trend = 0;
		y = 99;
		x = led_offms/10;
	}

	if (led_suspend) {
		pin_write_data(PIN_GRP_PL, 10, 1);
		for (i = 0; i < y * 64; i++)
			asm volatile ("nop");
		pin_write_data(PIN_GRP_PL, 10, 0);
		for (i = 0; i < (101 - y) * 64; i++)
			asm volatile ("nop");
	} else {
		value = (PWM_CH_ENTIRE_CYS << 16) | (PWM_CH_ENTIRE_CYS * (101 - y) / 100);
		writel(value, R_PWM_CH_PERIOD);
	}

	return OK;
}

static struct softtimer timer;

int led_bln_init(void)
{
	timer.cycle = mecs_to_ticks(SCAL_DELAY);
	timer.expires = 0;
	timer.cb = led_bln_adjust;
	timer.arg = 0;
	add_softtimer(&timer);

	LOG("led bln init ok\n");

	return OK;
}

int led_bln_suspend(void *arg)
{
	led_suspend = 1;

	if (led_stop)
		return OK;

	pin_set_multi_sel(PIN_GRP_PL, 10, 1);

	return OK;
}

int led_bln_resume(void *arg)
{
	led_suspend = 0;

	if (led_stop)
		return OK;

	pin_set_multi_sel(PIN_GRP_PL, 10, 2);

	return OK;
}
#endif
