/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                daemon module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : daemon.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-13
* Descript: daemon module.
* Update  : date                auther      ver     notes
*           2012-5-13 15:06:10  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "daemon_i.h"

//the list of daemon notifier
static struct notifier *daemon_list = NULL;
u32 ar100_code_addr;

int daemon_register_service(__pNotifier_t pcb)
{
	return notifier_insert(&daemon_list, pcb);
}

#if ((defined KERNEL_USED) || (defined TF_USED))
static s32 startup_state_notify(s32 result)
{
	struct message *pmessage;

	//allocate a message frame
	pmessage = message_allocate();
	if (pmessage == NULL)
	{
		WRN("allocate message for init fb failed\n");
		return -EFAULT;
	}
	LOG("feedback startup result [%d]\n", result);

	//initialize message
	pmessage->type     = AR100_STARTUP_NOTIFY;
	pmessage->attr     = MESSAGE_ATTR_HARDSYN;
	pmessage->private  = 0;
	pmessage->result   = result;
	pmessage->paras[0] = AR100_VERSIONS;
	//strcpy((char *)&(pmessage->paras[1]), SUB_VER); //FIXME
	strcpy((char *)&(pmessage->paras[1]), __DATE__); //FIXME
	hwmsgbox_send_message(pmessage, SEND_MSG_TIMEOUT);
	ar100_code_addr = pmessage->paras[0];

	message_free(pmessage);

	return OK;
}
#endif /* ((defined KERNEL_USED) || (defined TF_USED)) */

//system daemon vars
//extern u32 debug_level;

static void daemon_main(void)
{
	volatile int i;
#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
#if PMU_CHRCUR_CRTL_USED
	int bat_delay = 0;
#endif
#endif

#if AR100_TEST_EN
	//ar100 enter self test mode
	ar100_test();
#else
	//initialize cpu
	//cpu_init();
	cpuIntsOn();

	return;

#ifdef BOOT_USED
	while (1) {
		//LOG("cpus enter WFI now\n");
		for (i = 0; i < 500; i++)
			time_mdelay(10);//__asm volatile("WFI");
	}
#endif

	//add timer
	led_bln_init();

	//daemon loop process
	LOG("daemon service setup...\n");
	//lcd_screen_size_check();
	while (1) {
#ifndef FPGA_PLATFORM
		//process system daemon list
		if (((current_time_tick()) % DAEMON_ONCE_TICKS) == 0)
		{
			//daemon run one time
			printk("------------------------------\n");
			LOG("system tick:%d\n", DAEMON_ONCE_TICKS);
			//LOG("debug_mask:%d\n", debug_level);
			LOG("uart_buadrate:%d\n", uart_get_baudrate());
			notifier_notify(&daemon_list, DAEMON_RUN_NOTIFY, 0);
			//lcd_screen_size_check();
#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
#if PMU_CHRCUR_CRTL_USED
			if (++bat_delay > CHRCUR_CRTL_PER_SECONDS/10) {
				pmu_contrl_batchrcur();
				bat_delay = 0;
			}
#endif

			/* delay wait another tick */
			for (i = 0; i < 200000; i++)
				asm volatile ("nop");

#endif
		}

		/* delay wait another tick */
		for (i = 0; i < 10000; i++)
			asm volatile ("nop");
#else //FPGA_PLATFORM
			//daemon run one time
			//printk("\n------------------------------\n");
			//LOG("system tick:%d\n", DAEMON_ONCE_TICKS);
			//LOG("debug_mask:%d\n", debug_level);
			//LOG("uart_buadrate:%d\n", uart_get_baudrate());
			//notifier_notify(&daemon_list, DAEMON_RUN_NOTIFY, 0);
			for (i = 0; i < 6000; i++) //delay 6s = 6000ms
			{
				time_udelay(1000000); //1ms = 1000us
			}
#endif //FPGA_PLATFORM
		//maybe others
		//...
	}
#endif  //AR100_TEST_EN
}

/*
*********************************************************************************************************
*                                       STARTUP ENTRY
*
* Description:  the entry of startup.
*
* Arguments  :  none.
*
* Returns    :  none.
*********************************************************************************************************
*/
void startup_entry(void)
{
#ifdef BOOT_USED
	int val;
#endif
	//initialize notifier manager
	notifier_init();
	save_state_flag(REC_HOTPULG | 0x0);
	//initialize ccu driver
	ccu_init();
	save_state_flag(REC_HOTPULG | 0x1);

	//initialize pin driver
	pin_init();
	save_state_flag(REC_HOTPULG | 0x2);

	//initialize hwspinlock driver
	hwspinlock_init();
	save_state_flag(REC_HOTPULG | 0x3);

	//initialize interrupt driver
	interrupt_init();
	save_state_flag(REC_HOTPULG | 0x4);

	extiInit();

	/* get config by kernel */
	arisc_para_init();
	save_state_flag(REC_HOTPULG | 0x5);

	//initialize debugger system
	uart_init();
	printk("ARISC UP \n");
	save_state_flag(REC_HOTPULG | 0x6);
	LOG("debugger system ok\n");

	//initialize dram
	//dram_init();
	//LOG("dram driver ok\n");

#if RSB_USED
	//initialize rsb driver
	rsb_init();
	LOG("rsb driver ok\n");
#elif P2WI_USED
	//initialize p2wi driver
	p2wi_init();
	LOG("p2wi driver ok\n");
#else
	//initialize twi driver
	twi_init();
	save_state_flag(REC_HOTPULG | 0x7);
	LOG("twi driver ok\n");
#endif  //RSB_USED

#if CPUS_JTAG_DEBUG_EN
	//config ar100 jtag pins
	pin_set_multi_sel(PIN_GRP_PL, 8, 3);
	pin_set_pull     (PIN_GRP_PL, 8, PIN_PULL_UP);
	pin_set_drive    (PIN_GRP_PL, 8, PIN_MULTI_DRIVE_2);
	pin_set_multi_sel(PIN_GRP_PL, 7, 3);
	pin_set_pull     (PIN_GRP_PL, 7, PIN_PULL_UP);
	pin_set_drive    (PIN_GRP_PL, 7, PIN_MULTI_DRIVE_2);
	pin_set_multi_sel(PIN_GRP_PL, 6, 3);
	pin_set_pull     (PIN_GRP_PL, 6, PIN_PULL_UP);
	pin_set_drive    (PIN_GRP_PL, 6, PIN_MULTI_DRIVE_2);
	pin_set_multi_sel(PIN_GRP_PL, 5, 3);
	pin_set_pull     (PIN_GRP_PL, 5, PIN_PULL_UP);
	pin_set_drive    (PIN_GRP_PL, 5, PIN_MULTI_DRIVE_2);
#endif

	//initialize pmu driver
	pmu_init();
	save_state_flag(REC_HOTPULG | 0x8);
	LOG("pmu driver ok\n");

	//initialize hwmsgbox driver
	hwmsgbox_init();
	save_state_flag(REC_HOTPULG | 0x9);
	LOG("hwmsgbox driver ok\n");

	//initialize cpucfg driver
	//cpucfg_init();
	save_state_flag(REC_HOTPULG | 0xa);
	LOG("cpucfg driver ok\n");

	//initialize message manager
	message_manager_init();
	LOG("message manager ok\n");

	//initialize timer driver
	timer_init();
	LOG("timer driver ok\n");

	//systimer_init();
        //printk("systimer init ok\n");

	//initialize dvfs service
	dvfs_init();
	LOG("dvfs service driver ok\n");

	//initialize standby service
	standby_init();
	LOG("standby service ok\n");

	/* cpuidle init */
	cpuidle_init();

	save_state_flag(REC_HOTPULG | 0xb);
#ifndef FPGA_PLATFORM
	//initialize system time ticks
	//time_ticks_init();
	//LOG("time ticks ok\n");
#endif
	//initialize watchdog
	watchdog_init();
	LOG("watchdog ok\n");
#if THERMAL_USED
	thermal_init();
	LOG("thermal ok\n");
#endif

#if CHECK_USED
	check_board();
#endif

#if (defined KERNEL_USED)
	//feedback the startup state to ac327
	startup_state_notify(OK);
	LOG("startup feedback ok\n");
#elif (defined TF_USED)
	//feedback the startup state to ac327
	save_state_flag(REC_HOTPULG | 0xc);
	printk("notify \n");

	startup_state_notify(OK);
	printk("set paras \n");
	set_paras();
	LOG("startup feedback ok\n");
#endif

	printk("notify over \n");
	save_state_flag(REC_HOTPULG | 0xd);

	printk("ar400 version : %d\n",AR100_VERSIONS);
	LOG("ar100 firmware version : %d\n", AR100_VERSIONS);
	//set ar100 cpu voltage to 1.2v
	//pmu_set_voltage(POWER_VOL_DC5LDO, 1200);

	//enter daemon process main.
	daemon_main();

	//to avoid daemon main return.
	LOG("system daemon exit\n");
	//while (1)
		;
}

#if 0
extern int ss_bonding_id(void);
void lcd_screen_size_check(void)
{
	u32 width;
	u32 height;
	volatile u32 value;
	static s32 screen_valid = 0;

	if (screen_valid == 1)
	{
		//screen size have checked already,
		//not check again to save normal operation time.
		//LOG("lcd screen size valid already\n");
		return;
	}

	//check chip bonding id, screen size check just use for A31S
	if (ss_bonding_id() != 1)
	{
		//all screen size is valid
		screen_valid = 1;
		return;
	}

	//try to enable gating ahb clock for lcd0
	if ((readl(0x01c20064) & (0x1 << 4)) == 0)
	{
		value = readl(0x01c20064);
		value |= ((0x1 << 4));
		writel(value, 0x01c20064);
	}

	//read out display screen size information,
	//screen width :bit16~26 + 1, must <= 1366,
	//screen height:bit00~10 + 1, must <= 800.
	value = readl(0x01c0c048);
	width  = ((value >> 16) & 0x7ff) + 1;
	height = (value & 0x7ff) + 1;
	//LOG("lcd screen size width=%d height=%d\n", width, height);
	if (width < height) {
		u32 temp = width;
		width = height;
		height = temp;
	}
	if ((width > 1366) || (height > 800))
	{
		//invalid screen size, disable LCD controller.
		//LOG("lcd screen size invalid\n");
		screen_valid = 0;
		value = readl(0x01c0c000);
		value &= (~(0x1 << 31));
		writel(value, 0x01c0c000);
	}
	else
	{
		//screen size valid
		//LOG("lcd screen size valid\n");
		screen_valid = 1;
	}
}
#endif

