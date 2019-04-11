/*
 * arch/arm/mach-sunxi/include/mach/hardware.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi hardware header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUNXI_HARDWARE_H
#define __SUNXI_HARDWARE_H

#define IOMEM(x) (x)

#ifndef IO_ADDRESS
#define IO_ADDRESS(x)  IOMEM((x) + 0)
#endif

#endif /* __SUNXI_HARDWARE_H */
