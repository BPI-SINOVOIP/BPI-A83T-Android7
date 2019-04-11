/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                               standby module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : standby.h
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: standby module public header.
* Update  : date                auther      ver     notes
*           2012-5-13 15:32:01  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef __STANDBY_H__
#define __STANDBY_H__

#include <plat/inc/include.h>
/*
* the wakeup source of main cpu: cpu0
*/
#define CPU0_WAKEUP_MSGBOX      (1<<0)  /* external interrupt, pmu event for ex.    */
#define CPU0_WAKEUP_KEY         (1<<1)  /* key event    */
#define CPU0_WAKEUP_EXINT       (1<<2)
#define CPU0_WAKEUP_IR          (1<<3)
#define CPU0_WAKEUP_ALARM       (1<<4)
#define CPU0_WAKEUP_USB         (1<<5)
#define CPU0_WAKEUP_TIMEOUT     (1<<6)
#define CPU0_WAKEUP_PIO         (1<<7)


//the wakeup source of assistant cpu: cpus
#define CPUS_WAKEUP_LOWBATT     (1<<12)
#define CPUS_WAKEUP_USB         (1<<13)
#define CPUS_WAKEUP_AC          (1<<14)
#define CPUS_WAKEUP_ASCEND      (1<<15)
#define CPUS_WAKEUP_DESCEND     (1<<16)
#define CPUS_WAKEUP_SHORT_KEY   (1<<17)
#define CPUS_WAKEUP_LONG_KEY    (1<<18)
#define CPUS_WAKEUP_IR          (1<<19)
#define CPUS_WAKEUP_ALM0        (1<<20)
#define CPUS_WAKEUP_ALM1        (1<<21)
#define CPUS_WAKEUP_TIMEOUT     (1<<22)
#define CPUS_WAKEUP_GPIO        (1<<23)
#define CPUS_WAKEUP_USBMOUSE    (1<<24)
#define CPUS_WAKEUP_LRADC       (1<<25)
//null for 1<<26
#define CPUS_WAKEUP_CODEC       (1<<27)
#define CPUS_WAKEUP_BAT_TEMP    (1<<28)
#define CPUS_WAKEUP_FULLBATT    (1<<29)
#define CPUS_WAKEUP_HMIC        (1<<30)
#define CPUS_WAKEUP_POWER_EXP   (1<<31)
#define CPUS_WAKEUP_KEY         (CPUS_WAKEUP_SHORT_KEY | CPUS_WAKEUP_LONG_KEY)

//macro for axp wakeup source event
#define CPUS_WAKEUP_NMI     (CPUS_WAKEUP_LOWBATT   | \
                             CPUS_WAKEUP_USB       | \
                             CPUS_WAKEUP_AC        | \
                             CPUS_WAKEUP_ASCEND    | \
                             CPUS_WAKEUP_DESCEND   | \
                             CPUS_WAKEUP_SHORT_KEY | \
                             CPUS_WAKEUP_LONG_KEY  | \
                             CPUS_WAKEUP_BAT_TEMP  | \
                             CPUS_WAKEUP_FULLBATT)

//for format all the wakeup gpio into one word.
#define GPIO_PL_MAX_NUM		    (11)    //0-11
#define GPIO_PM_MAX_NUM		    (11)    //0-11
#define GPIO_AXP_MAX_NUM	    (7)	    //0-7
#define GPIO_PL_MASK            0x00000fff
#define GPIO_PM_MASK            0x00fff000
#define GPIO_AXP_MASK           0x03000000
#define CPUS_GPIO_PL(num)       (1 << num)
#define CPUS_GPIO_PM(num)       (1 << (num + 12))
#define CPUS_GPIO_AXP(num)      (1 << (num + 24))
//#define WAKEUP_GPIO_GROUP(num)  (1 << (num))
#define WAKEUP_GPIO_GROUP(group)  (1 << (group - 'A'))

/*
 * extended standby.
 */
#if (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
#define PLL_NUM     (11)
#define BUS_NUM     (8)
#define IO_NUM      (2)
#elif (defined CONFIG_ARCH_SUN50IW1P1)
#define PLL_NUM     (14)
#define BUS_NUM     (8)
#define IO_NUM      (2)
#elif (defined CONFIG_ARCH_SUN50IW2P1)
#define PLL_NUM     (14)
#define BUS_NUM     (8)
#define IO_NUM      (2)
#elif (defined CONFIG_ARCH_SUN50IW3P1)
#define PLL_NUM     (14)
#define BUS_NUM     (10)
#define IO_NUM      (2)
#elif (defined CONFIG_ARCH_SUN50IW6P1)
#define PLL_NUM     (14)
#define BUS_NUM     (10)
#define IO_NUM      (2)
#else
#define PLL_NUM     (11)
#define BUS_NUM     (6)
#define IO_NUM      (2)
#endif

//for bitmap macro definition
#define PLL_C0      (0)
#define PLL_C1      (1)
#define PLL_AUDIO   (2)
#define PLL_VIDEO0  (3)
#define PLL_VE      (4)
#define PLL_DRAM    (5)
#define PLL_PERIPH  (6)
#define PLL_GPU	    (7)
#define PLL_HSIC    (8)
#define PLL_DE	    (9)
#define PLL_VIDEO1  (10)
#define PLL_PERIPH1 (11)
#define PLL_DRAM1   (12)
#define PLL_MIPI    (13)

#define BUS_C0      (0)
#define BUS_C1      (1)
#define BUS_AXI0    (2)
#define BUS_AXI1    (3)
#define BUS_AHB1    (4)
#define BUS_AHB2    (5)
#define BUS_APB1    (6)
#define BUS_APB2    (7)
#define BUS_AHB3    (8)
#define BUS_PSI     (9)

#define OSC_LDO0    (0)
#define OSC_LDO1    (1)
#define OSC_LOSC    (2)
#define OSC_HOSC    (3)

/* FIXME: if you modify this struct, you should
 * sync this change with linux source,
 * by superm at 2015-05-15.
 */
typedef enum power_dm
{
	DM_CPUA = 0, /* 0  */
	DM_CPUB,     /* 1  */
	DM_DRAM,     /* 2  */
	DM_GPU,      /* 3  */
	DM_SYS,      /* 4  */
	DM_VPU,      /* 5  */
	DM_CPUS,     /* 6  */
	DM_DRAMPLL,  /* 7  */
	DM_ADC,      /* 8  */
	DM_PL,       /* 9  */
	DM_PM,       /* 10 */
	DM_IO,       /* 11 */
	DM_CPVDD,    /* 12 */
	DM_LDOIN,    /* 13 */
	DM_PLL,      /* 14 */
	DM_LPDDR,    /* 15 */
	DM_TEST,     /* 16 */
	DM_RES1,     /* 17 */
	DM_RES2,     /* 18 */
	DM_RES3,     /* 19 */
	DM_MAX,      /* 20 */
} power_dm_e;

#define A33_EX_STANDBY_PWR_DM_MASK (0x206d)
#define A80_EX_STANDBY_PWR_DM_MASK (0x31f143b)  //need add interface to translate this bit to corresponding power;
#define A83_EX_STANDBY_PWR_DM_MASK (0x7fd5)  //need add interface to translate this bit to corresponding power;

/* platform public used */
#if (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
typedef struct pll_para{
	unsigned int factor1;
	unsigned int factor2;
	unsigned int factor3;
	unsigned int factor4;
}pll_para_t;
#else
typedef struct pll_para{
	unsigned int n;
	unsigned int k;
	unsigned int m;
	unsigned int p;
}pll_para_t;
#endif

typedef struct bus_para{
	int src;
	int pre_div;
	int div_ratio;
	int n;
	int m;
}bus_para_t;


/* platform private used */
#if (defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
typedef struct pwr_dm_state{
    /*
     * for state bitmap:
     * bitx = 1: do not care.
     * bitx = 0: mean close corresponding power src.
     */
    unsigned int state;
    unsigned int sys_mask;	//bitx=1, the corresponding state is effect,
				//otherwise, the corresponding power is in charge in device driver.

    // sys_mask&state		: bitx=1, mean the power is on, for the "on" state power, u need care about the voltage.;
    // ((~sys_mask)|state)		: bitx=0, mean the power is close;
    // pwr_dm_state bitmap

    // actually: we care about the pwr_dm voltage,
    // such as: we want to keep the vdd_sys at 1.0v at standby period.
    //		we actually do not care how to do it.
    //		it can be sure that cpus can do it with the pmu's help.
    unsigned int volt[DM_MAX];
}pwr_dm_state_t;

typedef struct dram_state{
    unsigned int selfresh_flag; //selfresh_flag must be compatible with vdd_sys pwr state.
    unsigned int crc_en;
    unsigned int crc_start;
    unsigned int crc_len;
}dram_state_t;

typedef struct cpus_clk_para{
    unsigned int cpus_id;
}cpus_para_t;

typedef struct io_state_config{
    unsigned int paddr;
    unsigned int value_mask; //specify the effect bit.
    unsigned int value;
}io_state_config_t;

typedef struct soc_io_para{
    /*
     *	hold: mean before power off vdd_sys, whether hold gpio pad or not.
     *	this flag only effect: when vdd_sys is powered_off;
     *	this flag only affect hold&unhold operation.
     *	the recommended hold sequence is as follow:
     *	backup_io_cfg -> cfg_io(enter_low_power_mode) -> hold -> assert vdd_sys_reset -> poweroff vdd_sys.
     *  the recommended unhold sequence is as follow:
     *  poweron vdd_sys -> de_assert vdd_sys -> restore_io_cfg -> unhold.
     * */
    unsigned int hold_flag;


    /*
     * note: only specific bit mark by value_mask is effect.
     * IO_NUM: only uart and jtag needed care.
     * */
    io_state_config_t io_state[IO_NUM];
}soc_io_para_t;

typedef struct cpux_clk_para{
    /*
     * Hosc: losc: ldo: ldo1
     * the osc bit map is as follow:
     * bit3: bit2: bit1:bit0
     * Hosc: losc: ldo: ldo1
     */
    int osc_en;

    /*
     * for a83, pll bitmap as follow:
     * bit7:	    bit6:	bit5:	    bit4:	bit3:		bit2:		bit1:	    bit0
     * pll8(gpu): pll6(periph): pll5(dram): pll4(ve): pll3(video):	pll2(audio):	c1cpux:	    c0cpux
     *									pll11(video1):	pll10(de):  pll9(hsic)
     * */

    /* for disable bitmap:
     * bitx = 0: close
     * bitx = 1: do not care.
     */
    int init_pll_dis;

    /* for enable bitmap:
     * bitx = 0: do not care.
     * bitx = 1: open
     */
    int exit_pll_en;

    /*
     * set corresponding bit if it's pll factors need to be set some value.
     */
    int pll_change;

    /*
     * fill in the enabled pll freq factor sequently. unit is khz pll6: 0x90041811
     * factor n/m/k/p already do the pretreatment of the minus one
     */
    pll_para_t pll_factor[PLL_NUM];

    /*
     * **********A31************
     * at a31 platform, the clk relationship is as follow:
     * pll1->cpu -> axi
     *	     -> atb/apb
     * ahb1 -> apb1
     * apb2
     * so, at a31, only cpu:ahb1:apb2 need be cared.
     *
     * *********A80************
     * at a83 platform, the clk relationship is as follow:
     * c1 -> axi1
     * c0 -> axi0
     * gtbus
     * ahb0
     * ahb1
     * ahb2
     * apb0
     * apb1
     * so, at a80, c1:c0:gtbus:ahb0:ahb1:ahb2:apb0:apb1 need be cared.
     *
     * *********A83************
     * at a83 platform, the clk relationship is as follow:
     * c1 -> axi1
     * c0 -> axi0
     * ahb1 -> apb1
     * apb2
     * ahb2
     * so, at a83, only c1:c0:ahb1:apb2:ahb2 need be cared.
     *
     * normally, only clk src need be cared.
     * the bus bitmap as follow:
     * for a83, bus_en:
     * bit4: bit3: bit2: bit1: bit0
     * ahb2: apb2: ahb1: c1:   c0
     *
     * for a31, bus_en:
     * bit2:bit1:bit0
     * cpu:ahb1:apb2
     *
     * for a80, bus_en:
     * bit7: bit6:  bit5:  bit4: bit3: bit2: bit1: bit0
     * c1:   c0:    gtbus: ahb0: ahb1: ahb2: apb0: apb1
     */
    int bus_change;

    /*
     * bus_src: ahb1, apb2 src;
     * option:  pllx:axi:hosc:losc
     */
    bus_para_t bus_factor[BUS_NUM];

}cpux_clk_para_t;

typedef struct soc_pwr_dep{
    /*
     * id of extended standby
     */
    unsigned int id;

    pwr_dm_state_t soc_pwr_dm_state;
    dram_state_t soc_dram_state;
    soc_io_para_t soc_io_state;
    cpux_clk_para_t cpux_clk_state;

}soc_pwr_dep_t;

typedef struct extended_standby{
    /*
     * id of extended standby
     */
    unsigned int id;
    unsigned int pmu_id; //for: 808 || 809+806 || 803 || 813
    unsigned int soc_id;	 // a33, a80, a83,...,
				 // for compatible reason, different soc, each bit have different meaning.

    pwr_dm_state_t soc_pwr_dm_state;
    dram_state_t soc_dram_state;
    soc_io_para_t soc_io_state;
    cpux_clk_para_t cpux_clk_state;
}extended_standby_t;

#else // ! CONFIG_ARCH_SUN8IW6P1/W9
typedef struct extended_standby{
	/*
	 * id of extended standby
	 */
	int id;
	/*
	 * clk tree para description as follow:
	 * avcc : vcc_wifi : vcc_dram: vdd_sys : vdd_cpux : vdd_gpu : vcc_io : vdd_cpus
	 */
	int pwr_dm_en;  //bitx = 1, mean power on when sys is in standby state. otherwise, vice verse.

	/*
	 * Hosc: losc: ldo: ldo1
	 */
	int osc_en;

	/*
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int init_pll_dis;

	/*
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int exit_pll_en;

	/*
	 * set corresponding bit if it's pll factors need to be set some value.
	 * pll_10: pll_9: pll_mipi: pll_8: pll_7: pll_6: pll_5: pll_4: pll_3: pll_2: pll_1
	 */
	int pll_change;

	/*
	 * fill in the enabled pll freq factor sequently. unit is khz pll6: 0x90041811
	 * factor n/m/k/p already do the pretreatment of the minus one
	 */
	pll_para_t pll_factor[PLL_NUM];

	/*
	 * bus_en: cpu:axi:atb/apb:ahb1:apb1:apb2,
	 * normally, only clk src need be cared.
	 * so, at a31, only cpu:ahb1:apb2 need be cared.
	 * pll1->cpu -> axi
	 *       -> atb/apb
	 * ahb1 -> apb1
	 * apb2
	 */
	int bus_change;

	/*
	 * bus_src: ahb1, apb2 src;
	 * option:  pllx:axi:hosc:losc
	 */
	bus_para_t bus_factor[BUS_NUM];
}extended_standby_t;
#endif

typedef struct super_standby_para
{
	unsigned int event;                //cpus wakeup event types
	unsigned int resume_code_src;      //cpux resume code src
	unsigned int resume_code_length;   //cpux resume code length
	unsigned int resume_entry;         //cpux resume entry
	unsigned int timeout;              //wakeup after timeout seconds
	unsigned int gpio_enable_bitmap;
	unsigned int cpux_gpiog_bitmap;    //cpux gpio group bitmap
	extended_standby_t *pextended_standby; //extended standby config struct address
} super_standby_para_t;

typedef struct normal_standby_para
{
	unsigned int event;                 //cpus wakeup event types
	unsigned int timeout;               //wakeup after timeout seconds
	unsigned int gpio_enable_bitmap;
	unsigned int cpux_gpiog_bitmap;    //cpux gpio group bitmap
	extended_standby_t *pextended_standby; //extended standby config struct address
} normal_standby_para_t;

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1)
typedef struct pll_reg
{
	//unsigned int pll1;  //pll1 already restore
	unsigned int pll2;
	unsigned int pll3;
	unsigned int pll4;
	//unsigned int pll5;  //pll5 restore by dram driver
	unsigned int pll6;
	unsigned int pll7;
	unsigned int pll8;
	unsigned int pllmipi;
	unsigned int pll9;
	unsigned int pll10;
} pll_reg_t;
#elif (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
typedef struct pll_reg
{
	//unsigned int pll1;  //pll1 already restore
	//unsigned int pll2;  //pll2 already restore
	unsigned int pll3;
	unsigned int pll4;
	unsigned int pll5;
	//unsigned int pll6; //pll6 restore by dram driver
	unsigned int pll7;
	unsigned int pll8;
	unsigned int pll9;
	unsigned int pll10;
	unsigned int pll11;
} pll_reg_t;
#elif (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
typedef struct pll_reg
{
	//unsigned int pll1;  //pll1 already restore
	unsigned int pll2;
	unsigned int pll3;
	unsigned int pll4;
	//unsigned int pll5; //pll5 restore by dram driver
	unsigned int pll6;
	unsigned int pll7;
	unsigned int pll8;
	unsigned int pll9;
	unsigned int pll10;
	unsigned int pll11;
	unsigned int pll12;
	//unsigned int pll13; //pll13 restore by dram driver
} pll_reg_t;
#endif

/**
*@brief struct of pmu device arg
*/
struct aw_pmu_arg{
    unsigned int  twi_port;		/**<twi port for pmu chip   */
    unsigned char dev_addr;		/**<address of pmu device   */
};


/**
*@brief struct of standby
*/
struct aw_standby_para{
	unsigned int event;		/**<event type for system wakeup    */
	unsigned int axp_event;		/**<axp event type for system wakeup    */
	unsigned int debug_mask;	/* debug mask */
	signed int   timeout;		/**<time to power off system from now, based on second */
	unsigned int gpio_enable_bitmap;
};


/**
*@brief struct of power management info
*/
struct aw_pm_info{
    struct aw_standby_para		standby_para;   /* standby parameter            */
    struct aw_pmu_arg			pmu_arg;        /**<args used by main function  */
};

#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1)
#define CPUS_ENABLE_POWER_EXP   (1<<31)
#define CPUS_WAKEUP_POWER_STA   (1<<1 )
#define CPUS_WAKEUP_POWER_CSM   (1<<0 )

typedef enum arisc_rw_type
{
	ARISC_READ = 0x0,
	ARISC_WRITE = 0x1,
} arisc_rw_type_e;

typedef struct sst_power_info_para
{
	/*
	 * power_reg0 ~ 7 AXP_main REG10,  8~15 AXP_main REG12
	 * power_reg16~32 null
	 *
	 * AXP_main REG10: 0-off, 1-on
	 * bit7   bit6	 bit5   bit4   bit3   bit2   bit1   bit0
	 * aldo2  aldo1  dcdc5  dcdc4  dcdc3  dcdc2  dcdc1  dc5ldo
	 *
	 * REG12: 0-off, 1-on
	 * bit7   bit6   bit5   bit4   bit3   bit2   bit1   bit0
	 * dc1sw  dldo4  dldo3  dldo2  dldo1  eldo3  eldo2  eldo1
	 */

	unsigned int enable;		/* enable bit */
	unsigned int power_reg;		/* registers of power state should be */
	signed int system_power;	/* the max power of system, signed, power mabe negative when charge */
} sst_power_info_para_t;

typedef struct sst_dram_info
{
	unsigned int dram_crc_enable;
	unsigned int *dram_crc_src;
	unsigned int dram_crc_len;
	unsigned int dram_crc_error;
	unsigned int dram_crc_total_count;
	unsigned int dram_crc_error_count;
} sst_dram_info_t;

typedef struct standby_info_para
{
	sst_power_info_para_t power_state; /* size 3W=12B */
	sst_dram_info_t dram_state; /*size 6W=24B */
} standby_info_para_t;

extern s32 sstandby_query_info(struct message *pmsg);
extern int long_jump(int (*fn)(void *arg), void *arg);
#endif

#if STANDBY_USED

/*
*********************************************************************************************************
*                                       INIT STANDBY
*
* Description:  initialize standby module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*********************************************************************************************************
*/
s32 standby_init(void);

/*
*********************************************************************************************************
*                                       EXIT STANDBY
*
* Description:  exit standby module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*********************************************************************************************************
*/
s32 super_exit(void);

/*
*********************************************************************************************************
*                                       ENTEY OF SUPER-STANDBY
*
* Description:  the entry of super-standby.
*
* Arguments  :  request :   the request message.
*
* Returns    :  OK if enter super-standby succeeded, others if failed.
*********************************************************************************************************
*/
s32 super_standby_entry(struct message *request);

/*
*********************************************************************************************************
*                                       ENTEY OF NORMAL-STANDBY
*
* Description:  the entry of normal-standby.
*
* Arguments  :  request :   the request message.
*
* Returns    :  OK if enter normal-standby succeeded, others if failed.
*********************************************************************************************************
*/
s32 normal_standby_entry(struct message *request);

/*
*********************************************************************************************************
*                                       ENTEY OF SETING DRAM CRC PARAS
*
* Description:  set ar100 debug dram crc paras.
*
* Arguments  :  en      :  dram crc enable or disable;
*               srcaddr :  source address of dram crc area
*               len     :  lenght of dram crc area
*
* Returns    :  OK if set ar100 debug dram crc paras successed, others if failed.
*********************************************************************************************************
*/
s32 standby_set_dram_crc_paras(u32 enable, u32 src, u32 len);

s32 fake_power_off_entry(struct message *request);

/*
*********************************************************************************************************
*                                       ENTEY OF EXTENDED-STANDBY
*
* Description:  the entry of extended super-standby.
*
* Arguments  :  request:request command message.
*
* Returns    :  OK if enter extended super-standby succeeded, others if failed.
*********************************************************************************************************
*/
s32 extended_super_standby_entry(struct message *request);

s32 talk_standby_entry(struct message *request);
#else
static inline s32 standby_init(void) { return 0; }
static inline s32 super_exit(void) { return 0; }
#endif /* STANDBY_USED */

#if CPUOP_USED
int cpu_op(struct message *pmessage);
#else
static inline s32 cpu_op(void) { return 0; }
#endif

#if SYSOP_USED
int sys_op(struct message *pmessage);
#else
static inline s32 sys_op(void) { return 0; }
#endif

#endif  //__STANDBY_H__
