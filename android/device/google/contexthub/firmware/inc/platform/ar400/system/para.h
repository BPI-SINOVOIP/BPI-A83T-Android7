#ifndef __ARISC_PARA_H__
#define __ARISC_PARA_H__

#include "../driver/dram.h"
#include "../driver/ir.h"
#include "../driver/pmu.h"
#include "../service/dvfs.h"
#include "../service/standby.h"

#define MACHINE_PAD    0
#define MACHINE_HOMLET 1

#if (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)

typedef struct arisc_para
{
	u32 message_pool_phys;
	u32 message_pool_size;
	u32 standby_base;
	u32 standby_size;
	u32 suart_status;
	u32 pmu_bat_shutdown_ltf;
	u32 pmu_bat_shutdown_htf;
	u32 pmu_pwroff_vol;
	u32 power_mode;
	u32 power_start;
	u32 powchk_used;
	u32 power_reg;
	u32 system_power;
	struct ir_key ir_key;
	struct dram_para dram_para;
//dvfs config
#if DVFS_USED
	struct freq_voltage vf[DVFS_VF_TABLE_MAX];
#endif
	u32 power_regu_tree[DM_MAX];
	struct box_start_os_cfg start_os;
} arisc_para_t;
#else
/* arisc parameter size: 128byte */
/*
 * machine: machine id, pad = 0, homlet = 1;
 * message_pool_phys: message pool physical address;
 * message_pool_size: message pool size;
 */
 typedef struct arisc_para
{
	unsigned int machine;
	unsigned int oz_scale_delay;
	unsigned int oz_onoff_delay;
	unsigned int message_pool_phys;
	unsigned int message_pool_size;
	unsigned int suart_status;
	unsigned int services_used;
	unsigned int power_regu_tree[DM_MAX];
	unsigned int reseved[10];
} arisc_para_t;
#endif

extern struct arisc_para arisc_para;

#define ARISC_PARA_SIZE (sizeof(struct arisc_para))

extern void arisc_para_init(void);
extern void set_paras(void);
extern int arisc_para_get_message_pool_info(unsigned int *addr, unsigned int *size);
extern int arisc_para_get_standby_para_info(unsigned int *addr, unsigned int *size);
extern int arisc_para_get_uart_pin_info(void);

#endif /* __ARISC_PARA_H__ */
