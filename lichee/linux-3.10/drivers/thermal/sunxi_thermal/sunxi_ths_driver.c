/*
 * drivers/thermal/sunxi_thermal/sunxi_ths_driver.c
 *
 * Copyright (C) 2013-2024 allwinner.
 *	JiaRui Xiao<xiaojiarui@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#define NEED_DEBUG (0)

#if NEED_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <linux/thermal.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/sunxi-sid.h>
#include "sunxi_ths.h"
#include "sunxi_ths_core.h"
#include "sunxi_ths_driver.h"

static struct sunxi_ths_controller *main_ctrl;

/**
 *Init the thermal sensor and show them value in screen
 */
static void sunxi_ths_reg_init(struct sunxi_ths_data *ths_data)
{
	ths_driver_init_reg(ths_data, ths_data->ths_coefficent);
	ths_driver_reg_debug(ths_data, ths_data->ths_coefficent);
	return;
}

static void sunxi_ths_exit(struct sunxi_ths_data *ths_data)
{
	ths_driver_disable_reg(ths_data);
	return;
}

static int sunxi_ths_get_temp(struct sunxi_ths_controller *controller, u32 id,
			     long *temp)
{
	long t = 0;
	struct sunxi_ths_data *ths_data =
		(struct sunxi_ths_data *)controller->data;
	struct thermal_sensor_info *sensor_info =
		(struct thermal_sensor_info *)ths_data->data;

	if (ths_data->sensor_cnt <= id) {
		thsprintk("ths driver get wrong sensor num!\n");
		return -1;
	}

	t = ths_driver_get_temp(ths_data, id);

	if (-40 > t || 180 < t) {
		thsprintk("ths driver get unvalid temp!\n");
		return -1;
	}


	(sensor_info + id)->temp = t;
	*temp = t;
	return 0;
}

static void sunxi_ths_info_init(struct thermal_sensor_info *sensor_info,
				int sensor_num)
{
	sensor_num -= 1;

	while (sensor_num >= 0) {
		sensor_info[sensor_num].id = sensor_num;
		sensor_info[sensor_num].ths_name = id_name_mapping[sensor_num];
		sensor_info[sensor_num].temp = 0;
		thsprintk("sensor_info:id=%d,name=%s,temp=%ld!\n",
			sensor_info[sensor_num].id,
			sensor_info[sensor_num].ths_name,
			sensor_info[sensor_num].temp);
		sensor_num--;
	}
	return;
}

static void sunxi_ths_coefficent_init(struct thermal_sensor_coefficent *ths_coe)
{
	ths_coe->calcular_para = thermal_cal_coefficent;
	ths_coe->reg_para = thermal_reg_init;
	return;
}

static void sunxi_ths_para_init(struct sunxi_ths_data *ths_data,
				struct thermal_sensor_info *sensor_info)
{
	sunxi_ths_info_init(sensor_info, ths_data->sensor_cnt);
	sunxi_ths_coefficent_init(ths_data->ths_coefficent);
	ths_data->data = sensor_info;
	return;
}

int sunxi_get_sensor_temp(u32 sensor_num, long *temperature)
{
	return sunxi_ths_get_temp(main_ctrl, sensor_num, temperature);
}
EXPORT_SYMBOL(sunxi_get_sensor_temp);

static int sunxi_ths_suspend(struct sunxi_ths_controller *controller)
{
	struct sunxi_ths_data *ths_data =
	    (struct sunxi_ths_data *)controller->data;

	thsprintk("enter: sunxi_ths_controller_suspend.\n");
	atomic_set(&controller->is_suspend, 1);
	sunxi_ths_exit(ths_data);
	if (ths_data->parent_clk == true)
		clk_disable_unprepare(ths_data->mclk);
	return 0;
}

static int sunxi_ths_resume(struct sunxi_ths_controller *controller)
{
	struct sunxi_ths_data *ths_data =
	    (struct sunxi_ths_data *)controller->data;

	thsprintk("enter: sunxi_ths_controller_resume.\n");
	if (ths_data->parent_clk == true)
		clk_prepare_enable(ths_data->mclk);
	sunxi_ths_reg_init(ths_data);
	atomic_set(&controller->is_suspend, 0);
	return 0;
}

struct sunxi_ths_controller_ops sunxi_ths_ops = {
	.suspend = sunxi_ths_suspend,
	.resume = sunxi_ths_resume,
	.get_temp = sunxi_ths_get_temp,
};

static int sunxi_ths_probe(struct platform_device *pdev)
{
	int err = 0;
	struct sunxi_ths_data *ths_data;
	struct sunxi_ths_controller *ctrl;
	struct thermal_sensor_info *sensor_info;

	thsprintk("sunxi ths sensor probe start !\n");
	if (!pdev->dev.of_node) {
		pr_err("%s:sunxi ths device tree err!\n", __func__);
		return -EBUSY;
	}

	ths_data = kzalloc(sizeof(*ths_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(ths_data)) {
		pr_err("ths_data: not enough memory for ths_data\n");
		err = -ENOMEM;
		goto fail0;
	}

	ths_data->ths_coefficent =
		kzalloc(sizeof(*ths_data->ths_coefficent), GFP_KERNEL);
	if (IS_ERR_OR_NULL(ths_data->ths_coefficent)) {
		pr_err("ths_coe: not enough memory for ths_coe\n");
		err = -ENOMEM;
		goto fail1;
	}

	ths_data->parent_clk = ENABLE_CLK;
	ths_data->ths_driver_version = THERMAL_VERSION;
	ths_data->pdev = pdev;

	err = ths_driver_startup(ths_data, &pdev->dev);
	if (err)
		goto fail2;

	sensor_info =
		kcalloc(ths_data->sensor_cnt, sizeof(*sensor_info), GFP_KERNEL);
	if (IS_ERR_OR_NULL(sensor_info)) {
		pr_err("sensor_info: not enough memory for sensor_info\n");
		err = -ENOMEM;
		goto fail2;
	}

	sunxi_ths_para_init(ths_data, sensor_info);
	platform_set_drvdata(pdev, ths_data);
	ths_driver_clk_cfg(ths_data);
	sunxi_ths_reg_init(ths_data);
	ths_driver_create_sensor_info_attrs(ths_data, sensor_info);

	ctrl = sunxi_ths_controller_register(&pdev->dev,
					     &sunxi_ths_ops, ths_data);
	if (!ctrl) {
		pr_err("ths_data: thermal controller register err.\n");
		err = -ENOMEM;
		goto fail3;
	}
	ths_data->ctrl = ctrl;
	main_ctrl = ctrl;
	return 0;
fail3:
	kfree(sensor_info);
	sensor_info = NULL;
fail2:
	kfree(ths_data->ths_coefficent);
	ths_data->ths_coefficent = NULL;
fail1:
	kfree(ths_data);
	ths_data = NULL;
fail0:
	return err;
}

static int sunxi_ths_remove(struct platform_device *pdev)
{
	struct sunxi_ths_data *ths_data = platform_get_drvdata(pdev);
	sunxi_ths_controller_unregister(ths_data->ctrl);
	sunxi_ths_exit(ths_data);
	ths_driver_clk_uncfg(ths_data);
	ths_drvier_remove_trip_attrs(ths_data);
	kfree(ths_data->ths_coefficent);
	kfree(ths_data);
	return 0;
}

#ifdef CONFIG_OF
/* Translate OpenFirmware node properties into platform_data */
static struct of_device_id sunxi_ths_of_match[] = {
	{.compatible = "allwinner,thermal_sensor",},
	{},
};

MODULE_DEVICE_TABLE(of, sunxi_ths_of_match);
#endif

static struct platform_driver sunxi_ths_driver = {
	.probe = sunxi_ths_probe,
	.remove = sunxi_ths_remove,
	.driver = {
		   .name = SUNXI_THS_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(sunxi_ths_of_match),
		   },
};
static int __init sunxi_thermal_sensor_init(void)
{
	return platform_driver_register(&sunxi_ths_driver);
}

static void __exit sunxi_thermal_sensor_exit(void)
{
	platform_driver_unregister(&sunxi_ths_driver);
}

subsys_initcall_sync(sunxi_thermal_sensor_init);
module_exit(sunxi_thermal_sensor_exit);

MODULE_DESCRIPTION("SUNXI thermal sensor driver");
MODULE_AUTHOR("JRXiao");
MODULE_LICENSE("GPL v2");
