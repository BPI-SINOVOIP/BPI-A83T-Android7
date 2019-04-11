#ifndef   _MCTL_HAL_H
#define   _MCTL_HAL_H
/**********************
验证平台宏定义
1.FPGA_VERIFY --- FPGA验证打开
2.IC_VERIFY --- IC验证打开
3.SYSTEM_VERIFY --- 驱动使用打开
**********************/
//#define FPGA_VERIFY
#define IC_VERIFY
//#define SYSTEM_VERIFY

/**********************
DRAM类型定义
**********************/
#ifdef	IC_VERIFY
//#define	DRAM_TYPE_DDR3
//#define	DRAM_TYPE_LPDDR2
//#define DRAM_TYPE_LPDDR3
//#define DRAM_TYPE_DDR2
//#define DRAM_TYPE_AUTO
#endif
/**********************
STANDBY宏定义
standby唤醒不走boot流程
的可以使能该宏减少编译量
**********************/
//#define CPUS_STANDBY_CODE

/**********************
功能块宏定义
**********************/
#ifndef CPUS_STANDBY_CODE
//#define USE_PMU
//#define USE_CHIPID
//#define USE_SPECIAL_DRAM
//#define USE_CHEAP_DRAM
//#define IC_VERIFY_TEST
#endif

/**********************
自动配置宏定义
**********************/
#ifndef CPUS_STANDBY_CODE
#define DRAM_AUTO_SCAN

#ifdef DRAM_AUTO_SCAN
#define DRAM_TYPE_SCAN
#define DRAM_RANK_SCAN
#define DRAM_SIZE_SCAN
#endif
#endif
/**********************
打印等级宏定义
**********************/
//#define DEBUG_LEVEL_1
//#define DEBUG_LEVEL_4
//#define DEBUG_LEVEL_8
//#define TIMING_DEBUG
#define ERROR_DEBUG
//#define AUTO_DEBUG

#if defined DEBUG_LEVEL_8
#define dram_dbg_8(fmt,args...)	 printk(fmt ,##args)
#define dram_dbg_4(fmt,args...)	 printk(fmt ,##args)
#define dram_dbg_0(fmt,args...)  printk(fmt ,##args)
#elif defined DEBUG_LEVEL_4
#define dram_dbg_8(fmt,args...)
#define dram_dbg_4(fmt,args...)  printk(fmt ,##args)
#define dram_dbg_0(fmt,args...)  printk(fmt ,##args)
#elif defined DEBUG_LEVEL_1
#define dram_dbg_8(fmt,args...)
#define dram_dbg_4(fmt,args...)
#define dram_dbg_0(fmt,args...)  printk(fmt ,##args)
#else
#define dram_dbg_8(fmt,args...)
#define dram_dbg_4(fmt,args...)
#define dram_dbg_0(fmt,args...)
#endif

#if defined TIMING_DEBUG
#define dram_dbg_timing(fmt,args...)  printk(fmt ,##args)
#else
#define dram_dbg_timing(fmt,args...)
#endif

#if defined ERROR_DEBUG
#define dram_dbg_error(fmt,args...)  printk(fmt ,##args)
#else
#define dram_dbg_error(fmt,args...)
#endif


#if defined AUTO_DEBUG
#define dram_dbg_auto(fmt,args...)  printk(fmt ,##args)
#else
#define dram_dbg_auto(fmt,args...)
#endif

typedef struct __DRAM_TPR13
{
	unsigned	dram_size_auto_det	:	1;		//bit 0
	unsigned	timing_auto_det		:	1;		//bit 1
	unsigned	dqs_gate_mode		:	2;		//bit 3:2
	unsigned	type_auto_det_mode	:	1;		//bit 4
	unsigned	enable_2t_mode		:	1;		//bit 5
	unsigned	pll_src_sel			:	1;		//bit 6
	unsigned	dbg_0				:	1;		//bit 7
	unsigned	vtf_enable			:	1;		//bit 8
	unsigned	dynamic_clk			:	1;		//bit 9
	unsigned	mdfs_mode			:	1;		//bit 10
	unsigned	mdfs_enable			:	1;		//bit 11
	unsigned	mdfs_odt_mode		:	1;		//bit 12
	unsigned	dramtyp_det_enable	:	1;		//bit 13
	unsigned	rnk_wth_auto_det	:	1;		//bit 14
	unsigned	boot_auto_det		:	1;		//bit 15
	unsigned	cfs_ssc_step		:	4;		//bit 19:16
	unsigned	ssc_factor			:	3;		//bit 22:20
	unsigned	rfu_00				:	3;		//bit 25:23
	unsigned	dq_hold_disable		:	1;		//bit 26
	unsigned	mst_prio_disable	:	1;		//bit 27
	unsigned	rfu_1				:	4;		//bit 28:31
}__dram_tpr13_t;

extern unsigned int mctl_core_init(dram_para_t *para);
extern unsigned int mctl_init(void);
extern signed int init_DRAM(int type, dram_para_t *para);
#endif  //_MCTL_HAL_H
