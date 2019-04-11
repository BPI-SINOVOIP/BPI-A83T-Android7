#include <plat/inc/include.h>

#if MEM_USED
#define MEM_DEBUG 0

#define SIZE_OF_TYPE_NOT_LARGER_THAN(type, size) \
static inline char size_of_##type##_not_larger_than_##size() \
{ \
    char __dummy1[size - sizeof(type)] = {0}; \
    return __dummy1[-1]; \
}

SIZE_OF_TYPE_NOT_LARGER_THAN(system_state_t, ARISC_USE_DRAM_DATA_SIZE);

struct system_state *system_back = (struct system_state *)ARISC_USE_DRAM_DATA_PBASE;

int mem_long_jump(int (*fn)(u32 arg), u32 arg)
{
	return (*fn)(arg);
}

int mem_linux_save(void *arg)
{
#if MEM_DEBUG
	hexdump("sysctrl", (char *)IO_ADDRESS(SUNXI_SYSCFG_PBASE), SYSCFG_REG_LENGTH * 4);
	hexdump("smc0", (char *)IO_ADDRESS(SUNXI_SMC_PBASE), SMC_REG0_LENGTH * 4);
	hexdump("smc1", (char *)IO_ADDRESS(SUNXI_SMC_PBASE + SMC_REG1_START), SMC_REG1_LENGTH * 4);
	hexdump("smc2", (char *)IO_ADDRESS(SUNXI_SMC_PBASE + SMC_REG2_START), SMC_REG2_LENGTH * 4);
	hexdump("spc", (char *)IO_ADDRESS(SUNXI_SPC_PBASE), SPC_REG_LENGTH * SPC_SKIP * 4);
	hexdump("gpio", (char *)IO_ADDRESS(SUNXI_PIO_PBASE), GPIO_REG_LENGTH * 4);
	hexdump("ccm", (char *)IO_ADDRESS(SUNXI_CCM_PBASE), CCM_REG_LENGTH * 4);
	hexdump("timestamp", (char *)IO_ADDRESS(SUNXI_TIMESTAMP_CTRL_PBASE), TMSTMP_REG_LENGTH * 4);
#endif

	mem_syscfg_save();

	mem_smc_save();

	mem_spc_save();

	//mem_timer_save();

	mem_gpio_save(arg);

#if (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
	mem_ccu_save();
#endif

	mem_tmstmp_save();

	return OK;
}

int mem_linux_restore(void *arg)
{
	mem_tmstmp_restore();

#if (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
	mem_ccu_restore();
#endif

	mem_gpio_restore(arg);

	//mem_timer_restore();

	mem_spc_restore();

	mem_smc_restore();

	mem_syscfg_restore();

#if MEM_DEBUG
	hexdump("sysctrl", (char *)IO_ADDRESS(SUNXI_SYSCFG_PBASE), SYSCFG_REG_LENGTH * 4);
	hexdump("smc0", (char *)IO_ADDRESS(SUNXI_SMC_PBASE), SMC_REG0_LENGTH * 4);
	hexdump("smc1", (char *)IO_ADDRESS(SUNXI_SMC_PBASE + SMC_REG1_START), SMC_REG1_LENGTH * 4);
	hexdump("smc2", (char *)IO_ADDRESS(SUNXI_SMC_PBASE + SMC_REG2_START), SMC_REG2_LENGTH * 4);
	hexdump("spc", (char *)IO_ADDRESS(SUNXI_SPC_PBASE), SPC_REG_LENGTH * SPC_SKIP * 4);
	hexdump("gpio", (char *)IO_ADDRESS(SUNXI_PIO_PBASE), GPIO_REG_LENGTH * 4);
	hexdump("ccm", (char *)IO_ADDRESS(SUNXI_CCM_PBASE), CCM_REG_LENGTH * 4);
	hexdump("timestamp", (char *)IO_ADDRESS(SUNXI_TIMESTAMP_CTRL_PBASE), TMSTMP_REG_LENGTH * 4);
#endif

	return OK;
}

#endif /* MEM_USED */
