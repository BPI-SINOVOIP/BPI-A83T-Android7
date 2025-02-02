/*
 * ac100-core.c  --  Device access for allwinnertech ac100
 *
 * Author:
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/mfd/ac100-mfd.h>
#include <linux/arisc/arisc.h>
#ifdef CONFIG_ARCH_SUN8IW7
	static unsigned int twi_id = 2;
#else
#define AUDIO_RSB_BUS
static unsigned int twi_id;
#endif
#define SUNXI_CHIP_NAME	"AC100-CHIP"
struct regmap_config ac100_base_regmap_config = {
	.reg_bits = 8,
	.val_bits = 16,
};


/**
 * ac100_rsb_reg_read: Read a single AC100 register.
 *
 * @reg: Register to read.
 */
int ac100_rsb_reg_read(unsigned short reg)
{
	int	ret;
	arisc_rsb_block_cfg_t rsb_data;
	unsigned char addr;
	unsigned int data;

	addr = (unsigned char)reg;
	rsb_data.len = 1;
	rsb_data.datatype = RSB_DATA_TYPE_HWORD;
	rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	rsb_data.devaddr = RSB_RTSADDR_AC100;
	rsb_data.regaddr = &addr;
	rsb_data.data = &data;

	/* read axp registers */
	ret = arisc_rsb_read_block_data(&rsb_data);
	if (ret != 0) {
		pr_err("failed reads to 0x%02x\n", reg);
		return ret;
	}
	return data;
}
EXPORT_SYMBOL_GPL(ac100_rsb_reg_read);

/**
 * ac100_rsb_reg_write: Write a single AC100 register.
 *
 * @reg: Register to write to.
 * @val: Value to write.
 */
int ac100_rsb_reg_write(unsigned short reg, unsigned short value)
{
	int	ret;
	arisc_rsb_block_cfg_t rsb_data;
	unsigned char addr;
	unsigned int data;

	addr = (unsigned char)reg;
	data = value;
	rsb_data.len = 1;
	rsb_data.datatype = RSB_DATA_TYPE_HWORD;
	rsb_data.msgattr = ARISC_MESSAGE_ATTR_SOFTSYN;
	rsb_data.devaddr = RSB_RTSADDR_AC100;
	rsb_data.regaddr = &addr;
	rsb_data.data = &data;

	/* read axp registers */
	ret = arisc_rsb_write_block_data(&rsb_data);
	if (ret != 0) {
		pr_err("failed reads to 0x%02x\n", reg);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(ac100_rsb_reg_write);

/**
 * ac100_reg_read: Read a single AC100 register.
 *
 * @ac100: Device to read from.
 * @reg: Register to read.
 */
int ac100_reg_read(struct ac100 *ac100, unsigned short reg)
{
#ifdef AUDIO_RSB_BUS
	return ac100_rsb_reg_read(reg);
#else
	unsigned int val;
	int ret;
	ret = regmap_read(ac100->regmap, reg, &val);

	if (ret < 0)
		return ret;
	else
		return val;
#endif
}
EXPORT_SYMBOL_GPL(ac100_reg_read);

/**
 * ac100_reg_write: Write a single AC100 register.
 *
 * @ac100: Device to write to.
 * @reg: Register to write to.
 * @val: Value to write.
 */
int ac100_reg_write(struct ac100 *ac100, unsigned short reg,
		     unsigned short val)
{
	#ifdef AUDIO_RSB_BUS
	return ac100_rsb_reg_write(reg, val);
	#else
	return regmap_write(ac100->regmap, reg, val);
	#endif
}
EXPORT_SYMBOL_GPL(ac100_reg_write);

static struct mfd_cell ac100_devs[] = {
	{
		.name = "ac100",
	},
	{
		.name = "rtc0",
	},
};

/*
 * Instantiate the generic non-control parts of the device.
 */
static int ac100_device_init(struct ac100 *ac100, int irq)
{
	int ret;

	dev_set_drvdata(ac100->dev, ac100);

	ret = mfd_add_devices(ac100->dev, -1,
			      ac100_devs, ARRAY_SIZE(ac100_devs),
			      NULL, 0, NULL);
	if (ret != 0) {
		dev_err(ac100->dev, "Failed to add children: %d\n", ret);
		goto err;
	}
	pr_debug("%s,line:%d\n", __func__, __LINE__);
	return 0;

err:
	mfd_remove_devices(ac100->dev);
	return ret;
}

static void ac100_device_exit(struct ac100 *ac100)
{
	mfd_remove_devices(ac100->dev);
}

static int ac100_i2c_probe(struct i2c_client *i2c,
				      const struct i2c_device_id *id)
{
	struct ac100 *ac100;
#ifndef AUDIO_RSB_BUS
	int ret = 0;
#endif

	pr_debug("%s, line:%d, i2c->irq:%d\n", __func__, __LINE__, i2c->irq);
	ac100 = devm_kzalloc(&i2c->dev, sizeof(struct ac100), GFP_KERNEL);
	if (ac100 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, ac100);
	ac100->dev = &i2c->dev;
	ac100->irq = i2c->irq;
#ifdef AUDIO_RSB_BUS
	if (arisc_rsb_set_rtsaddr(RSB_DEVICE_SADDR7, RSB_RTSADDR_AC100))
		pr_err("AUDIO config codec failed\n");
#else
	ac100->regmap = devm_regmap_init_i2c(i2c, &ac100_base_regmap_config);
	if (IS_ERR(ac100->regmap)) {
		ret = PTR_ERR(ac100->regmap);
		dev_err(ac100->dev, "Failed to allocate register map: %d\n",
			ret);
		return ret;
	}
#endif
	return ac100_device_init(ac100, i2c->irq);
}

static int ac100_i2c_remove(struct i2c_client *i2c)
{
	struct ac100 *ac100 = i2c_get_clientdata(i2c);

	ac100_device_exit(ac100);

	return 0;
}

static int ac100_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (twi_id == adapter->nr) {
		strlcpy(info->type, SUNXI_CHIP_NAME, I2C_NAME_SIZE);
		return 0;
	} else {
		return -ENODEV;
	}
}

static const unsigned short normal_i2c[] = {0x1a, I2C_CLIENT_END};

static const struct i2c_device_id ac100_id[] = {
	{"AC100-CHIP", 0},
	{}
};

static const struct of_device_id sunxi_ac100_match[] = {
	{ .compatible = "allwinner,sunxi-ac100", },
	{},
};

static struct i2c_driver ac100_i2c_driver = {
	.class		= I2C_CLASS_HWMON,
	.id_table	= ac100_id,
	.probe		= ac100_i2c_probe,
	.remove		= ac100_i2c_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "AC100-CHIP",
		.of_match_table = sunxi_ac100_match,
	},
	.address_list = normal_i2c,
};

static int __init ac100_i2c_init(void)
{
	int ret = 0;

	ac100_i2c_driver.detect = ac100_detect;
	ret = i2c_add_driver(&ac100_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register ac100 I2C driver: %d\n", ret);

	return ret;
}
/*subsys_initcall(ac100_i2c_init);*/
module_init(ac100_i2c_init);

static void __exit ac100_i2c_exit(void)
{
	i2c_del_driver(&ac100_i2c_driver);
}
module_exit(ac100_i2c_exit);

MODULE_DESCRIPTION("Core support for the AC100 audio CODEC");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangxin<huangxin@allwinnertech.com>");
