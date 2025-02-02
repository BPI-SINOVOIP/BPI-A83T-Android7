/*
 * Allwinner sun8iw5p1 SoCs R_PIO pinctrl driver.
 *
 * Copyright(c) 2016-2020 Allwinnertech Co., Ltd.
 * Author: Weijianluo <luoweijian@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-sunxi.h"

static const struct sunxi_desc_pin sun8iw5p1_r_pins[] = {
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 0),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_rsb0"),		/*SCK*/
		SUNXI_FUNCTION(0x3, "s_twi0"),		/*SCK*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 0)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 1),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_rsb0"),		/*SDA*/
		SUNXI_FUNCTION(0x3, "s_twi0"),		/*SDA*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 1)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 2),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_uart0"),		/*TX*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 2)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 3),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_uart0"),		/*RX*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 3)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 4),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_jtag0"),		/*MS*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 4)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 5),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_jtag0"),		/*CK*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 5)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 6),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_jtag0"),		/*DO*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 6)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 7),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_jtag0"),		/*DI*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 7)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 8),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_twi0"),		/*SCK*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 8)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 9),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_twi0"),		/*SDA*/
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 9)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 10),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x2, "s_pwm0"),
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 10)),
	SUNXI_PIN(SUNXI_PINCTRL_PIN(L, 11),
		SUNXI_FUNCTION(0x0, "gpio_in"),
		SUNXI_FUNCTION(0x1, "gpio_out"),
		SUNXI_FUNCTION(0x7, "io_disabled"),
		SUNXI_FUNCTION_IRQ_BANK(0x4, 0, 11)),
};

static const struct sunxi_pinctrl_desc sun8iw5p1_r_pinctrl_data = {
	.pins = sun8iw5p1_r_pins,
	.npins = ARRAY_SIZE(sun8iw5p1_r_pins),
	.pin_base = PL_BASE,
	.irq_banks = 1,
};

static int sun8iw5p1_r_pinctrl_probe(struct platform_device *pdev)
{
	return sunxi_pinctrl_init(pdev, &sun8iw5p1_r_pinctrl_data);
}

static struct of_device_id sun8iw5p1_r_pinctrl_match[] = {
	{ .compatible = "allwinner,sun8iw5p1-r-pinctrl", },
	{}
};
MODULE_DEVICE_TABLE(of, sun8iw5p1_r_pinctrl_match);

static struct platform_driver sun8iw5p1_r_pinctrl_driver = {
	.probe	= sun8iw5p1_r_pinctrl_probe,
	.driver	= {
		.name		= "sun8iw5p1-r-pinctrl",
		.owner		= THIS_MODULE,
		.of_match_table	= sun8iw5p1_r_pinctrl_match,
	},
};

static int __init sun8iw5p1_r_pio_init(void)
{
	int ret;
	ret = platform_driver_register(&sun8iw5p1_r_pinctrl_driver);
	if (IS_ERR_VALUE(ret)) {
		pr_debug("register sun8iw5p1 r-pio controller failed\n");
		return -EINVAL;
	}
	return 0;
}
postcore_initcall(sun8iw5p1_r_pio_init);

MODULE_AUTHOR("Weijianluo<luoweijian@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner sun8iw5p1 R_PIO pinctrl driver");
MODULE_LICENSE("GPL");
