/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                 rsb module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : rsb_i.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-22
* Descript: rsb internal header.
* Update  : date                auther      ver     notes
*           2012-5-22 9:46:18   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __RSB_I_H__
#define __RSB_I_H__

#include <plat/inc/include.h>

/* rsb transfer data type */
typedef enum arisc_rsb_bits_ops {
	RSB_CLR_BITS,
	RSB_SET_BITS
} arisc_rsb_bits_ops_e;

#define RSB_SCK                 (3000000)
#define RSB_SDAODLY             (1)

/* RSB SHIFT */
#define RSB_RTSADDR_SHIFT       (16)//runtime slave address shift
#define RSB_SADDR_SHIFT         (0)//Slave Address shift

/* RSB command */
#define RSB_CMD_BYTE_WRITE      (0x4E)//(0x27)//Byte write
#define RSB_CMD_HWORD_WRITE     (0x59)//(0x2c)//Half word write
#define RSB_CMD_WORD_WRITE      (0x63)//(0x31)//Word write
#define RSB_CMD_BYTE_READ       (0x8B)//(0x45)//Byte read
#define RSB_CMD_HWORD_READ      (0x9C)//(0x4e)//Half word read
#define RSB_CMD_WORD_READ       (0xA6)//(0x53)//Word read
#define RSB_CMD_SET_RTSADDR     (0xE8)//(0x74)//Set Run-time Address

//register define
#define RSB_REG_BASE            (R_RSB_REG_BASE)
#define RSB_REG_CTRL            (RSB_REG_BASE + 0x00)
#define RSB_REG_CCR             (RSB_REG_BASE + 0x04)
#define RSB_REG_INTE            (RSB_REG_BASE + 0x08)
#define RSB_REG_STAT            (RSB_REG_BASE + 0x0c)
#define RSB_REG_DADDR0          (RSB_REG_BASE + 0x10)
#define RSB_REG_DADDR1          (RSB_REG_BASE + 0x14)
#define RSB_REG_DLEN            (RSB_REG_BASE + 0x18)
#define RSB_REG_DATA0           (RSB_REG_BASE + 0x1c)
#define RSB_REG_DATA1           (RSB_REG_BASE + 0x20)
#define RSB_REG_LCR             (RSB_REG_BASE + 0x24)
#define RSB_REG_PMCR            (RSB_REG_BASE + 0x28)
#define RSB_REG_CMD             (RSB_REG_BASE + 0x2c) //RSB Command Register
#define RSB_REG_SADDR           (RSB_REG_BASE + 0x30) //RSB Slave address Register

//rsb control bit field
#define RSB_SOFT_RST            (1U << 0)
#define RSB_GLB_INTEN           (1U << 1)
#define RSB_ABT_TRANS           (1U << 6)
#define RSB_START_TRANS         (1U << 7)

//rsb state bit field
#define RSB_TOVER_INT           (1U << 0)
#define RSB_TERR_INT            (1U << 1)
#define RSB_LBSY_INT            (1U << 2)

//rsb or twi pmu mode bit field
#define RSB_PMU_INIT            (1U << 31)
#define NTWI_PMU_INIT           (0U << 31)

//local functions
s32  rsb_wait_state(void);
s32 rsb_set_pmu_mode(u32 slave_addr, u32 reg, u32 data);
s32 rsb_set_run_time_addr(u32 saddr,u32 rtsaddr);

#endif  //__RSB_I_H__
