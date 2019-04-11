#ifndef __MEM_GPIO_SUN50IW3_H__
#define __MEM_GPIO_SUN50IW3_H__

#if defined CONFIG_ARCH_SUN50IW3P1

#define GPIO_REG_LENGTH ((0x348 + 0x4) >> 2)

struct gpio_state{
	unsigned int gpio_reg_back[GPIO_REG_LENGTH];
};

static inline void open_gpio_gating(void)
{
}
#endif /* CONFIG_ARCH_SUN50IW3P1 */
#endif /* __MEM_GPIO_SUN50IW3_H__ */
