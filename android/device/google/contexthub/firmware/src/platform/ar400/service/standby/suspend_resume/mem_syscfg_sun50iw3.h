#ifndef __MEM_SYSCFG_SUN50IW3_H__
#define __MEM_SYSCFG_SUN50IW3_H__

#if defined CONFIG_ARCH_SUN50IW3P1

#define SYSCFG_REG_LENGTH		((0xec+0x4)>>2)

struct syscfg_state{
	unsigned int syscfg_reg_back[SYSCFG_REG_LENGTH];
};

#endif /* CONFIG_ARCH_SUN50IW3P1 */
#endif /* __MEM_SYSCFG_SUN50IW3_H__ */
