#include <plat/inc/include.h>

#if MEM_USED
#if defined CONFIG_ARCH_SUN8IW6P1
#include "mem_syscfg_sun8iw6.h"
#elif defined CONFIG_ARCH_SUN50IW1P1
#include "mem_syscfg_sun50iw1.h"
#elif defined CONFIG_ARCH_SUN50IW2P1
#include "mem_syscfg_sun50iw2.h"
#elif defined CONFIG_ARCH_SUN50IW3P1
#include "mem_syscfg_sun50iw3.h"
#elif defined CONFIG_ARCH_SUN50IW6P1
#include "mem_syscfg_sun50iw6.h"
#endif

int mem_syscfg_save(void)
{
	mem_reg_save(system_back->syscfg_back.syscfg_reg_back, \
	             (const unsigned int *)IO_ADDRESS(SUNXI_SYSCFG_PBASE), SYSCFG_REG_LENGTH, 1);

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_SRAMCTRL_PBASE), SRAM_REG_LENGTH);

	return OK;
}

int mem_syscfg_restore(void)
{
	mem_reg_restore((unsigned int *)IO_ADDRESS(SUNXI_SYSCFG_PBASE), \
	                (const unsigned int *)system_back->syscfg_back.syscfg_reg_back, SYSCFG_REG_LENGTH, 1);

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_SRAMCTRL_PBASE), SRAM_REG_LENGTH);

	return OK;
}

#endif /* MEM_USED */
