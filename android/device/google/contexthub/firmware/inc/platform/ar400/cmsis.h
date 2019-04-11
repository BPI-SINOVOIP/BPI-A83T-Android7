/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CMSIS_H_
#define _CMSIS_H_


#ifdef __cplusplus
extern "C" {
#endif

#define __NVIC_PRIO_BITS 4
#define __FPU_PRESENT 1



typedef enum IRQn
{
/* -------------------  Cortex    Processor Exceptions Numbers  ------------------ */
    NonMaskableInt_IRQn          = -14,      /*!<  2 Non Maskable Interrupt          */
    HardFault_IRQn               = -13,      /*!<  3 HardFault Interrupt             */
    MemoryManagement_IRQn        = -12,      /*!<  4 Memory Management Interrupt     */
    BusFault_IRQn                = -11,      /*!<  5 Bus Fault Interrupt             */
    UsageFault_IRQn              = -10,      /*!<  6 Usage Fault Interrupt           */
    SVCall_IRQn                  =  -5,      /*!< 11 SV Call Interrupt               */
    DebugMonitor_IRQn            =  -4,      /*!< 12 Debug Monitor Interrupt         */
    PendSV_IRQn                  =  -2,      /*!< 14 Pend SV Interrupt               */
    SysTick_IRQn                 =  -1,      /*!< 15 System Tick Interrupt           */

/* ----------------------  AR400 Specific Interrupt Numbers  ----------------- */
    UART0_IRQn                   = 0,
    UART1_IRQn                   = 1,
    UART2_IRQn                   = 2,
    UART3_IRQn                   = 3,
    I2C0_IRQn                    = 4,
    I2C1_IRQn                    = 5,
    I2C2_IRQn                    = 6,
    I2C3_IRQn                    = 7,
    I2C4_IRQn                    = 8,
    SPI0_IRQn                    = 9,
    SPI1_IRQn                    = 10,
    GPADC_IRQn                   = 13,
    THS_IRQn                     = 14,
    LRADC_IRQn                   = 15,
    I2S0_IRQn                    = 16,
    I2S1_IRQn                    = 17,
    I2S2_IRQn                    = 18,
    DMIC_IRQn                    = 19,
    ACDET_IRQn                   = 20,
    AC_IRQn                      = 21,
    USB_DRD_DEV_IRQn             = 22,
    USB_DRD_EHCI_IRQn            = 23,
    USB_DRD_OHCI_IRQn            = 24,
    USB_HOST_EHCI_IRQn           = 25,
    USB_HOST_OHCI_IRQn           = 26,
    DRAM_IRQn                    = 31,
    NAND_IRQn                    = 32,
    SMHC0_IRQn                   = 33,
    SMHC1_IRQn                   = 34,
    SMHC2_IRQn                   = 35,
    IOMMU_IRQn                   = 36,
    PSI_IRQn                     = 37,
    DMA_IRQn                     = 38,
    MBOX_IRQn                    = 39,
    SPINLOCK_IRQn                = 40,
    HSTIMER0_IRQn                = 41,
    SMC_IRQn                     = 42,
    TIMER0_IRQn                  = 43,
    TIMER1_IRQn                  = 44,
    WDG_IRQn                     = 45,
    GPIOB_IRQn                   = 46,
    GPIOF_IRQn                   = 47,
    GPIOG_IRQn                   = 48,
    GPIOH_IRQn                   = 49,
    CLK_DET_IRQn                 = 50,
    BUS_TIMEOUT_IRQn             = 51,
    MIPI_DSI0_IRQn               = 52,
    MIPI_DSI1_IRQn               = 53,
    TCON_LCD0_IRQn               = 54,
    TCON_LCD1_IRQn               = 55,
    EDP_TX0_IRQn                 = 56,
    CSI0_DMA0_IRQn               = 57,
    CSI0_DMA1_IRQn               = 58,
    CSI0_PARSER0_IRQn            = 61,
    CSI0_CCI0_IRQn               = 63,
    DE_IRQn                      = 68,
    DE_ROT_IRQn                  = 69,
    GPU0_IRQGPU_IRQn             = 71,
    GPU1_IRQJOB_IRQn             = 72,
    GPU2_IRQMMU_IRQn             = 73,
    CE_NS_IRQn                   = 74,
    CE_S_IRQn                    = 75,
    VE_IRQn                      = 76,
    VP9_IRQn                     = 77,
    CPUIDLE_IRQn                 = 78,
    R_GPIOM_IRQn                 = 79,
    EXT_NMI_IRQn                 = 80,
    R_TIMER0_IRQn                = 81,
    R_TIMER1_IRQn                = 82,
    R_TIMER2_IRQn                = 83,
    R_TIMER3_IRQn                = 84,
    R_ALARM0_IRQn                = 85,
    R_ALARM1_IRQn                = 86,
    R_WDG_IRQn                   = 87,
    R_TWDG_IRQn                  = 88,
    R_GPIOL_IRQn                 = 89,
    R_UART0_IRQn                 = 90,
    R_I2C0_IRQn                  = 91,
    R_I2C1_IRQn                  = 92,
    R_I2C2_IRQn                  = 93,
    R_RSB_IRQn                   = 94,
    R_SPI_IRQn                   = 95,
    GIC_OUT_C0_IRQn              = 96,
    NUM_INTERRUPTS
} IRQn_Type;

#include "core_cm4.h"

#ifdef __cplusplus
}
#endif


#endif

