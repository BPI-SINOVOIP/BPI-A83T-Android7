/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 p2wi module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : p2wi_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: p2wi internal header.
* Update  : date                auther      ver     notes
*           2012-5-22 9:46:18   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __P2WI_I_H__
#define __P2WI_I_H__

#include <plat/inc/include.h>

typedef enum arisc_p2wi_bits_ops {
	P2WI_CLR_BITS,
	P2WI_SET_BITS
} arisc_p2wi_bits_ops_e;

#define P2WI_SCK                (3000000)
#define P2WI_SDAODLY            (1)

//register define
#define P2WI_REG_BASE           (R_P2WI_REG_BASE)
#define P2WI_REG_CTRL           (P2WI_REG_BASE + 0x00)
#define P2WI_REG_CCR            (P2WI_REG_BASE + 0x04)
#define P2WI_REG_INTE           (P2WI_REG_BASE + 0x08)
#define P2WI_REG_STAT           (P2WI_REG_BASE + 0x0c)
#define P2WI_REG_DADDR0         (P2WI_REG_BASE + 0x10)
#define P2WI_REG_DADDR1         (P2WI_REG_BASE + 0x14)
#define P2WI_REG_DLEN           (P2WI_REG_BASE + 0x18)
#define P2WI_REG_DATA0          (P2WI_REG_BASE + 0x1c)
#define P2WI_REG_DATA1          (P2WI_REG_BASE + 0x20)
#define P2WI_REG_LCR            (P2WI_REG_BASE + 0x24)
#define P2WI_REG_PMCR           (P2WI_REG_BASE + 0x28)

//p2wi control bit field
#define P2WI_SOFT_RST       (1U << 0)
#define P2WI_GLB_INTEN      (1U << 1)
#define P2WI_ABT_TRANS      (1U << 6)
#define P2WI_START_TRANS    (1U << 7)

//p2wi state bit field
#define P2WI_TOVER_INT      (1U << 0)
#define P2WI_TERR_INT       (1U << 1)
#define P2WI_LBSY_INT       (1U << 2)

//p2wi or twi pmu mode bit field
#define P2WI_PMU_INIT       (1U << 31)
#define NTWI_PMU_INIT       (0U << 31)

//local functions
s32  p2wi_wait_state(void);
void p2wi_set_pmu_mode(u32 slave_addr, u32 reg, u32 data);
void p2wi_set_clk(u32 sck);
s32  p2wi_clkchangecb(u32 command, u32 freq);

#endif  //__P2WI_I_H__
