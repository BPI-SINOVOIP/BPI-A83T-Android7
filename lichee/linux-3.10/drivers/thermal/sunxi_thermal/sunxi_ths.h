/*
 *
 * Copyright(c) 2014-2016 Allwinnertech Co., Ltd.
 *         http://www.allwinnertech.com
 *
 * Author: JiaRui Xiao <xiaojiarui@allwinnertech.com>
 *
 * allwinner sunxi thermal sensor driver data struct.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SUNXI_THS_H
#define SUNXI_THS_H

#define MAX_CHN (4)
#define THERMAL_DATA_DELAY	(5000)

#define SUNXI_THS_NAME "sunxi_ths_controller"
#define SUNXI_THS_COMBINE_NAME "sunxi_ths_combine"

struct sunxi_ths_controller;

enum combine_ths_temp_type{
	COMBINE_MAX_TEMP = 0,
	COMBINE_AVG_TMP,
	COMBINE_MIN_TMP,
};

struct sunxi_ths_controller_ops {
	int (*suspend)(struct sunxi_ths_controller *);
	int (*resume)(struct sunxi_ths_controller *);
	int (*get_temp)(struct sunxi_ths_controller *,u32 id, long *temp);
};

struct sunxi_ths_controller {
	struct device *dev;
	struct sunxi_ths_controller_ops *ops;
	atomic_t initialize;
	atomic_t usage;
	atomic_t is_suspend;
	struct mutex lock;
	struct list_head combine_list;
	struct list_head node;
	int combine_num;
	void *data;
};

struct sunxi_ths_combine_disc {
	u32  combine_ths_count;
	enum combine_ths_temp_type type;
	const char *combine_ths_type;
	u32 combine_ths_id[MAX_CHN];
	struct sunxi_ths_controller *controller;
};

struct sunxi_ths_sensor{
	struct platform_device *pdev;
	u32 sensor_id;
	long last_temp;
	struct sunxi_ths_combine_disc *combine;
	struct thermal_zone_device *tz;
	atomic_t is_suspend;
	struct list_head node;
};

struct sunxi_ths_controller *
sunxi_ths_controller_register(struct device *dev ,struct sunxi_ths_controller_ops *ops, void *data);
void sunxi_ths_controller_unregister(struct sunxi_ths_controller *controller);

#endif /* SUNXI_THS_H */
