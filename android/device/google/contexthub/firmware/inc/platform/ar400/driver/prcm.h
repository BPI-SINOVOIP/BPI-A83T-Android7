/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                       clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : prcm.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-28
* Descript: clock control unit public header.
* Update  : date                auther      ver     notes
*           2012-4-28 14:48:38  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __PRCM_H__
#define __PRCM_H__

#if (defined CONFIG_ARCH_SUN8IW1P1)
#define CCU_PLL1_REG                (0x01c20000) //CPU
#define CCU_PLL2_REG                (0x01c20008) //AUDIO
#define CCU_PLL3_REG                (0x01c20010) //VIDEO0
#define CCU_PLL4_REG                (0x01c20018) //VE
#define CCU_PLL5_REG                (0x01c20020) //DDR
#define CCU_PLL6_REG                (0x01c20028) //PERIPH
#define CCU_PLL7_REG                (0x01c20030) //VIDEO1
#define CCU_PLL8_REG                (0x01c20038) //GPU
#define CCU_PLL_MIPI_REG            (0x01c20040) //MIPI
#define CCU_PLL9_REG                (0x01c20044) //GPU2
#define CCU_PLL10_REG               (0x01c20048) //DE
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060)
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064)
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_ATS_CLK_REG             (0x01c201b0)
#define CCU_TRACE_CLK_REG           (0x01c201b4)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0)
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4)
#define CCU_PLL_CTRL1               (0x01f01444)
#define CCU_PLL1_LOCK_CTRL1         (0x01c20204)
#elif (defined CONFIG_ARCH_SUN8IW3P1)
#define CCU_PLL1_REG                (0x01c20000) //CPU
#define CCU_PLL2_REG                (0x01c20008) //AUDIO
#define CCU_PLL3_REG                (0x01c20010) //VIDEO
#define CCU_PLL4_REG                (0x01c20018) //VE
#define CCU_PLL5_REG                (0x01c20020) //DDR
#define CCU_PLL6_REG                (0x01c20028) //PERIPH
#define CCU_PLL7_REG                (0x01c20030) //NONE
#define CCU_PLL8_REG                (0x01c20038) //GPU
#define CCU_PLL_MIPI_REG            (0x01c20040) //MIPI
#define CCU_PLL9_REG                (0x01c20044) //HSIC
#define CCU_PLL10_REG               (0x01c20048) //DE
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060)
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064)
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_ATS_CLK_REG             (0x01c201b0)
#define CCU_TRACE_CLK_REG           (0x01c201b4)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0)
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4)
#define CCU_PLL_CTRL1               (0x01f01444)
#define CCU_PLL1_LOCK_CTRL1         (0x01c20204)
#elif (defined CONFIG_ARCH_SUN8IW5P1)
#define CCU_PLL1_REG                (0x01c20000)
#define CCU_PLL2_REG                (0x01c20008)
#define CCU_PLL3_REG                (0x01c20010)
#define CCU_PLL4_REG                (0x01c20018)
#define CCU_PLL5_REG                (0x01c20020)
#define CCU_PLL6_REG                (0x01c20028)
#define CCU_PLL7_REG                (0x01c20038)
#define CCU_PLL8_REG                (0x01c20040)
#define CCU_PLL9_REG                (0x01c20044)
#define CCU_PLL10_REG               (0x01c20048)
#define CCU_PLL11_REG               (0x01c2004c)
#define CCU_PLL_C0_REG              (0x01c20000)
#define CCU_PLL_AUDIO_REG           (0x01c20008)
#define CCU_PLL_VIDEO0_REG          (0x01c20010)
#define CCU_PLL_VE_REG              (0x01c20018)
#define CCU_PLL_DDR0_REG            (0x01c20020)
#define CCU_PLL_PERIPH_REG          (0x01c20028)
#define CCU_PLL_GPU_REG             (0x01c20038)
#define CCU_PLL_MIPI_REG            (0x01c20040)
#define CCU_PLL_HSIC_REG            (0x01c20044)
#define CCU_PLL_DE_REG              (0x01c20048)
#define CCU_PLL_DDR1_REG            (0x01c2004c)
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060)
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064)
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_ATS_CLK_REG             (0x01c201b0)
#define CCU_TRACE_CLK_REG           (0x01c201b4)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0)
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4)
#define CCU_PLL_CTRL1               (0x01f01444)
#define CCU_PLL1_LOCK_CTRL1         (0x01c20204)
#elif (defined CONFIG_ARCH_SUN8IW6P1)
//the base address of ccu register
//name by pll order
#define CCU_PLL1_REG                (0x01c20000)
#define CCU_PLL2_REG                (0x01c20004)
#define CCU_PLL3_REG                (0x01c20008)
#define CCU_PLL4_REG                (0x01c20010)
#define CCU_PLL5_REG                (0x01c20018)
#define CCU_PLL6_REG                (0x01c20020)
#define CCU_PLL7_REG                (0x01c20028)
#define CCU_PLL8_REG                (0x01c20038)
#define CCU_PLL9_REG                (0x01c20044)
#define CCU_PLL10_REG               (0x01c20048)
#define CCU_PLL11_REG               (0x01c2004C)
//name by pll function
#define CCU_PLL_C0_REG              (0x01c20000)
#define CCU_PLL_C1_REG              (0x01c20004)
#define CCU_PLL_AUDIO_REG           (0x01c20008)
#define CCU_PLL_VIDEO0_REG          (0x01c20010)
#define CCU_PLL_VE_REG              (0x01c20018)
#define CCU_PLL_DDR_REG             (0x01c20020)
#define CCU_PLL_PERIPH_REG          (0x01c20028)
#define CCU_PLL_GPU_REG             (0x01c20038)
#define CCU_PLL_HSIC_REG            (0x01c20044)
#define CCU_PLL_DE_REG              (0x01c20048)
#define CCU_PLL_VIDEO1_REG          (0x01c2004C)
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB2_CFG_REG            (0x01c2005C)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060) //CCU_BUS_GATING_REG0 = CCU_AHB1_MCLK_GATING_REG0
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064) //CCU_BUS_GATING_REG1 = CCU_AHB1_MCLK_GATING_REG1
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_CCI400_CLK_REG          (0x01c20078)
#define CCU_PLLS_STB_TIME_REG       (0x01c20200)
#define CCU_PLL_CPUX_STB_TIME_REG   (0x01c20204)
#define CCU_PLL_STB_STATUS_REG      (0x01c2020C)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0) //CCU_BUS_RESET_REG0 = CCU_AHB1_MCLK_RESET_REG0
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4) //CCU_BUS_RESET_REG1 = CCU_AHB1_MCLK_RESET_REG1
#define CCU_BUS_RESET_REG2          (0x01c202c8) //LVDS reset
#define CCU_APB1_MCLK_RESET_REG     (0x01c202D0) //CCU_BUS_RESET_REG3 = CCU_APB1_MCLK_RESET_REG
#define CCU_APB2_MCLK_RESET_REG     (0x01c202D8) //CCU_BUS_RESET_REG4 = CCU_APB2_MCLK_RESET_REG
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x44)
#elif (defined CONFIG_ARCH_SUN8IW9P1)
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x44)
//name by pll order
#define CCU_PLL1_REG                (0x01c20000)
#define CCU_PLL2_REG                (0x01c20004)
#define CCU_PLL3_REG                (0x01c20008)
#define CCU_PLL4_REG                (0x01c20010)
#define CCU_PLL5_REG                (0x01c20018)
#define CCU_PLL6_REG                (0x01c20020)
#define CCU_PLL7_REG                (0x01c20028)
#define CCU_PLL8_REG                (0x01c20038)
#define CCU_PLL9_REG                (0x01c20044)
#define CCU_PLL10_REG               (0x01c20048)
#define CCU_PLL11_REG               (0x01c2004C)
//name by pll function
#define CCU_PLL_C0_REG              (0x01c20000)
#define CCU_PLL_C1_REG              (0x01c20004)
#define CCU_PLL_AUDIO_REG           (0x01c20008)
#define CCU_PLL_VIDEO0_REG          (0x01c20010)
#define CCU_PLL_VE_REG              (0x01c20018)
#define CCU_PLL_DDR_REG             (0x01c20020)
#define CCU_PLL_PERIPH_REG          (0x01c20028)
#define CCU_PLL_GPU_REG             (0x01c20038)
#define CCU_PLL_HSIC_REG            (0x01c20044)
#define CCU_PLL_DE_REG              (0x01c20048)
#define CCU_PLL_VIDEO1_REG          (0x01c2004C)
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB2_CFG_REG            (0x01c2005C)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060) //CCU_BUS_GATING_REG0 = CCU_AHB1_MCLK_GATING_REG0
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064) //CCU_BUS_GATING_REG1 = CCU_AHB1_MCLK_GATING_REG1
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_CCI400_CLK_REG          (0x01c20078)
#define CCU_PLLS_STB_TIME_REG       (0x01c20200)
#define CCU_PLL_CPUX_STB_TIME_REG   (0x01c20204)
#define CCU_PLL_STB_STATUS_REG      (0x01c2020C)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0) //CCU_BUS_RESET_REG0 = CCU_AHB1_MCLK_RESET_REG0
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4) //CCU_BUS_RESET_REG1 = CCU_AHB1_MCLK_RESET_REG1
#define CCU_BUS_RESET_REG2          (0x01c202c8) //LVDS reset
#define CCU_APB1_MCLK_RESET_REG     (0x01c202D0) //CCU_BUS_RESET_REG3 = CCU_APB1_MCLK_RESET_REG
#define CCU_APB2_MCLK_RESET_REG     (0x01c202D8) //CCU_BUS_RESET_REG4 = CCU_APB2_MCLK_RESET_REG

#elif (defined CONFIG_ARCH_SUN50IW1P1)
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x44)
//name by pll order
#define CCU_PLL1_REG                (0x01c20000)
#define CCU_PLL2_REG                (0x01c20008)
#define CCU_PLL3_REG                (0x01c20010)
#define CCU_PLL4_REG                (0x01c20018)
#define CCU_PLL5_REG                (0x01c20020)
#define CCU_PLL6_REG                (0x01c20028)
#define CCU_PLL7_REG                (0x01c2002C)
#define CCU_PLL8_REG                (0x01c20030)
#define CCU_PLL9_REG                (0x01c20038)
#define CCU_PLL10_REG               (0x01c20040)
#define CCU_PLL11_REG               (0x01c20044)
#define CCU_PLL12_REG               (0x01c20048)
#define CCU_PLL13_REG               (0x01c2004C)
//name by pll function
#define CCU_PLL_C0_REG              (0x01c20000)
#define CCU_PLL_AUDIO_REG           (0x01c20008)
#define CCU_PLL_VIDEO0_REG          (0x01c20010)
#define CCU_PLL_VE_REG              (0x01c20018)
#define CCU_PLL_DDR0_REG            (0x01c20020)
#define CCU_PLL_PERIPH0_REG         (0x01c20028)
#define CCU_PLL_PERIPH1_REG         (0x01c2002C)
#define CCU_PLL_VIDEO1_REG          (0x01c20030)
#define CCU_PLL_GPU_REG             (0x01c20038)
#define CCU_PLL_MIPI_REG            (0x01c20040)
#define CCU_PLL_HSIC_REG            (0x01c20044)
#define CCU_PLL_DE_REG              (0x01c20048)
#define CCU_PLL_DDR1_REG            (0x01c2004C)
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB2_CFG_REG            (0x01c2005C)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060) //CCU_BUS_GATING_REG0 = CCU_AHB1_MCLK_GATING_REG0
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064) //CCU_BUS_GATING_REG1 = CCU_AHB1_MCLK_GATING_REG1
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_USBPHY_CFG_REG          (0x01c200cc)
#define CCU_MBUS_CLK_REG            (0x01c2015c)

#define CCU_PLLS_STB_TIME_REG       (0x01c20200)
#define CCU_PLL_CPUX_STB_TIME_REG   (0x01c20204)
#define CCU_PLL_STB_STATUS_REG      (0x01c2020C)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0) //CCU_BUS_RESET_REG0 = CCU_AHB1_MCLK_RESET_REG0
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4) //CCU_BUS_RESET_REG1 = CCU_AHB1_MCLK_RESET_REG1
#define CCU_BUS_RESET_REG2          (0x01c202c8) //LVDS reset
#define CCU_APB1_MCLK_RESET_REG     (0x01c202D0) //CCU_BUS_RESET_REG3 = CCU_APB1_MCLK_RESET_REG
#define CCU_APB2_MCLK_RESET_REG     (0x01c202D8) //CCU_BUS_RESET_REG4 = CCU_APB2_MCLK_RESET_REG
#define CCU_PLL_LOCK_CTRL_REG       (0x01c20320)

#elif (defined CONFIG_ARCH_SUN50IW2P1)
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x44)
//name by pll order
#define CCU_PLL1_REG                (0x01c20000)
#define CCU_PLL2_REG                (0x01c20008)
#define CCU_PLL3_REG                (0x01c20010)
#define CCU_PLL4_REG                (0x01c20018)
#define CCU_PLL5_REG                (0x01c20020)
#define CCU_PLL6_REG                (0x01c20028)
#define CCU_PLL7_REG                (0x01c2002C)
#define CCU_PLL8_REG                (0x01c20030)
#define CCU_PLL9_REG                (0x01c20038)
#define CCU_PLL10_REG               (0x01c20040)
#define CCU_PLL11_REG               (0x01c20044)
#define CCU_PLL12_REG               (0x01c20048)
#define CCU_PLL13_REG               (0x01c2004C)
//name by pll function
#define CCU_PLL_C0_REG              (0x01c20000)
#define CCU_PLL_AUDIO_REG           (0x01c20008)
#define CCU_PLL_VIDEO0_REG          (0x01c20010)
#define CCU_PLL_VE_REG              (0x01c20018)
#define CCU_PLL_DDR0_REG            (0x01c20020)
#define CCU_PLL_PERIPH0_REG         (0x01c20028)
#define CCU_PLL_GPU_REG             (0x01c20038)
#define CCU_PLL_PERIPH1_REG         (0x01c20044)
#define CCU_PLL_DE_REG              (0x01c20048)
#define CCU_CPU_AXI_CFG_REG         (0x01c20050)
#define CCU_AHB1_APB1_CLK_RATIO_REG (0x01c20054)
#define CCU_APB2_CLK_DIV_RATIO_REG  (0x01c20058)
#define CCU_AHB2_CFG_REG            (0x01c2005C)
#define CCU_AHB1_MCLK_GATING_REG0   (0x01c20060) //CCU_BUS_GATING_REG0 = CCU_AHB1_MCLK_GATING_REG0
#define CCU_AHB1_MCLK_GATING_REG1   (0x01c20064) //CCU_BUS_GATING_REG1 = CCU_AHB1_MCLK_GATING_REG1
#define CCU_APB1_MCLK_GATING_REG    (0x01c20068) //CCU_BUS_GATING_REG2 = CCU_APB1_MCLK_GATING_REG
#define CCU_APB2_MCLK_GATING_REG    (0x01c2006c) //CCU_BUS_GATING_REG3 = CCU_APB2_MCLK_GATING_REG
#define CCU_USBPHY_CFG_REG          (0x01c200cc)
#define CCU_MBUS_CLK_REG            (0x01c2015c)

#define CCU_PLLS_STB_TIME_REG       (0x01c20200)
#define CCU_PLL_CPUX_STB_TIME_REG   (0x01c20204)
#define CCU_PLL_STB_STATUS_REG      (0x01c2020C)
#define CCU_AHB1_MCLK_RESET_REG0    (0x01c202c0) //CCU_BUS_RESET_REG0 = CCU_AHB1_MCLK_RESET_REG0
#define CCU_AHB1_MCLK_RESET_REG1    (0x01c202c4) //CCU_BUS_RESET_REG1 = CCU_AHB1_MCLK_RESET_REG1
#define CCU_BUS_RESET_REG2          (0x01c202c8) //LVDS reset
#define CCU_APB1_MCLK_RESET_REG     (0x01c202D0) //CCU_BUS_RESET_REG3 = CCU_APB1_MCLK_RESET_REG
#define CCU_APB2_MCLK_RESET_REG     (0x01c202D8) //CCU_BUS_RESET_REG4 = CCU_APB2_MCLK_RESET_REG
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x44)
#elif (defined CONFIG_ARCH_SUN50IW3P1)
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x244)

//name by pll order
#define CCU_PLLx_REG(n)             (CCU_REG_BASE + (0x8 * (n - 1)))

//name by pll function
#define CCU_PLL_C0_REG              (CCU_PLLx_REG(1))
#define CCU_PLL_DDR0_REG            (CCU_PLLx_REG(3))
#define CCU_PLL_DDR1_REG            (CCU_PLLx_REG(4))
#define CCU_PLL_PERIPH0_REG         (CCU_PLLx_REG(5))
#define CCU_PLL_PERIPH1_REG         (CCU_PLLx_REG(6))
#define CCU_PLL_GPU0_REG            (CCU_PLLx_REG(7))
#define CCU_PLL_VIDEO0_REG          (CCU_PLLx_REG(9))
#define CCU_PLL_VIDEO1_REG          (CCU_PLLx_REG(10))
#define CCU_PLL_VE_REG              (CCU_PLLx_REG(12))
#define CCU_PLL_DE_REG              (CCU_PLLx_REG(13))
#define CCU_PLL_HSIC_REG            (CCU_PLLx_REG(15))
#define CCU_PLL_AUDIO_REG           (CCU_PLLx_REG(16))
#define CCU_CPU_AXI_CFG_REG         (CCU_REG_BASE + 0x500)
#define CCU_PSI_AHB1_AHB2_CFG_REG   (CCU_REG_BASE + 0x510)
#define CCU_AHB3_CFG_REG            (CCU_REG_BASE + 0x51c)
#define CCU_APB1_CFG_REG            (CCU_REG_BASE + 0x520)
#define CCU_APB2_CFG_REG            (CCU_REG_BASE + 0x524)
#define CCU_CCI_CFG_REG             (CCU_REG_BASE + 0x530)
#define CCU_MBUS_CLK_REG            (CCU_REG_BASE + 0x540)
#define CCU_MSGBOX_BGR_REG          (CCU_REG_BASE + 0x71c)
#define CCU_SPINLOCK_BGR_REG        (CCU_REG_BASE + 0x72c)
#elif (defined CONFIG_ARCH_SUN50IW6P1)
//prcm regs
#define CCU_PLL_CTRL1               (R_PRCM_REG_BASE + 0x244)

//name by pll order
#define CCU_PLLx_REG(n)             (CCU_REG_BASE + (0x8 * (n - 1)))

//name by pll function
#define CCU_PLL_C0_REG              (CCU_PLLx_REG(1))
#define CCU_PLL_DDR0_REG            (CCU_PLLx_REG(3))
#define CCU_PLL_DDR1_REG            (CCU_PLLx_REG(4))
#define CCU_PLL_PERIPH0_REG         (CCU_PLLx_REG(5))
#define CCU_PLL_PERIPH1_REG         (CCU_PLLx_REG(6))
#define CCU_PLL_GPU0_REG            (CCU_PLLx_REG(7))
#define CCU_PLL_VIDEO0_REG          (CCU_PLLx_REG(9))
#define CCU_PLL_VIDEO1_REG          (CCU_PLLx_REG(10))
#define CCU_PLL_VE_REG              (CCU_PLLx_REG(11))
#define CCU_PLL_DE_REG              (CCU_PLLx_REG(12))
#define CCU_PLL_HSIC_REG            (CCU_PLLx_REG(14))
#define CCU_PLL_AUDIO_REG           (CCU_PLLx_REG(15))
#define CCU_CPU_AXI_CFG_REG         (CCU_REG_BASE + 0x500)
#define CCU_PSI_AHB1_AHB2_CFG_REG   (CCU_REG_BASE + 0x510)
#define CCU_AHB3_CFG_REG            (CCU_REG_BASE + 0x51c)
#define CCU_APB1_CFG_REG            (CCU_REG_BASE + 0x520)
#define CCU_APB2_CFG_REG            (CCU_REG_BASE + 0x524)
#define CCU_CCI_CFG_REG             (CCU_REG_BASE + 0x530)
#define CCU_MBUS_CLK_REG            (CCU_REG_BASE + 0x540)
#define CCU_MSGBOX_BGR_REG          (CCU_REG_BASE + 0x71c)
#define CCU_SPINLOCK_BGR_REG        (CCU_REG_BASE + 0x72c)

#endif

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || (defined CONFIG_ARCH_SUN8IW5P1) || (defined CONFIG_ARCH_SUN8IW9P1)
//hosc and losc frequency
#define CCU_HOSC_FREQ               (24000000)  //24M
#define CCU_LOSC_FREQ               (32768)     //32768
#define CCU_IOSC_FREQ               (700000)    //700K, should refernce IC hardware
#define CCU_CPUS_POST_DIV           (200000000) //cpus post div source clock freq
#elif (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
#define CCU_HOSC_FREQ               (24000000)  //24M
#define CCU_LOSC_FREQ               (31250)     //31250
#define CCU_IOSC_FREQ               (16000000)  //16M
#define CCU_CPUS_POST_DIV           (300000000) //cpus post div source clock freq
#define CCU_PERIPH0_FREQ            (600000000) //600M
#endif

#if (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN9IW1P1) || (defined CONFIG_ARCH_SUN9IW1P1)
//rtc domian regs
#define RTC_DM_REG0 (0x0)
#define RTC_DM_REG1 (0x1)
#define RTC_DM_REG2 (0x2)
#define RTC_DM_REG3 (0x3)
#define R_PIO_PAD_HOLD_REG  RTC_DM_REG0
#elif (defined CONFIG_ARCH_SUN50IW6P1) || (defined CONFIG_ARCH_SUN50IW3P1)
//rtc domian regs
#define RTC_DM_REG0 (RTC_REG_BASE + 0x100 + (0x4 * 0))
#define RTC_DM_REG1 (RTC_REG_BASE + 0x100 + (0x4 * 1))
#define RTC_DM_REG2 (RTC_REG_BASE + 0x100 + (0x4 * 2))
#define RTC_DM_REG3 (RTC_REG_BASE + 0x100 + (0x4 * 3))
#define RTC_DM_REG4 (RTC_REG_BASE + 0x100 + (0x4 * 4))
#define RTC_DM_REG5 (RTC_REG_BASE + 0x100 + (0x4 * 5))
#define RTC_DM_REG6 (RTC_REG_BASE + 0x100 + (0x4 * 6))
#else
//rtc domian regs
#define RTC_DM_REG0 (0x01f00100)
#define RTC_DM_REG1 (0x01f00104)
#define RTC_DM_REG2 (0x01f00108)
#define RTC_DM_REG3 (0x01f0010c)
#define RTC_DM_REG4 (0x01f00110)
#define RTC_DM_REG5 (0x01f00114)
#define RTC_DM_REG6 (0x01f00118)
#endif



//the clock status of on-off
typedef enum ccu_clk_onoff
{
	CCU_CLK_OFF = 0x0,          //clock on status
	CCU_CLK_ON  = 0x1,          //clock off status
} ccu_clk_onff_e;

//the clock status of reset
typedef enum ccu_clk_reset
{
	CCU_CLK_RESET   = 0x0,      //reset valid status
	CCU_CLK_NRESET  = 0x1,      //reset invalid status
} ccu_clk_reset_e;

//command for call-back function of clock change
typedef enum ccu_clk_cmd
{
	CCU_CLK_CLKCHG_REQ = 0x0,   //command for notify that clock will change
	CCU_CLK_CLKCHG_DONE,        //command for notify that clock change finish
} ccu_clk_cmd_e;

//command for call-back function of 24M hosc on-off
typedef enum ccu_hosc_cmd
{
	CCU_HOSC_ON_READY_NOTIFY = 0x0, //command for notify that 24mhosc power-on already
	CCU_HOSC_WILL_OFF_NOTIFY,       //command for notify that 24mhosc will off
} ccu_hosc_cmd_e;

//the state of power-off gating
typedef enum poweroff_gating_state
{
	CCU_POWEROFF_GATING_INVALID = 0x0,
	CCU_POWEROFF_GATING_VALID   = 0x1,
} poweroff_gating_state_e;

//source clocks ID
typedef enum ccu_src_clk
{
	CCU_SYS_CLK_NONE = 0x0, //invalid source clock id

	CCU_SYS_CLK_LOSC,   //LOSC, 33/50/67:32768Hz, 73:16MHz/512=31250
	CCU_SYS_CLK_IOSC,   //InternalOSC,  33/50/67:700KHZ, 73:16MHz
	CCU_SYS_CLK_HOSC,   //HOSC, 24MHZ clock
	CCU_SYS_CLK_AXI,    //AXI clock
	CCU_SYS_CLK_16M,    //16M for the backdoor

	CCU_SYS_CLK_PLL1,   //PLL1 clock
	CCU_SYS_CLK_PLL2,   //PLL2 clock
	CCU_SYS_CLK_PLL3,   //PLL3 clock
	CCU_SYS_CLK_PLL4,   //PLL4 clock
	CCU_SYS_CLK_PLL5,   //PLL5 clock
	CCU_SYS_CLK_PLL6,   //PLL6 clock
	CCU_SYS_CLK_PLL7,   //PLL7 clock
	CCU_SYS_CLK_PLL8,   //PLL8 clock
	CCU_SYS_CLK_PLL9,   //PLL9 clock
	CCU_SYS_CLK_PLL10,  //PLL10 clock
	CCU_SYS_CLK_PLL11,  //PLL10 clock

	CCU_SYS_CLK_CPUS,   //cpus clock
	CCU_SYS_CLK_C0,     //cluster0 clock
	CCU_SYS_CLK_C1,     //cluster1 clock
	CCU_SYS_CLK_AXI0,   //AXI0 clock
	CCU_SYS_CLK_AXI1,   //AXI0 clock
	CCU_SYS_CLK_AHB0,   //AHB0 clock
	CCU_SYS_CLK_AHB1,   //AHB1 clock
	CCU_SYS_CLK_AHB2,   //AHB2 clock
	CCU_SYS_CLK_APB0,   //APB0 clock
	CCU_SYS_CLK_APB1,   //APB1 clock
	CCU_SYS_CLK_APB2,   //APB2 clock
	CCU_SYS_CLK_AHB3,   //AHB3 clock
	CCU_SYS_CLK_PSI,    //PSI clock
	CCU_SYS_CLK_AHBS,   //AHBS clock
	CCU_SYS_CLK_APBS1,  //APBS1 clock
	CCU_SYS_CLK_APBS2,  //APBS2 clock
	CCU_SYS_CLK_PLL_PERI0_1X,  //PLL_PERI0(1X) clock
	CCU_SYS_CLK_PLL_PERI1_1X,  //PLL_PERI1(1X) clock
	CCU_SYS_CLK_PLL_PERI0_2X,  //PLL_PERI0(2X) clock
	CCU_SYS_CLK_PLL_PERI1_2X,  //PLL_PERI1(2X) clock
	CCU_SYS_CLK_SPI,           //spi clock
} ccu_sys_clk_e;

//module clocks ID
typedef enum ccu_mod_clk
{
	CCU_MOD_CLK_NONE,

	CCU_MOD_CLK_CPUS,
	CCU_MOD_CLK_AHB0,
	CCU_MOD_CLK_APB0,

	CCU_MOD_CLK_C0,
	CCU_MOD_CLK_C1,
	CCU_MOD_CLK_CPU0,
	CCU_MOD_CLK_CPU1,
	CCU_MOD_CLK_CPU2,
	CCU_MOD_CLK_CPU3,
	CCU_MOD_CLK_AHB1,
	CCU_MOD_CLK_AHB2,
	CCU_MOD_CLK_APB1,
	CCU_MOD_CLK_APB2,
	CCU_MOD_CLK_DMA,
	CCU_MOD_CLK_SDRAM,
	CCU_MOD_CLK_SPINLOCK,
	CCU_MOD_CLK_MSGBOX,
	CCU_MOD_CLK_SS,
	CCU_MOD_CLK_AXI,
	CCU_MOD_CLK_AXI0,
	CCU_MOD_CLK_AXI1,
	CCU_MOD_CLK_R_TH,
	CCU_MOD_CLK_R_TWI,
	CCU_MOD_CLK_R_TWI0,
	CCU_MOD_CLK_R_TWI1,
	CCU_MOD_CLK_R_TWI2,
	CCU_MOD_CLK_R_ONEWIRE,
	CCU_MOD_CLK_R_UART,
	CCU_MOD_CLK_R_TIMER0_1,
	CCU_MOD_CLK_R_P2WI,
	CCU_MOD_CLK_R_RSB,
	CCU_MOD_CLK_R_SPI,
	CCU_MOD_CLK_R_CIR,
	CCU_MOD_CLK_R_PIO,

	CCU_MOD_CLK_VDD_SYS,
	CCU_MOD_CLK_CCI400,
	CCU_MOD_CLK_PSI,
	CCU_MOD_CLK_AHB3,
	CCU_MOD_CLK_AHBS,
	CCU_MOD_CLK_APBS1,
	CCU_MOD_CLK_APBS2,
	CCU_MOD_CLK_R_RTC,
	CCU_MOD_CLK_R_CPUSCFG,
	CCU_MOD_CLK_R_PRCM,
	CCU_MOD_CLK_R_WDG,
	CCU_MOD_CLK_R_TWD,
	CCU_MOD_CLK_R_PWM,
	CCU_MOD_CLK_R_INTC,
	CCU_MOD_CLK_CPU_APB,
	CCU_MOD_CLK_SYSTICK,
} ccu_mod_clk_e;

//the power control modules
typedef enum power_control_module
{
	//cpux power controls
	PWRCTL_C0CPUX,
	PWRCTL_C0CPU0,
	PWRCTL_C0CPU1,
	PWRCTL_C0CPU2,
	PWRCTL_C0CPU3,

	PWRCTL_C1CPUX,
	PWRCTL_C1CPU0,
	PWRCTL_C1CPU1,
	PWRCTL_C1CPU2,
	PWRCTL_C1CPU3,

	//vdd-sys power controls
	PWRCTL_VDD_CPUX_GPIO_PAD_HOLD,
	PWRCTL_VDD_CPUS,
	PWRCTL_VDD_AVCC_A,
	PWRCTL_VCC_PLL,
	PWRCTL_VCC_PLL_LOW_VOLT,

	//gpu power control
	PWRCTL_GPU,
} power_control_module_e;

/*
*********************************************************************************************************
*                                       INITIALIZE CCU
*
* Description:  initialize clock control unit.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize ccu succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_init(void);

/*
*********************************************************************************************************
*                                       EXIT CCU
*
* Description:  exit clock control unit.
*
* Arguments  :  none.
*
* Returns    :  OK if exit ccu succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_exit(void);

/*
*********************************************************************************************************
*                                      SET SOURCE FREQUENCY
*
* Description:  set the frequency of a specific source clock.
*
* Arguments  :  sclk : the source clock ID which we want to set frequency.
*               freq : the frequency which we want to set.
*
* Returns    :  OK if set source frequency succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_sclk_freq(u32 sclk, u32 freq);

/*
*********************************************************************************************************
*                                     GET SOURCE FREQUENCY
*
* Description:  get the frequency of a specific source clock.
*
* Arguments  :  sclk : the source clock ID which we want to get frequency.
*
* Returns    :  frequency of the specific source clock.
*********************************************************************************************************
*/
s32 ccu_get_sclk_freq(u32 sclk);

/*
*********************************************************************************************************
*                                     REGISTER MODULE CB
*
* Description:  register call-back for module clock, when the source frequency
*               of the module clock changed, it will use this call-back to notify
*               module driver.
*
* Arguments  :  mclk    : the module clock ID which we want to register call-back.
*               pcb     : the call-back which we want to register.
*
* Returns    :  OK if register call-back succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_reg_mclk_cb(u32 mclk, __pNotifier_t pcb);

/*
*********************************************************************************************************
*                                     UNREGISTER MODULE CB
*
* Description:  unregister call-back for module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to unregister call-back.
*               pcb     : the call-back which we want to unregister.
*
* Returns    :  OK if unregister call-back succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_unreg_mclk_cb(u32 mclk, __pNotifier_t pcb);

/*
*********************************************************************************************************
*                                    SET SOURCE OF MODULE CLOCK
*
* Description:  set the source of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to set source.
*               sclk    : the source clock ID whick we want to set as source.
*
* Returns    :  OK if set source succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_mclk_src(u32 mclk, u32 sclk);

/*
*********************************************************************************************************
*                                    GET SOURCE OF MODULE CLOCK
*
* Description:  get the source of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to get source.
*
* Returns    :  the source clock ID of source clock.
*********************************************************************************************************
*/
s32 ccu_get_mclk_src(u32 mclk);

/*
*********************************************************************************************************
*                                    SET DIVIDER OF MODULE CLOCK
*
* Description:  set the divider of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to set divider.
*               div     : the divider whick we want to set as source.
*
* Returns    :  OK if set divider succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_mclk_div(u32 mclk, u32 div);

/*
*********************************************************************************************************
*                                    GET DIVIDER OF MODULE CLOCK
*
* Description:  get the divider of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to get divider.
*
* Returns    :  the divider of the specific module clock.
*********************************************************************************************************
*/
s32 ccu_get_mclk_div(u32 mclk);

/*
*********************************************************************************************************
*                                    SET ON-OFF STATUS OF MODULE CLOCK
*
* Description:  set the on-off status of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to set on-off status.
*               onoff   : the on-off status which we want to set, the detail please
*                         refer to the clock status of on-off.
*
* Returns    :  OK if set module clock on-off status succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_mclk_onoff(u32 mclk, s32 onoff);

/*
*********************************************************************************************************
*                                    SET RESET STATUS OF MODULE CLOCK
*
* Description:  set the reset status of a specific module clock.
*
* Arguments  :  mclk    : the module clock ID which we want to set reset status.
*               reset   : the reset status which we want to set, the detail please
*                         refer to the clock status of reset.
*
* Returns    :  OK if set module clock reset status succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_mclk_reset(u32 mclk, s32 reset);

/*
*********************************************************************************************************
*                                    SET POWER OFF STATUS OF HWMODULE
*
* Description:  set the power off gating status of a specific module.
*
* Arguments  :  module  : the module ID which we want to set power off gating status.
*               status  : the power off status which we want to set, the detail please
*                         refer to the status of power-off gating.
*
* Returns    :  OK if set module power off gating status succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_set_poweroff_gating_state(s32 module, s32 state);

/*
*********************************************************************************************************
*                                           RESET MODULE
*
* Description:  reset a specific module.
*
* Arguments  :  module  : the module clock ID which we want to reset.
*
* Returns    :  OK if reset module succeeded, others if failed.
*********************************************************************************************************
*/
s32 ccu_reset_module(u32 mclk);

s32 ccu_24mhosc_disable(void);
s32 ccu_24mhosc_enable(void);
s32 ccu_24mhosc_reg_cb(__pNotifier_t pcb);
s32 is_hosc_lock(void);

void save_state_flag(u32 value);
u32 read_state_flag(void);

/* box */
extern u32 read_rtc_domain_reg(u32 reg);
extern void write_rtc_domain_reg(u32 reg, u32 value);
#define tvbox_save_rtc_flag(v) write_rtc_domain_reg(RTC_DM_REG3, v)
#define tvbox_read_rtc_flag() read_rtc_domain_reg(RTC_DM_REG3)

#if (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
int cpu_power_set(unsigned int cluster, unsigned int cpu, bool enable);
int cluster_power_set(unsigned int cluster, bool enable);
#endif // CONFIG_ARCH_SUN8IW6P1

#if (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
extern u32 iosc_freq;
extern u32 losc_freq;
void osc_freq_init(void);
#endif

#endif  //__PRCM_H__
