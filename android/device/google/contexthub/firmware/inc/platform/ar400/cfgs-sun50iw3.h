/*
 * include/cfgs.h
 *
 * Descript: system configure header.
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: superm <superm@allwinnertech.com>
 *
 */
#ifndef __SUN50IW3_CFGS_H__
#define __SUN50IW3_CFGS_H__

#if defined CONFIG_ARCH_SUN50IW3P1

#define ARISC_USE_DRAM_CODE_PBASE       (0x48100000)
#define ARISC_USE_DRAM_CODE_SIZE        (0x00003000)
#define ARISC_USE_DRAM_DATA_PBASE       (ARISC_USE_DRAM_CODE_PBASE + ARISC_USE_DRAM_CODE_SIZE)
#define ARISC_USE_DRAM_DATA_SIZE        (0x00002000)
#define ARISC_PARA_ADDR                 (0x48105000)

#define DDR2_FPGA_S2C_16B

#define TWI_SCK_FREQ                  (400000)           //the p2wi SCK freq(2M)
#define TWI_SDAODLY                   (1)             //SDA output delay
#define TWI_TRANS_BYTE_MAX            (8)             //the max number of P2WI transfer byte

/* uart config */
#define UART_BAUDRATE                   (115200)

#if ((defined KERNEL_USED) || (defined TF_USED))
/* services define */
#define AUDIO_USED              (0)     /* config here */
#define DVFS_USED               (1)     /* config here */
#define SST_USED                (0)     /* config here */
#define EST_USED                (1)     /* config here */
#define NST_USED                (0)     /* config here */
#define TST_USED                (0)     /* config here */
#define FAKE_POWOFF_USED        (0)     /* config here */
#define MEM_USED                (1)     /* config here */
#define SUSPEND_POWER_CHECK     (0)     /* config here */ /* power check fun during standby */
#define CPUIDLE_USED            (1)     /* config here */
#define LED_BLN_USED            (0)     /* config here */
#define CHECK_USED              (0)     /* config here */
#define VERIFY_USED             (0)     /* config here */
#define CPUOP_USED              (1)     /* config here */
#define SYSOP_USED              (1)     /* config here */
/* system define */
#define INF_USED                (0)     /* config here */
#define LOG_USED                (1)     /* config here */
#define WRN_USED                (1)     /* config here */
#define ERR_USED                (1)     /* config here */
#define SHELL_USED              (0)     /* config here */
#define TRACETIME_USED          (0)     /* config here */
/* devices define */
#define TWI_USED                (1)     /* config here */
#define RSB_USED                (1)     /* config here */
#define PMU_USED                (1)     /* config here */
#define PMU_CHRCUR_CRTL_USED    (0)     /* config here */
#define DMA_USED                (0)     /* config here */ /* super-standby whether used dma_memcpy */
#define SS_USED                 (0)     /* config here */
#define IR_USED                 (0)     /* config here */
#define HW_MESSAGE_USED         (1)     /* config here */
#define WATCHDOG_USED           (1)     /* config here */
#define THERMAL_USED            (0)     /* config here */
#define SET_PWR_TREE_USED       (1)     /* config here */
#define DRAM_USED               (1)     /* config here */
#else /* !((defined KERNEL_USED) || (defined TF_USED)), BOOT_USED */
#define AUDIO_USED              (0)     /* not KERNEL mode, audio can't be used */
#define DVFS_USED               (0)     /* not KERNEL mode, dvfs can't be used  */
#define SST_USED                (0)     /* not KERNEL mode, sst can't be used   */
#define EST_USED                (0)     /* not KERNEL mode, est can't be used   */
#define NST_USED                (0)     /* not KERNEL mode, nst can't be used   */
#define TST_USED                (0)     /* not KERNEL mode, tst can't be used   */
#define FAKE_POWOFF_USED        (0)     /* not KERNEL mode, fpf can't be used   */
#define MEM_USED                (0)     /* not KERNEL mode, mem not be used     */
#define SUSPEND_POWER_CHECK     (0)     /* not KERNEL mode, powcheck not be used*/
#define CPUIDLE_USED            (0)     /* not KERNEL mode, cpuidl can't be used*/
#define LED_BLN_USED            (0)     /* not KERNEL mode, bln not be used     */
#define VERIFY_USED             (0)     /* not KERNEL mode, firmware verify uls */
/* system define */
#define INF_USED                (1)     /* config here */
#define LOG_USED                (1)     /* config here */
#define WRN_USED                (1)     /* config here */
#define ERR_USED                (1)     /* config here */
#define SHELL_USED              (0)     /* config here */
#define TRACETIME_USED          (0)     /* config here */
/* devices define */
#define TWI_USED                (0)     /* config here */
#define PMU_USED                (1)     /* config here */
#define PMU_CHRCUR_CRTL_USED    (0)     /* not KERNEL mode, chrcur can't be used*/
#define DMA_USED                (0)     /* not KERNEL mode, dma usedless        */
#define SS_USED                 (0)     /* config here */
#define IR_USED                 (1)     /* config here */
#define HW_MESSAGE_USED         (0)     /* not KERNEL mode, msg used            */
#define WATCHDOG_USED           (1)     /* config here */
#define DRAM_USED               (0)     /* config here */
#endif
#else
#err "this file should not been included!"
#endif
#endif /* __SUN50IW3_CFGS_H__ */
