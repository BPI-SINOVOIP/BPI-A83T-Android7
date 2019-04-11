/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 pin module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : pin_regs.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-8
* Descript: pin managment module register defines.
* Update  : date                auther      ver     notes
*           2012-5-8 14:22:39   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PIN_REGS_H__
#define __PIN_REGS_H__

#include <plat/inc/include.h>

#define PL_NUM  20
#define PM_NUM  6

//pin controller register list
#define PINC_REGS_BASE              (R_PIO_REG_BASE)
#define PIN_REG_CFG(n, i)           ((volatile u32 *)(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>3)*0x4 + 0x00))
#define PIN_REG_DLEVEL(n, i)        ((volatile u32 *)(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>4)*0x4 + 0x14))
#define PIN_REG_PULL(n, i)          ((volatile u32 *)(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>4)*0x4 + 0x1C))
#define PIN_REG_DATA(n)             ((volatile u32 *)(PINC_REGS_BASE + ((n)-1)*0x24 + 0x10))

#define PIN_REG_CFG_VALUE(n, i)     readl(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>3)*0x4 + 0x00)
#define PIN_REG_DLEVEL_VALUE(n, i)  readl(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>4)*0x4 + 0x14)
#define PIN_REG_PULL_VALUE(n, i)    readl(PINC_REGS_BASE + ((n)-1)*0x24 + ((i)>>4)*0x4 + 0x1C)
#define PIN_REG_DATA_VALUE(n)       readl(PINC_REGS_BASE + ((n)-1)*0x24 + 0x10)

#define PIN_REG_INT_CFG(n, i)       ((volatile u32 *)(PINC_REGS_BASE + (n-1)*0x20 + ((i)>>3)*0x4 + 0x200))
#define PIN_REG_INT_CTL(n)          ((volatile u32 *)(PINC_REGS_BASE + (n-1)*0x20 + 0x210))
#define PIN_REG_INT_STAT(n)         ((volatile u32 *)(PINC_REGS_BASE + (n-1)*0x20 + 0x214))
#define PIN_REG_INT_DEBOUNCE(n)     ((volatile u32 *)(PINC_REGS_BASE + (n-1)*0x20 + 0x218))

#endif  //__PIN_REGS_H__
