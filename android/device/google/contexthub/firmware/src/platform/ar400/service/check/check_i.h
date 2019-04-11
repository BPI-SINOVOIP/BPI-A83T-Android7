/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                check module
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : ckeck_i.h
* By      : Superm
* Version : v1.0
* Date    : 2015-1-19
* Descript: check module internal header.
* Update  : date                auther      ver     notes
*           2015-1-19 8:58:37   Superm      1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __CHECK_I_H__
#define __CHECK_I_H__

#include "include.h"

#define		H8_CHIP_ID		0x03		//000011B
#define		AXP818_CHIP_ID		0x18

#define		R58_CHIP_ID1		0x05		//000101B
#define		R58_CHIP_ID2		0x04		//000100B

#define		A83T_CHIP_ID		0x1		//000001B
#define		AXP813_CHIP_ID		0x13

#define		AXP_DEFAULT_CHIP_ID	0x00
#define		IC_DEFAULT_CHIP_ID	0x0		//000000B

#define		ID_VERSION			        "V0.2"

#if defined(HOMLET_PLATFORM)
#define		IC_NAME					"H8"
#define		IC_CHIP_ID				H8_CHIP_ID
#define		IC_CHIP_ID2				H8_CHIP_ID
#define		AXP_NAME				"AXP818"
#define		AXP_CHIP_ID				AXP818_CHIP_ID
#elif defined(R58_PLATFORM)
#define		IC_NAME					"R58"
#define		IC_CHIP_ID				R58_CHIP_ID1
#define		IC_CHIP_ID2				R58_CHIP_ID2
#define		AXP_NAME				"AXP813"
#define		AXP_CHIP_ID				AXP813_CHIP_ID
#else
#define		IC_NAME					"A83T"
#define		IC_CHIP_ID				A83T_CHIP_ID
#define		IC_CHIP_ID2				A83T_CHIP_ID
#define		AXP_NAME				"AXP813"
#define		AXP_CHIP_ID				AXP813_CHIP_ID
#endif

#define IC_CHIP_ID_ADDR				        0x01c14200
#define PMU_PACKAGE_REG                                 (0x13e)

//local functions


#endif  //__DVFS_I_H__
