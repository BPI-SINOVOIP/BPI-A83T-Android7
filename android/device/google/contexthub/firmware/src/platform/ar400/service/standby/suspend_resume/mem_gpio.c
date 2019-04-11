#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>
#if MEM_USED
#if defined CONFIG_ARCH_SUN8IW6P1
#include "mem_gpio_sun8iw6.h"
#elif defined CONFIG_ARCH_SUN50IW1P1
#include "mem_gpio_sun50iw1.h"
#elif defined CONFIG_ARCH_SUN50IW2P1
#include "mem_gpio_sun50iw2.h"
#elif defined CONFIG_ARCH_SUN50IW3P1
#include "mem_gpio_sun50iw3.h"
#elif defined CONFIG_ARCH_SUN50IW6P1
#include "mem_gpio_sun50iw6.h"
#endif

#define IOCFG(n) (config->soc_io_state.io_state[n])

int mem_gpio_suspend_cfg(struct super_standby_para *para)
{
	//MEM_LOG("%s\n", __func__);
	if (para->event & CPUS_WAKEUP_GPIO) {
		mem_int_enable(R_GPIOL_IRQn);
		mem_int_enable(R_GPIOM_IRQn);

		//config cpux EINT
		if (para->cpux_gpiog_bitmap) {
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('B')) {
				mem_int_enable(GPIOB_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('F')) {
				mem_int_enable(GPIOF_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('G')) {
				mem_int_enable(GPIOG_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('H')) {
				mem_int_enable(GPIOH_IRQn);
			}
		}
	}

	return OK;
}

int mem_gpio_resume_cfg(struct super_standby_para *para)
{
	//MEM_LOG("%s\n", __func__);
	if (para->event & CPUS_WAKEUP_GPIO) {
		//config cpux EINT
		if (para->cpux_gpiog_bitmap) {
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('B')) {
				mem_int_disable(GPIOB_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('F')) {
				mem_int_disable(GPIOF_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('G')) {
				mem_int_disable(GPIOG_IRQn);
			}
			if (para->cpux_gpiog_bitmap & WAKEUP_GPIO_GROUP('H')) {
				mem_int_disable(GPIOH_IRQn);
			}
		}
	}

	return OK;
}

int mem_gpio_save(struct extended_standby *config)
{
	volatile int i=0;
	volatile u32 value;

	mem_reg_save(system_back->gpio_back.gpio_reg_back, \
	             (const unsigned int *)IO_ADDRESS(SUNXI_PIO_PBASE), GPIO_REG_LENGTH, 1);

	//mem_long_jump((mem_long_jump_fn)time_mdelay, 1);
	for (i = 0; i < IO_NUM; i++) {
		value = (readl(IOCFG(i).paddr) & (~(IOCFG(i).value_mask))) | (IOCFG(i).value & IOCFG(i).value_mask);
		writel(value, IOCFG(i).paddr);
	}

	//mem_reg_debug(__func__, 0x01c20800, GPIO_REG_LENGTH);
	//mem_reg_debug(__func__, system_back->gpio_back.gpio_reg_back, GPIO_REG_LENGTH);

	return OK;
}

int mem_gpio_restore(struct extended_standby *config)
{
	open_gpio_gating();

	mem_reg_restore((unsigned int *)IO_ADDRESS(SUNXI_PIO_PBASE), \
	                (const unsigned int *)system_back->gpio_back.gpio_reg_back, GPIO_REG_LENGTH, 1);

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_PIO_PBASE), GPIO_REG_LENGTH);

	return OK;
}

#endif /* MEM_USED */
