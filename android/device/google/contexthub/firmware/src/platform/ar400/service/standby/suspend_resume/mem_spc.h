#ifndef __MEM_SPC_H__
#define __MEM_SPC_H__

#if defined CONFIG_ARCH_SUN8IW6P1
#define SPC_REG_START       (0x4)
#define SPC_REG_END         (0x24 + 0x04)
#define SPC_SKIP            3
#define SPC_REG_LENGTH      (((0x24 + 0x04)>>2)/SPC_SKIP)
#elif (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1)
#define SPC_REG_START       (0x4)
#define SPC_REG_END         (0x48 + 0x04)
#define SPC_SKIP            3
#define SPC_REG_LENGTH      (((0x48 + 0x04)>>2)/SPC_SKIP)
#elif (defined CONFIG_ARCH_SUN50IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1)
#define SPC_REG_START       (0x0)
#define SPC_REG_END         (0xd8 + 0x04)
#define SPC_SKIP            4
#define SPC_REG_LENGTH      (((0xd8 + 0x04)>>2)/SPC_SKIP)
#endif

struct spc_regs{
	unsigned int spc_reg_back[SPC_REG_LENGTH];
};

#endif /* __MEM_SPC_H__ */
