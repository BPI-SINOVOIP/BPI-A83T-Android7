/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                DVFS module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : dvfs.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: DVFS module public header.
* Update  : date                auther      ver     notes
*           2012-5-13 15:23:44  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __DVFS_H__
#define __DVFS_H__
#if DVFS_USED

//the freq and voltage mapping value
typedef struct freq_voltage
{
	u32 freq;       //cpu frequency
	u32 voltage;    //voltage for the frequency
	u32 axi_div;    //the divide ratio of AXI bus
} freq_voltage_t;

/*
*********************************************************************************************************
*                                       INIT DVFS
*
* Description:  initialize dvfs module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_init(void);

/*
*********************************************************************************************************
*                                       EXIT DVFS
*
* Description:  exit dvfs module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_exit(void);

/*
*********************************************************************************************************
*                                       SET FREQ
*
* Description:  set the frequency of PLL1 module.
*
* Arguments  :  freq    : the frequency which we want to set.
*
* Returns    :  OK if set frequency succeeded, others if failed.
*********************************************************************************************************
*/
s32 dvfs_set_freq(struct message *pmessage);
s32 dvfs_config_vf_table(struct message *pmessage);

#else
static inline s32 dvfs_init(void) { return 0; }
static inline s32 dvfs_exit(void) { return 0; }
#endif /* DVFS_USED */
#endif  //__DVFS_H__
