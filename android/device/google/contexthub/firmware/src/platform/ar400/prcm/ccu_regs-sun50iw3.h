/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                         clock control unit module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : ccu_regs.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: clock control unit register defines.
* Update  : date                auther      ver     notes
*           2012-5-7 8:47:58    Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __CCU_REGS_H__
#define __CCU_REGS_H__

#if (defined CONFIG_ARCH_SUN50IW3P1)

typedef struct ccu_cpus_clk_cfg_reg0000
{
	u32 factor_m:5;             //bit0,  cpus clock ratio
	u32 reserved2:3;            //bit5,  reserved
	u32 factor_n:2;             //bit8,  cpus post divider
	u32 reserved1:14;           //bit10, reserved
	u32 src_sel:2;              //bit24, cpus source select
	u32 reserved0:6;            //bit26, reserved
} ccu_cpus_clk_cfg_reg0000_t;

typedef struct ccu_systick_clk_cfg_reg0008
{
	u32 src_sel:2;             //bit0,  systick clock source select
	u32 reserved1:14;          //bit1,  reserved
	u32 enable:1;              //bit16,  clock gating enable
	u32 reserved0:15;          //bit17, reserved
} ccu_systick_clk_cfg_reg0008_t;

typedef struct ccu_apbs1_cfg_reg000c
{
	u32 factor_m:2;              //bit0,  apbs1 clock divider ratio
	u32 reserved0:30;           //bit2,  reserved
} ccu_apbs1_cfg_reg000c_t;

typedef struct ccu_apbs2_cfg_reg0010
{
	u32 factor_m:5;             //bit0,  cpus clock ratio
	u32 reserved2:3;            //bit5,  reserved
	u32 factor_n:2;             //bit8,  cpus post divider
	u32 reserved1:14;           //bit10, reserved
	u32 src_sel:2;              //bit24, cpus source select
	u32 reserved0:6;            //bit26, reserved
} ccu_apbs2_cfg_reg0010_t;

typedef struct ccu_mod_gate_reset_reg
{
	u32 gate:1;             //bit0,  gate
	u32 reserved1:15;       //bit1,  reserved
	u32 reset:1;            //bit16, reset
	u32 reserved0:15;       //bit17, reserved
} ccu_mod_gate_reset_reg_t;

typedef struct ccu_twi_gate_reset_reg
{
	u32 twi0_gate:1;        //bit0,  gate
	u32 twi1_gate:1;        //bit1,  gate
	u32 twi2_gate:1;        //bit2,  gate
	u32 reserved1:13;       //bit3,  reserved
	u32 twi0_reset:1;       //bit16, reset
	u32 twi1_reset:1;       //bit17, reset
	u32 twi2_reset:1;       //bit18, reset
	u32 reserved0:13;       //bit19, reserved
} ccu_twi_gate_reset_reg_t;

typedef struct ccu_mod_clk_reg
{
	u32 factor_m:5;             //bit0,  clock divider ratio m
	u32 reserved2:3;            //bit5,  reserved
	u32 factor_n:2;             //bit8,  clock pre-divider ratio n
	u32 reserved1:14;           //bit10, reserved
	u32 src_sel:1;              //bit24, clock source select
	u32 reserved0:6;            //bit25, reserved
	u32 sclk_gate:1;            //bit31, gating special clock(max clock = 24M)
} ccu_mod_clk_reg_t;

typedef struct ccu_spi_clk_reg
{
	u32 factor_m:4;             //bit0,  clock divider ratio m
	u32 reserved2:4;            //bit5,  reserved
	u32 factor_n:2;             //bit8,  clock pre-divider ratio n
	u32 reserved1:14;           //bit10, reserved
	u32 src_sel:3;              //bit24, clock source select
	u32 reserved0:4;            //bit25, reserved
	u32 sclk_gate:1;            //bit31, gating special clock(max clock = 24M)
} ccu_spi_clk_reg_t;

typedef struct ccu_pll_ctrl_reg0240
{
	u32 pll_bias_en:1;              //bit0,  pll bias enbale
	u32 gm0:1;                      //bit1,  gm0
	u32 gm1:1;                      //bit2,  gm1
	u32 reserved1:21;               //bit3,  reserved
	u32 test_clk_sel:1;             //bit24, test clock select
	u32 reserved0:7;                //bit25, reserved
} ccu_pll_ctrl_reg0240_t;

typedef struct ccu_pll_ctrl_reg0244
{
	u32 ldo_en:1;               //bit0,  ldo enable, all pll digital power
	u32 reserved2:1;            //bit1,  reserved
	u32 osc24M_en:1;            //bit2,  osc24M enable
	u32 plltest_en:1;           //bit3,  clock test enable, for verify
	u32 mbias_en:1;             //bit4,  chip master bias enable
	u32 reserved1:11;           //bit5,  reserved
	u32 pllvdd_ldo_out_ctrl:3;  //bit16, pllvdd ldo output control
	u32 reserved0:5;            //bit19, reserved
	u32 key_field:8;            //bit24, key field LDO enable bit
} ccu_pll_ctrl_reg0244_t;

typedef struct ccu_cpu_pwroff_gate_reg0100
{
	u32 cpu0_poweroff_gate:1;   //bit0, cpu0 power off gate
	u32 cpu1_poweroff_gate:1;   //bit1, cpu1 power off gate
	u32 cpu2_poweroff_gate:1;   //bit2, cpu2 power off gate
	u32 cpu3_poweroff_gate:1;   //bit3, cpu3 power off gate
	u32 reserved0:28;           //bit4, reserved
} ccu_cpu_pwroff_gate_reg0100_t;

typedef struct ccu_sys_pwroff_gate_reg0250
{
	u32 dram_ch0_pad_hold:1;//bit0, hold the pad of dram channel0
	u32 dram_ch1_pad_hold:1;//bit1, hold the pad of dram channel1
	u32 avcc_a_gate:1;      //bit2, gating the corresponding modules to the
				//AVCC_A power domain when VDD_SYS power off
	u32 vdd_cpus_gate:1;    //bit3, gating the corresponding modules to the
				//CPUS power domain when VDD_SYS power off
	u32 reserved0:28;       //bit4, reserved
} ccu_sys_pwroff_gate_reg0250_t;

typedef struct ccu_gpu_pwroff_gate_reg0254
{
	u32 poweroff_gate:1;    //bit0, gating the corresponding modules
				//when GPU power off
	u32 reserved0:31;       //bit1, reserved
} ccu_gpu_pwroff_gate_reg0254_t;

typedef struct ccu_ve_pwroff_gate_reg0258
{
	u32 poweroff_gate:1;    //bit0, gating the corresponding modules
				//when VE power off
	u32 reserved0:31;       //bit1, reserved
} ccu_ve_pwroff_gate_reg0258_t;

typedef struct ccu_sys_pwr_rst_reg0260
{
	u32 module_reset:1;     //bit0, VDD_SYS power domain modules should be reset
							//before VDD_SYS power on
	u32 reserved0:31;       //bit1, reserved
} ccu_sys_pwr_rst_reg0260_t;

typedef struct ccu_prcm_sec_sw_cfg_reg0290
{
	u32 cpus_clk_sec:1; //bit0, cpus clk relevant register's security
	u32 pll_sec:1;      //bit1, pll ctrl relevant register's security
	u32 power_sec:1;    //bit2, power relevant register' security
	u32 reserved0:29;   //bit3, reserved
} ccu_prcm_sec_sw_cfg_reg0290_t;

typedef struct ccu_nmi_int_ctrl_reg0320
{
	u32 nmi_src_type:2;     //bit0, external nmi interrupt source type
	u32 reserved0:30;       //bit2, reserved
} ccu_nmi_int_ctrl_reg0320_t;

typedef struct ccu_nmi_irq_en_reg0324
{
	u32 nmi_irq_en:1;       //bit0, nmi interrupt irq enable
	u32 reserved0:31;       //bit1, reserved
} ccu_nmi_irq_en_reg0324_t;

typedef struct ccu_nmi_irq_status_reg0328
{
	u32 nmi_irq_pend:1;     //bit0, nmi interrupt irq pending status
	u32 reserved0:31;       //bit1, reserved
} ccu_nmi_irq_status_reg0328_t;

typedef struct ccu_pll_c0_cpux_reg0000
{
	volatile u32 factor_m:2;    //bit0,  PLL1 Factor_M
	volatile u32 reserved3:6;   //bit2,  reserved
	volatile u32 factor_n:8;    //bit8,  PLL1 Factor_N
	volatile u32 factor_p:2;    //bit16, PLL1 Factor_P
	volatile u32 reserved2:6;   //bit18, reserved
	volatile u32 lock_time:3;   //bit24, lock time:freq scaling step
	volatile u32 reserved1:1;   //bit27, reserved
	volatile u32 lock_st:1;     //bit28, 0-unlocked, 1-locked(PLL has been stable)
	volatile u32 lock_en:1;     //bit29, 0-disable lock, 1-enable lock
	volatile u32 reserved0:1;   //bit30, reserved
	volatile u32 enable:1;      //bit31, 0-disable, 1-enable, (24Mhz*N*K)/(M)
} ccu_pll_c0_cpux_reg0000_t;

typedef struct ccu_pll_periph0_reg0020
{
	volatile u32 factor_m0:1;   //bit0,  factor_m0
	volatile u32 factor_m1:1;   //bit1,  factor_m1
	volatile u32 reserved2:6;   //bit2, reserved
	volatile u32 factor_n:8;    //bit8,  factor_n
	volatile u32 reserved1:12;  //bit16, reserved
	volatile u32 lock_st:1;     //bit28, reserved
	volatile u32 lock_en:1;     //bit29, 0-unlocked, 1-locked(PLL has been stable)
	volatile u32 reserved0:1;   //bit30, reserved
	volatile u32 enable:1;      //bit31, 0-disable, 1-enable, (24Mhz*N*K)/(M)
} ccu_pll_periph0_reg0020_t;

typedef struct ccu_reg_list
{
	volatile ccu_cpus_clk_cfg_reg0000_t     cpus_clk_cfg;   //0x0000
	volatile u32                            reserved0[1];   //0x0004
	volatile ccu_systick_clk_cfg_reg0008_t  systick_clk_cfg;//0x0008
	volatile ccu_apbs1_cfg_reg000c_t        apbs1_cfg;      //0x000c
	volatile ccu_apbs2_cfg_reg0010_t        apbs2_cfg;      //0x0010
	volatile u32                            reserved1[66];  //0x0014 - 0x0110
	volatile ccu_mod_gate_reset_reg_t       r_timer;        //0x011c
	volatile u32                            reserved2[3];   //0x0120 - 0x0128
	volatile ccu_mod_gate_reset_reg_t       r_twd;          //0x012c
	volatile u32                            reserved3[3];   //0x0130 - 0x0138
	volatile ccu_mod_gate_reset_reg_t       r_pwm;          //0x013c
	volatile u32                            reserved4[19];  //0x0140 - 0x0188
	volatile ccu_mod_gate_reset_reg_t       r_uart;         //0x018c
	volatile u32                            reserved5[3];   //0x0190 - 0x0198
	volatile ccu_twi_gate_reset_reg_t       r_twi;          //0x019c
	volatile u32                            reserved6[7];   //0x01a0 - 0x01b8
	volatile ccu_mod_gate_reset_reg_t       r_rsb;          //0x01bc
	volatile ccu_mod_clk_reg_t              r_ir_clk;       //0x01c0
	volatile u32                            reserved7[2];   //0x01c4 - 0x01c8
	volatile ccu_mod_gate_reset_reg_t       r_ir;           //0x01cc
	volatile u32                            reserved8[4];   //0x01d0 - 0x01dc
	volatile ccu_mod_clk_reg_t              r_owc_clk;      //0x01e0
	volatile u32                            reserved9[2];   //0x01e4 - 0x01e8
	volatile ccu_mod_gate_reset_reg_t       r_owc;          //0x01ec
	volatile ccu_spi_clk_reg_t              r_spi_clk;      //0x01f0
	volatile u32                            reserved10[2];  //0x01f4 - 0x01f8
	volatile ccu_mod_gate_reset_reg_t       r_spi;          //0x01fc
	volatile u32                            reserved11[3];  //0x0200 - 0x0208
	volatile ccu_mod_gate_reset_reg_t       r_rtc;          //0x020c
	volatile u32                            reserved12[12]; //0x0210 - 0x023c
	volatile ccu_pll_ctrl_reg0240_t         pll_ctrl0;      //0x0240
	volatile ccu_pll_ctrl_reg0244_t         pll_ctrl1;      //0x0244
	volatile u32                            reserved13[2];  //0x0248 - 0x024c
	volatile ccu_sys_pwroff_gate_reg0250_t  sys_pwroff_gate;//0x0250
	volatile ccu_gpu_pwroff_gate_reg0254_t	gpu_pwroff_gate;//0x0254
	volatile ccu_ve_pwroff_gate_reg0258_t	ve_pwroff_gate; //0x0258
	volatile u32                            reserved14;     //0x025c
	volatile ccu_sys_pwr_rst_reg0260_t      sys_pwr_rst;    //0x0260
	volatile u32                            reserved15[3];  //0x0264 - 0x26c
	volatile u32                            ram_cfg;        //0x0270
	volatile u32                            ram_test;       //0x0274
	volatile u32                            reserved16[2];  //0x0278 - 0x027c
	volatile u32                            ac_pr_cfg;      //0x0280
	volatile u32                            reserved17[3];  //0x0284 - 0x028c
	volatile ccu_prcm_sec_sw_cfg_reg0290_t  prcm_sec_sw_cfg;//0x0290
	volatile u32                            reserved18[27]; //0x0294 - 0x02fc
	volatile u32                            sjtag_en_lock;  //0x0300
	volatile u32                            reserved19[3];  //0x0304 - 0x030c
	volatile u32                            res_cal_ctrl;   //0x0310
	volatile u32                            res200_man_ctrl;//0x0314
	volatile u32                            res240_man_ctrl;//0x0318
	volatile u32                            res_cal_st;     //0x031c
	volatile ccu_nmi_int_ctrl_reg0320_t     nmi_int_ctrl;   //0x0320
	volatile ccu_nmi_irq_en_reg0324_t       nmi_int_en;     //0x0324
	volatile ccu_nmi_irq_status_reg0328_t   nmi_int_st;     //0x0328
	volatile u32                            reserved20[49]; //0x032c - 0x03ec
	volatile u32                            prcm_version;   //0x03f0
} ccu_reg_list_t;

#endif // sun50iw3
#endif  //__CCU_REGS_H__
