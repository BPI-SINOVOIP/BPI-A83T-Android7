/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                dram module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dram.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-4-28
* Descript: dram driver public header.
* Update  : date                auther      ver     notes
*           2012-4-28 10:28:51  Sunny       1.0     Create this file.
*********************************************************************************************************
*/
#ifndef __DRAM_H__
#define __DRAM_H__

#include <plat/inc/autoconf.h>

typedef struct dram_para
{
	//normal configuration
	unsigned int        dram_clk;
	unsigned int        dram_type;      //dram_type         DDR2: 2             DDR3: 3     LPDDR2: 6   LPDDR3: 7   DDR3L: 31
	//unsigned int        lpddr2_type;  //LPDDR2 type       S4:0    S2:1    NVM:2
	unsigned int        dram_zq;        //do not need
	unsigned int        dram_odt_en;

	//control configuration
	unsigned int        dram_para1;
	unsigned int        dram_para2;

	//timing configuration
	unsigned int        dram_mr0;
	unsigned int        dram_mr1;
	unsigned int        dram_mr2;
	unsigned int        dram_mr3;
	unsigned int        dram_tpr0;  //DRAMTMG0
	unsigned int        dram_tpr1;  //DRAMTMG1
	unsigned int        dram_tpr2;  //DRAMTMG2
	unsigned int        dram_tpr3;  //DRAMTMG3
	unsigned int        dram_tpr4;  //DRAMTMG4
	unsigned int        dram_tpr5;  //DRAMTMG5
	unsigned int        dram_tpr6;  //DRAMTMG8

	//reserved for future use
	unsigned int        dram_tpr7;
	unsigned int        dram_tpr8;
	unsigned int        dram_tpr9;
	unsigned int        dram_tpr10;
	unsigned int        dram_tpr11;
	unsigned int        dram_tpr12;
	unsigned int        dram_tpr13;
}dram_para_t;

typedef struct dram_data
{
	unsigned int        dram_data0;
	unsigned int        dram_data1;
	unsigned int        dram_data2;
	unsigned int        dram_data3;
	unsigned int        dram_data4;
	unsigned int        dram_data5;
	unsigned int        dram_data6;
	unsigned int        dram_data7;
	unsigned int        dram_data8;
	unsigned int        dram_data9;
	unsigned int        dram_data10;
	unsigned int        dram_data11;
	unsigned int        dram_data12;
	unsigned int        dram_data13;
	unsigned int        dram_data14;
	unsigned int        dram_data15;
	unsigned int 	    data_temp[48];		//for future use
}dram_data_t;

#if DRAM_USED
unsigned int dram_power_save_process(void);
unsigned int dram_power_up_process(void);
void dram_enable_all_master(void);
void dram_disable_all_master(void);
void dram_master_enable_cpus(void);
#else
#define dram_power_save_process() {};
#define dram_power_up_process() {};
#define dram_enable_all_master() {};
#define dram_disable_all_master() {};
#define dram_master_enable() {};
#endif
#define DELAY_FOR_DRAM
#define dram_enter_suspend() dram_power_save_process()
#define dram_exit_suspend() dram_power_up_process()
unsigned int ccm_set_pll_ddr1_clk_new_mode(void);
int dram_config_paras(u32 index, u32 len, u32 *data);


int dram_config_paras(u32 index, u32 len, u32 *data);
unsigned int dram_idle_enter_process(void);
unsigned int dram_idle_exit_process(void);

#endif  //__DRAM_H__
