/**
 * service\cpuidle\cpuidle.c
 *
 * Descript: cpuidle service.
 * Copyright (C) 2014-2016 AllWinnertech Ltd.
 * Author: Xiafeng <xiafeng@allwinnertech.com>
 *
 */

#include "include.h"
#include "cpucfg_regs-sun8iw6w9.h"

#if CPUIDLE_USED

//#define CPUIDLE_DEBUG

/**
 * Maximum number of possible clusters / CPUs per cluster.
 *
 * This should be sufficient for quite a while, while keeping the
 * (assembly) code simpler.  When this starts to grow then we'll have
 * to consider dynamic allocation.
 */
#define MAX_CPUS_PER_CLUSTER    4
#define MAX_NR_CLUSTERS         2
#define MAX_NR_CPUS             (MAX_NR_CLUSTERS * MAX_CPUS_PER_CLUSTER)

/*
 * @flags: bit1:dram enter self-refresh, bit16:C1/C2 state.
 * @mpidr: cpu info.
 * @resume_addr: resume address for cpux out of idle.
 */
typedef struct cpuidle_para{
	unsigned long flags;
	unsigned long mpidr;
}cpuidle_para_t;

#ifdef CPUIDLE_DEBUG
static volatile u32 cpu_idlein_cnt[MAX_NR_CLUSTERS * MAX_CPUS_PER_CLUSTER];
static volatile u32 cpu_idleout_cnt[MAX_NR_CLUSTERS * MAX_CPUS_PER_CLUSTER];
static volatile u32 cluster_idlein_cnt[MAX_NR_CLUSTERS];
static volatile u32 cluster_idleout_cnt[MAX_NR_CLUSTERS];
static u32 dram_enter_cnt, dram_out_cnt;
#endif

/* flg between cpux & cpus for synchronization */
#define CLUSTER_CPUX_FLG(cluster, cpu) \
         (MESSAGE_POOL_END -  4 - 16*(cluster) - 4*(cpu))
#define CLUSTER_CPUS_FLG(cluster, cpu) \
         (MESSAGE_POOL_END - 36 - 16*(cluster) - 4*(cpu))
#define CLUSTER_CPUW_FLG(cluster, cpu) \
         (MESSAGE_POOL_END - 68 - 16*(cluster) - 4*(cpu))

#define CLUSTER_CPUS_GICW_FLG() \
         (MESSAGE_POOL_END - 100)

/* C2 State Flags */
#define CPUIDLE_FLAG_C2_STATE   (1<<16) /* is in c2 state */
/* dram enter self-refreash flg */
#define DRAM_SELFREFLASH        (1<<0)

#define CPUIDLE_MASK 0x0ff00
#define CPUIDLE_SHIRFT 16

#define MPIDR_LEVEL_BITS 8
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
         ((mpidr >> (MPIDR_LEVEL_BITS * level)) & MPIDR_LEVEL_MASK)

/* mask define */
#define cpuidle_read_mask() (readl(CPUXCFG_REG_ADDR + 0x40))
#define CPUIDLE_CPU_MASK(n) ((1<<8 | 1) << (n))
#define CPUIDLE_CLUSTER_MASK(cluster) (cluster ? ((0x0f<<8 | 0x0f) << 4) : (0x0f<<8 | 0x0f))
#define cpuidle_set_mask(val) (writel(val, (CPUXCFG_REG_ADDR + 0x40)))
/* irq define */
#define cpuidle_get_irq() (readl(CPUXCFG_REG_ADDR + 0x3c))
#define cpuidle_cpu_query_irq(v, n) ((v) & ((1<<8 | 1) << (n)))
#define cpuidle_cluster_query_irq(v, cluster) \
         ((v) & CPUIDLE_CLUSTER_MASK(cluster))

/* record cpu & cluster state. */
static volatile u32 cpu_state = 0; /* 1:off, 0:on, used in C1 and C2 state. */
static volatile u32 cluster_state = 0x3; /* 1:cluster0 alive, 2:cluster1 alive, used in C2 state. */

/* sunxi cluster and cpu use status,
 * this is use to detect the first-man and last-man.
 */
static volatile int cpu_c2_cnt[MAX_NR_CLUSTERS][MAX_CPUS_PER_CLUSTER];
static volatile int cluster_c2_cnt[MAX_NR_CLUSTERS];

static void cpux_hotplug_control(bool enable)
{
	if (enable) {
		writel(0xFA50392F, PRIVATE_REG0);
		writel(0xFA50392F, BOOT_CPU_HOTPLUG_REG);
	} else
		writel(0x00000000, BOOT_CPU_HOTPLUG_REG);
}

static int dram_suspended = 0;

static void gic_out_handler(void *arg)
{
	u32 i;
	u32 cluster, cpu;
	u32 cpuidle_mask;
	u32 cpu_irq_state;

	trace_time_tag(8);

	if (dram_suspended) {
		dram_idle_exit_process();
		dram_suspended = 0;
#ifdef CPUIDLE_DEBUG
		dram_out_cnt++;
#endif
	}

	//writel(0xa5, CLUSTER_CPUS_GICW_FLG());
	if (hwspin_lock_timeout(HWSPINLOCK_CPUIDLE, SPINLOCK_TIMEOUT) != OK)
		LOG("gto!\n");

	cpuidle_mask = cpuidle_read_mask();
	cpu_irq_state = cpuidle_get_irq();

	for (i = 0; i < MAX_NR_CLUSTERS; ++i) {
		if (cluster_state & (1<<i))
			continue;

		if (cpuidle_cluster_query_irq(cpu_irq_state, i)) {
			if (i)
				pmu_set_voltage_state(AW1660_POWER_DCDC3, POWER_VOL_ON);
			else
				pmu_set_voltage_state(AW1660_POWER_DCDC2, POWER_VOL_ON);
			time_mdelay(1);
			cluster_power_set(i, 1);
			cluster_state |= (1<<i);
			time_mdelay(1);
#ifdef CPUIDLE_DEBUG
			cluster_idleout_cnt[i]++;
#endif
		}
	}

	for (i = 0; i < MAX_NR_CPUS; i++) {
		if ((cpu_state & (1<<i)) && (cpuidle_mask & (1<<i)) && \
		    cpuidle_cpu_query_irq(cpu_irq_state, i)) {
			cluster = i / 4;
			cpu = i % 4;
			trace_time_tag(i);
			cpuidle_mask &= ~(CPUIDLE_CPU_MASK(i));
			cpuidle_set_mask(cpuidle_mask);
			cpu_state &= ~(1<<i);

			//save_state_flag(0xa9);
			cpu_power_set(cluster, cpu, 1);
			INF("cpu%d pwon\n", i);
#ifdef CPUIDLE_DEBUG
			cpu_idleout_cnt[i]++;
#endif
			 if (!cpu_c2_cnt[cluster][cpu]) {
				cluster_c2_cnt[cluster]++;
				cpu_c2_cnt[cluster][cpu]++;
			}
			trace_time_record(i);
		}
	}

	time_cdelay(50);
	hwspin_unlock(HWSPINLOCK_CPUIDLE);
	//writel(0, CLUSTER_CPUS_GICW_FLG());

	interrupt_clear_pending(INTC_GIC_OUT_IRQ);
	trace_time_record(8);
}

static s32 cpuidle_config(u32 flg, u32 mpidr)
{
	u32 cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
	u32 cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);
	u32 cpu_n = (cluster<<2) + cpu;

	if (flg & (1<<24)) {
		if (flg & (1<<25)) {
			cluster_c2_cnt[cluster]++;
			cpu_c2_cnt[cluster][cpu]++;
			cpu_state &= ~(1<<cpu_n);
		} else {
			cluster_c2_cnt[cluster]--;
			cpu_c2_cnt[cluster][cpu]--;
			cpu_state |= 1<<cpu_n;
		}
		LOG("cpu:%d,cnt:%d\n", cpu_n, cluster_c2_cnt[cluster]);
	}

	return 0;
}

/*
 * cpuidle_enter - the entry of cpuidle.
 * @mpidr: cpu info.
 * @addr: cpu run address after come out form idle
 * @returns: OK if enter idle succeeded, others if failed.
 */
static s32 cpuidle_enter(u32 flg, u32 mpidr)
{
	u32 value = 0;
	u32 cluster = MPIDR_AFFINITY_LEVEL(mpidr, 1);
	u32 cpu = MPIDR_AFFINITY_LEVEL(mpidr, 0);
	u32 cpu_n = (cluster<<2) + cpu;
	u32 cpuidle_mask, cpu_irq_state;
	u32 cluster_frezzing_flg = 0;

	//BUG_ON(cpu >= MAX_CPUS_PER_CLUSTER || cluster >= MAX_NR_CLUSTERS);
	trace_time_tag(cpu_n);

	/* step1: clear pending, the pending maybe generate by normal cpux int.
	 * GIC_OUT int will generate again, if there is a GIC_OUT int need to handle
	 */
	interrupt_clear_pending(INTC_GIC_OUT_IRQ);
	//save_state_flag(0xa1);

	if (flg & CPUIDLE_FLAG_C2_STATE) {
		cluster_c2_cnt[cluster]--;
		cpu_c2_cnt[cluster][cpu]--;
		if (readl(CLUSTER_CPUX_FLG(cluster, cpu))) {
			if (cluster_c2_cnt[cluster] == 0) {
				writel(2, CLUSTER_CPUS_FLG(cluster, cpu));
				cluster_frezzing_flg = 1;
			} else {
				writel(4, CLUSTER_CPUS_FLG(cluster, cpu));
			}
		}
	}
	/* wait for cpu enter wfi without hwspinlock */
	while (readl(CLUSTER_CPUW_FLG(cluster, cpu)))
		INF("%dw\n", cpu);

	//writel(0xa5, CLUSTER_CPUS_GICW_FLG());
	if (hwspin_lock_timeout(HWSPINLOCK_CPUIDLE, SPINLOCK_TIMEOUT) != OK)
		LOG("cto!\n");

	//save_state_flag(0xa3);
	while (1) {
		if (CPU_IS_WFI_MODE(cluster, cpu))
			break;
		time_udelay(2); /* delay 2us */
		if (value++ == 10000) { /* wait timeout */
			writel(0, CLUSTER_CPUS_FLG(cluster, cpu));
			ERR("cpuidle wait cpu%d enter wfi timeout\n", \
			    (cluster<<2) + cpu);
			goto out;
		}
	}

	//save_state_flag(0xa5);
	writel(0, CLUSTER_CPUS_FLG(cluster, cpu));

	/* set cpux power off */
	cpu_power_set(cluster, cpu, 0);
	INF("c:%d, c:%d\n", cluster, cpu);
	cpu_state |= 1 << cpu_n;
	cpuidle_mask = cpuidle_read_mask();
	cpuidle_set_mask(cpuidle_mask | CPUIDLE_CPU_MASK(cpu_n));
#ifdef CPUIDLE_DEBUG
	cpu_idlein_cnt[cpu_n]++;
#endif
	if (cluster_frezzing_flg) {
		cluster_power_set(cluster, 0);
		cluster_state &= ~(1<<cluster);
		time_udelay(10);
#ifdef CPUIDLE_DEBUG
		cluster_idlein_cnt[cluster]++;
#endif
		/* there is no need to pmu power off cluster,
		 * if cpu irq has came
		 */
		cpu_irq_state = cpuidle_get_irq();
		if (cpuidle_cluster_query_irq(cpu_irq_state, cluster))
			goto out;
		time_mdelay(1);
		if (cluster)
			pmu_set_voltage_state(AW1660_POWER_DCDC3, POWER_VOL_OFF);
		else
			pmu_set_voltage_state(AW1660_POWER_DCDC2, POWER_VOL_OFF);
		time_mdelay(1);
	}
	if ((flg & DRAM_SELFREFLASH) && (!cluster_c2_cnt[0]) && \
	    (!cluster_c2_cnt[1])) {
		dram_idle_enter_process();
		dram_suspended = 1;
#ifdef CPUIDLE_DEBUG
		dram_enter_cnt++;
#endif
	}
	//save_state_flag(0xa7);

out:
	hwspin_unlock(HWSPINLOCK_CPUIDLE);

	time_udelay(10);
	interrupt_clear_pending(INTC_GIC_OUT_IRQ);

	//save_state_flag(0xa9);
	trace_time_record(cpu_n);

	return 0;
}

//static
int cpuidle_suspend(void *arg)//struct device *dev)
{
	cpux_hotplug_control(0);

	return 0;
}

//static
int cpuidle_resume(void *arg)//struct device *dev)
{
	cpux_hotplug_control(1);

	return 0;
}

int cpuidle_msg_handler(u32 req, void *para)
{
	int result = 0;
	struct cpuidle_para *idle_para = para;
	u32 flags = idle_para->flags;
	u32 mpidr = idle_para->mpidr;

	switch (req) {
	case CPUIDLE_ENTER_REQ:
		INF("cpu idle enter\n");
		result = cpuidle_enter(flags, mpidr);
		break;
	case CPUIDLE_CFG_REQ:
		INF("cpu idle config\n");
		result = cpuidle_config(flags, mpidr);
		break;
	default :
		ERR("%s,%d err!\n", __func__, __LINE__);
		result = 1;
	}

	return result;
}

int cpuidle_init(void)
{
	int i, j;

	cpucfg_mcpm_init();

	for (i = 0; i < MAX_NR_CLUSTERS; i++) {
		for (j = 0; j < MAX_CPUS_PER_CLUSTER; j++)
			cpu_c2_cnt[i][j] = 1;
		cluster_c2_cnt[i] = MAX_CPUS_PER_CLUSTER;
	}

	interrupt_clear_pending(INTC_GIC_OUT_IRQ);
	install_isr(INTC_GIC_OUT_IRQ, (__pISR_t)gic_out_handler, NULL);
	interrupt_enable(INTC_GIC_OUT_IRQ);
	writel(0, CLUSTER_CPUS_GICW_FLG());
#ifdef CPUIDLE_DEBUG
	LOG("cpu0w:%x\n", CLUSTER_CPUW_FLG(0, 0));
	LOG("cpu1w:%x\n", CLUSTER_CPUW_FLG(0, 1));
#endif
	LOG("cpuidle init ok\n");

	return 0;
}

#ifdef CPUIDLE_DEBUG
void cpuidle_debug(void)
{
	int i, j;

	LOG("cpu_state:%x\n", cpu_state);
	LOG("rmask:%x\n", cpuidle_read_mask());
	LOG("rirq_state:%x\n", cpuidle_get_irq());

	for (i = 0; i < (MAX_NR_CLUSTERS * MAX_CPUS_PER_CLUSTER); i++)
		LOG("idi_%dcnt:%d\n", i, cpu_idlein_cnt[i]);

	for (i = 0; i < (MAX_NR_CLUSTERS * MAX_CPUS_PER_CLUSTER); i++)
		LOG("ido_%dcnt:%d\n", i, cpu_idleout_cnt[i]);

	LOG("cluster_state:%x\n", cluster_state);

	for (i = 0; i < MAX_NR_CLUSTERS; i++)
		LOG("cluster_idlein_cnt[%d]:%d\n", i, cluster_idlein_cnt[i]);
	for (i = 0; i < MAX_NR_CLUSTERS; i++)
		LOG("cluster_idleout_cnt[%d]:%d\n", i, cluster_idleout_cnt[i]);

	for (i = 0; i < MAX_NR_CLUSTERS; i++) {
		LOG("cluster_c2_cnt[%d]:%x\n", i, cluster_c2_cnt[i]);
		for (j = 0; j < MAX_CPUS_PER_CLUSTER; j++)
			LOG("cpu_c2_cnt[%d][%d]:%x\n", i, j, \
			    cpu_c2_cnt[i][j]);
	}
	LOG("dram_enter_cnt:%d\n", dram_enter_cnt);
	LOG("dram_out_cnt:%d\n", dram_out_cnt);
}
#endif
#endif
