/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                cpucfg module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : cpucfg.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-7
* Descript: cpu config module.
* Update  : date                auther      ver     notes
*           2012-5-7 17:24:32   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "cpucfg_i.h"

#if defined CONFIG_ARCH_SUN50IW3P1
//void set_secondary_entry(u32 entry, u32 cpu);
//void sun50i_set_AA32nAA64(u32 cluster, u32 cpu, bool is_aa64);
static void cpucfg_counter_init(void);
volatile u32 soc_version_a = 1; /* default version a */
volatile u32 cpu_domain_lock = 0;

/*
*********************************************************************************************************
*                                       INITIALIZE CPUCFG
*
* Description:  initialize cpu configure module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize cpu configure succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_init(void)
{
	cpucfg_counter_init();

	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT CPUCFG
*
* Description:  exit cpu configure module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit cpu configure succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_exit(void)
{
	return OK;
}

/*
*********************************************************************************************************
*                                       SET SUPER-STANDBY FLAG
*
* Description:  set super-standby flag.
*
* Arguments  :  none.
*
* Returns    :  OK if set super-standby flag succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_set_super_standby_flag(void)
{
	//first write 0x16AA-EFE8
	writel(0x16AAEFE8, SUPER_STANDBY_FLAG_REG);

	//second write 0xAA16-EFE8
	writel(0xAA16EFE8, SUPER_STANDBY_FLAG_REG);

	return OK;
}

/*
*********************************************************************************************************
*                                       CLEAR SUPER-STANDBY FLAG
*
* Description:  clear super-standby flag.
*
* Arguments  :  none.
*
* Returns    :  OK if clear super-standby flag succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_clear_super_standby_flag(void)
{
	//first write 0x16AA-0000
	writel(0x16AA0000, SUPER_STANDBY_FLAG_REG);

	//second write 0xAA16-0000
	writel(0xAA160000, SUPER_STANDBY_FLAG_REG);

	return OK;
}

/*
*********************************************************************************************************
*                                      SET CPU RESET STATE
*
* Description:  set the reset state of cpu.
*
* Arguments  :  cpu_num : the cpu id which we want to set reset status.
*               state   : the reset state which we want to set.
*
* Returns    :  OK if get power status succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_set_cpu_reset_state(u32 cpu_num, s32 state)
{
	volatile u32 value;

	ASSERT(cpu_num < CPUCFG_CPU_NUMBER);

	//set cluster0 cpux state
	value  = readl(C0_RST_CTRL_REG);
	value &= ~(0x1 << cpu_num);
	value |= ((state & 0x1) << cpu_num);
	writel(value, C0_RST_CTRL_REG);

	return OK;
}

/*
*********************************************************************************************************
*                                      CLEAR 64BIT COUNTER
*
* Description:  clear 64bit counter, after this operation,
*               the value of conuter will reset to zero.
*
* Arguments  :  none.
*
* Returns    :  OK if clear counter succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_counter_clear(void)
{
	writel(0, CNT_LOW_REG_SET);
	writel(0, CNT_HIGH_REG_SET);

	return OK;
}

/*
*********************************************************************************************************
*                                      READ 64BIT COUNTER
*
* Description:  read 64bit counter, the counter value base on us.
*
* Arguments  :  none.
*
* Returns    :  the value of counter, base on us.
*********************************************************************************************************
*/
u64 cpucfg_counter_read(void)
{
	volatile u32 high;
	volatile u32 low;
	u64          counter;

	do {
		low  = readl(CNT_LOW_REG);
		high = readl(CNT_HIGH_REG);
	} while ((high != readl(CNT_HIGH_REG)) || (low > readl(CNT_LOW_REG)));

	counter = high;
	counter = (counter << 32) + low;

	return counter;
}

void cpucfg_counter_ctrl(bool enable)
{
	writel(enable, CNT_CTRL_REG);
}

static void cpucfg_counter_init(void)
{
	writel(CCU_HOSC_FREQ, CNT_FREQID_REG);
}

/*
*********************************************************************************************************
*                                       SET CPUX JUMP ADDR
*
* Description:  set cpux jump addr.
*
* Arguments  :
*               addr:    the boot address
*               type:    the type of resume
*
* Returns    :  OK if set cpux jump addr succeeded, others if failed.
*********************************************************************************************************
*/
s32 cpucfg_set_cpux_jump_addr(u32 addr)
{
	writel(addr, PRIVATE_REG1);

	return OK;
}

static void cpucfg_acinactm_process(u32 status)
{
	volatile u32 value;

	//Assert ACINACTM to LOW
	value  = readl(C0_CTRL_REG1);
	value &= ~0x1;
	value |= (0x1 & status);
	writel(value, C0_CTRL_REG1);
}

static void cpucfg_wait_l2_enter_wfi(void)
{
#ifndef FPGA_PLATFORM
	while ((readl(C0_CPU_STATUS_REG) & (1 << 0)) != 1)
		;
#endif
}

#if ((defined KERNEL_USED) || (defined TF_USED))
static void cpucfg_l1l2_reset_by_hardware(u32 status)
{
	volatile u32 value;

	value  = readl(C0_CTRL_REG0);
	value &= ~0x1f;
	value |= (0x1f & status);
	writel(value, C0_CTRL_REG0);
}
#endif

static void cpucfg_cluster0_process(u32 status)
{
	//volatile u32 value;

	switch(status)
	{
		case CLUSTER_RESET_ASSERT:
		{
			writel(0, SUNXI_CPU_RST_CTRL(0));
			break;
		}
		case CLUSTER_RESET_DEASSERT:
		{
			writel(0x13ff0100, SUNXI_CPU_RST_CTRL(0));
			writel(0xf, SUNXI_DBG_REG0);
			break;
		}
		default:
		{
			WRN("invalid cluster reset status\n");
		}
	}
}

static int cpu_power_switch_set(u32 cluster, u32 cpu, bool enable)
{
	if (enable) {
		if (0x00 == readl(SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			//WRN("cpu%d power switch enable already\n", cpu);
			return OK;
		}
		/* de-active cpu power clamp */
		writel(0xFE, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(20);

		writel(0xF8, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(10);

		writel(0xE0, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(10);

		writel(0xc0, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(10);

		writel(0x80, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(10);

		writel(0x00, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(20);
		while(0x00 != readl(SUNXI_CPU_PWR_CLAMP(cluster, cpu)))
			;
	} else {
		if (0xFF == readl(SUNXI_CPU_PWR_CLAMP(cluster, cpu))) {
			WRN("cpu%d power switch disable already\n", cpu);
			return OK;
		}
		writel(0xFF, SUNXI_CPU_PWR_CLAMP(cluster, cpu));
		time_udelay(30);
		while(0xFF != readl(SUNXI_CPU_PWR_CLAMP(cluster, cpu)))
			;
	}
	return OK;
}

/*
 * set cpu die before pmu power off
 */
void cpucfg_cpu_suspend(void)
{
	volatile u32 value;

	cpu_domain_lock = 1;

	/* Assert ACINACTM to HIGH */
	cpucfg_acinactm_process(ACINACTM_HIGH);
	save_state_flag(REC_HOTPULG | 0x0);

	/* wait L2 enter WFI */
	cpucfg_wait_l2_enter_wfi();
	save_state_flag(REC_HOTPULG | 0x1);

	value = readl(SUNXI_DBG_REG0);
	value &= (~(1<<0));
	writel(value, SUNXI_DBG_REG0);

	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU0, CPU_RESET_ASSERT);

	//set cpu0 poweron reset as assert state
	value  = readl(SUNXI_CLUSTER_PWRON_RESET(0));
	value &= ~(0x1 << 0);
	writel(value, SUNXI_CLUSTER_PWRON_RESET(0));

	//set cpu0 core_reset as assert state.
	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU0, CPU_RESET_ASSERT);
	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU1, CPU_RESET_ASSERT);
	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU2, CPU_RESET_ASSERT);
	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU3, CPU_RESET_ASSERT);
	save_state_flag(REC_HOTPULG | 0x2);
}

void cpucfg_cpu_suspend_late(void)
{
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));
	cpucfg_cluster0_process(CLUSTER_RESET_ASSERT);
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));
	save_state_flag(REC_HOTPULG | 0x3);

	//set cpu0 H_reset as assert state
	writel(0, SUNXI_CLUSTER_PWRON_RESET(0));
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));

	//set system reset to assert
	writel(0, SUNXI_CPU_SYS_RESET);
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));
	time_mdelay(1);

	//set clustr0+c0_cpu0 power off gating to valid
	ccu_set_poweroff_gating_state(PWRCTL_C0CPU0, CCU_POWEROFF_GATING_VALID);
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));
	save_state_flag(REC_HOTPULG | 0x4);

	//power off cluster0 cpu0
	cpu_power_switch_set(0, 0, 0);
	INF("%s-%u, %x\n", __func__, __LINE__, readl(0xD8110000));
	save_state_flag(REC_HOTPULG | 0x5);
}

/*
 * set standby or hotplug flag before.
 */
s32 cpucfg_cpu_resume_early(u32 resume_addr)
{
	//power on cluster0 cpu0
	cpu_power_switch_set(0, 0, 1);
	save_state_flag(REC_HOTPULG | 0x6);

	//set clustr0+cpu0 power off gating to invalid
	ccu_set_poweroff_gating_state(PWRCTL_C0CPU0, CCU_POWEROFF_GATING_INVALID);
	time_mdelay(1);
	save_state_flag(REC_HOTPULG | 0x7);
	//set system reset to de-assert state
	writel(1, SUNXI_CPU_SYS_RESET);

	//set cpu0 H_reset as de-assert state
	writel(1 << 16, SUNXI_CLUSTER_PWRON_RESET(0));

	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU0, CPU_RESET_ASSERT);
	cpucfg_cluster0_process(CLUSTER_RESET_ASSERT);
	save_state_flag(REC_HOTPULG | 0x8);

	//enable l1&l2 reset by hardware
	cpucfg_l1l2_reset_by_hardware(L1_EN_L2_EN);
	save_state_flag(REC_HOTPULG | 0x9);

	//Assert ACINACTM to LOW
	cpucfg_acinactm_process(ACINACTM_LOW);
	save_state_flag(REC_HOTPULG | 0xa);

	cpucfg_cluster0_process(CLUSTER_RESET_DEASSERT);

	set_secondary_entry(resume_addr, 0);

	 /* set AA32nAA64 to AA64 */
	sun50i_set_AA32nAA64(0, 0, 1);

	save_state_flag(REC_HOTPULG | 0xb);

	return OK;
}

#if STANDBY_USED
s32 cpucfg_cpu_resume(u32 resume_addr)
{
	u32 value;

	save_state_flag(REC_HOTPULG | 0xc);

	//set cpu0 power-on reset as deassert state
	value  = readl(SUNXI_CLUSTER_PWRON_RESET(0));
	value |= (0x1 << 0);
	writel(value, SUNXI_CLUSTER_PWRON_RESET(0));

	//set cpu0 core_reset as de-assert state.
	cpucfg_set_cpu_reset_state(CPUCFG_C0_CPU0, CPU_RESET_DEASSERT);
	save_state_flag(REC_HOTPULG | 0xd);

	cpu_domain_lock = 0;

	return OK;
}
#endif

#if CPUIDLE_USED
void cpucfg_mcpm_init(void)
{
	u32 value;

	value = readl(SUNXI_SYSCFG_PBASE + 0x24);
	LOG("soc version:%x\n", value);

	/* Version_B if bit8 is 1, set version_a to 0 */
	if (value & 0x0100)
		soc_version_a = 0;
}
#endif

#if CPUOP_USED
void set_secondary_entry(u32 entry, u32 cpu)
{
	 writel(entry, SUNXI_CPU_RVBA_L(cpu));
	 writel(0, SUNXI_CPU_RVBA_H(cpu));
}

void sun50i_set_AA32nAA64(u32 cluster, u32 cpu, bool is_aa64)
{
	volatile u32 value;

	value = readl(SUNXI_CLUSTER_CTRL0(cluster));
	value &= ~(1<<(cpu + 24));
	value |= (is_aa64 <<(cpu + 24));
	writel(value, SUNXI_CLUSTER_CTRL0(cluster));
	value = readl(SUNXI_CLUSTER_CTRL0(cluster));
}

void cpu_power_up(u32 cluster, u32 cpu)
{
	u32 value;

	/* Assert nCPUPORESET LOW */
	value	= readl(SUNXI_CPU_RST_CTRL(cluster));
	value &= (~(1<<cpu));
	writel(value, SUNXI_CPU_RST_CTRL(cluster));

	/* Assert cpu power-on reset */
	value	= readl(SUNXI_CLUSTER_PWRON_RESET(cluster));
	value &= (~(1<<cpu));
	writel(value, SUNXI_CLUSTER_PWRON_RESET(cluster));

	/* set AA32nAA64 to AA64 */
	sun50i_set_AA32nAA64(cluster, cpu, 1);

	/* Apply power to the PDCPU power domain. */
	cpu_power_switch_set(cluster, cpu, 1);

	/* Release the core output clamps */
	value = readl(SUNXI_CLUSTER_PWROFF_GATING(cluster));
	value &= (~(0x1<<cpu));
	writel(value, SUNXI_CLUSTER_PWROFF_GATING(cluster));
	time_udelay(1);

	/* Deassert cpu power-on reset */
	value	= readl(SUNXI_CLUSTER_PWRON_RESET(cluster));
	value |= ((1<<cpu));
	writel(value, SUNXI_CLUSTER_PWRON_RESET(cluster));

	/* Deassert core reset */
	value	= readl(SUNXI_CPU_RST_CTRL(cluster));
	value |= (1<<cpu);
	writel(value, SUNXI_CPU_RST_CTRL(cluster));

	/* Assert DBGPWRDUP HIGH */
	value = readl(SUNXI_DBG_REG0);
	value |= (1<<cpu);
	writel(value, SUNXI_DBG_REG0);
}

void cpu_power_down(u32 cluster, u32 cpu)
{
	u32 value;

	while (!CPU_IS_WFI_MODE(cluster, cpu));

	value = readl(SUNXI_IRQ_FIQ_MASK);
	value &= (~(0x11<<cpu));
	writel(value, SUNXI_IRQ_FIQ_MASK);

	/* step7: Deassert DBGPWRDUP LOW */
	value = readl(SUNXI_DBG_REG0);
	value &= (~(1<<cpu));
	writel(value, SUNXI_DBG_REG0);

	/* step8: Activate the core output clamps */
	value = readl(SUNXI_CLUSTER_PWROFF_GATING(cluster));
	value |= (1 << cpu);
	writel(value, SUNXI_CLUSTER_PWROFF_GATING(cluster));
	time_udelay(1);

	/* step9: Assert nCPUPORESET LOW */
	value	= readl(SUNXI_CPU_RST_CTRL(cluster));
	value &= (~(1<<cpu));
	writel(value, SUNXI_CPU_RST_CTRL(cluster));

       /* step10: Assert cpu power-on reset */
       value  = readl(SUNXI_CLUSTER_PWRON_RESET(cluster));
       value &= (~(1<<cpu));
       writel(value, SUNXI_CLUSTER_PWRON_RESET(cluster));

       /* step11: Remove power from th e PDCPU power domain */
	cpu_power_switch_set(cluster, cpu, 0);

}
#endif //CPUOP_USED

#if CPUIDLE_USED
int cpuidle_init(void)
{
	writel(24,SUNXI_F1F2_CONFIG_DELAY); //set config delay
	writel(24,SUNXI_PWR_SW_DELAY);      //set switch delay
	writel(0x16aa0000,SUNXI_CPUIDLE);   //enable hardware cpuidle
	writel(0xaa160001,SUNXI_CPUIDLE);
	//unmask fiq and irq to cpuidle hardware
        //so the cpuidle hardware can correspond
        //all interrupts.
	writel(0xff,SUNXI_IRQ_FIQ_MASK);

	INF("cpuidle init ok\n");
	return 0;
}

int cpuidle_exit(void)
{
	writel(0x16aa0000,SUNXI_CPUIDLE);   //enable hardware cpuidle
	writel(0xaa160000,SUNXI_CPUIDLE);

	INF("cpuidle exit ok\n");
	return 0;
}

int cpuidle_enter(u32 entrypoint, u32 mpidr, u32 level)
{
	u32 value = 0;

	//set re-entrypoint to corresponding
	writel(entrypoint,SUNXI_CPU_RVBA_L(mpidr));
	writel(0x0,SUNXI_CPU_RVBA_H(mpidr));
	sun50i_set_AA32nAA64(0,mpidr,1);
	//printk("0x%x, 0x%xi, 0x%x\n",readl(SUNXI_CPU_RVBA_L(mpidr)), readl(SUNXI_CPU_RVBA_H(mpidr)), readl(SUNXI_CLUSTER_CTRL0(0)));

	//set F1 bit for hardware to  close core
	value = readl(SUNXI_F1F2_FLAG);
	value |= 0x1 << (mpidr + (16 * (level - 1)));
	writel(value,SUNXI_F1F2_FLAG);

	//wait core to powr down
	//FIXME:Can remove, because this operation is useless
	while(!(readl(SUNXI_IDLE_STATE) & (0x1 << (mpidr + (4 * level)))))
		;

	//arisc_cpuidle_notify(mpidr,1,readl(SUNXI_IDLE_STATE));
	//printk("cpu :%d \n",mpidr);
	udelay(100);
	printk("idle_state :0x%x\n",readl(SUNXI_IDLE_STATE));
	return OK;
}
#endif //CPUIDLE_USED

s32 is_cpu_domain_lock(void)
{
	return cpu_domain_lock;
}
#endif // sun50iw6p1
