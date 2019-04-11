/*
 * arch/arm/mach-sunxi/include/mach/sun8i/platform-sun8iw5p1.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sun8i platform header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __PLATFORM_SUN50I_W3P1_H
#define __PLATFORM_SUN50I_W3P1_H

/*
 *  device physical addresses
 */

#define SUNXI_TIMESTAMP_CTRL_PBASE       0xD8120000
#define SUNXI_TIMESTAMP_CTRL_SIZE        0x00001000
#define SUNXI_TIMESTAMP_STATE_PBASE      0xD8110000
#define SUNXI_TIMESTAMP_STATE_SIZE       0x00001000

#define SUNXI_SYSCFG_PBASE               0xD3000000

#define SUNXI_SMC_PBASE                  0xD3007000

#define SUNXI_CCM_PBASE                  0xD3001000
#define SUNXI_CCM_PEND                   0x03001ff0

#define SUNXI_PIO_PBASE                  0xD300b000

#define SUNXI_TIMER_PBASE                0xD3009000
#define SUNXI_SPC_PBASE                  0xD3008000

/*
 * define virt addresses
 */
#define SUNXI_SYSCFG_VBASE               IO_ADDRESS(SUNXI_SYSCFG_PBASE     )
#define SUNXI_CCM_VBASE                  IO_ADDRESS(SUNXI_CCM_PBASE          )
#define SUNXI_PIO_VBASE                  IO_ADDRESS(SUNXI_PIO_PBASE          )
#define SUNXI_TIMER_VBASE                IO_ADDRESS(SUNXI_TIMER_PBASE        )
#endif    /* __PLATFORM_SUN50I_W2P1_H */
