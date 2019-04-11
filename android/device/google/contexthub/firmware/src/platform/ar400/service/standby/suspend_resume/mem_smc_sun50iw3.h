#ifndef __MEM_SMC_SUN50IW3_H__
#define __MEM_SMC_SUN50IW3_H__

#define SMC_REG0_START       (0x0)
#define SMC_REG0_END         (0x58 + 0x04)
#define SMC_REG0_LENGTH      ((SMC_REG0_END - SMC_REG0_START)>>2)
#define SMC_REG1_START       (0x100)
#define SMC_REG1_END         (0x1F8 + 0x04)
#define SMC_REG1_LENGTH      ((SMC_REG1_END - SMC_REG1_START)>>2)
#define SMC_REG2_START       (0x70)
#define SMC_REG2_END         (0x78 + 0x04)
#define SMC_REG2_LENGTH      ((SMC_REG2_END - SMC_REG2_START)>>2)

struct smc_regs{
	unsigned int smc_reg0_back[SMC_REG0_LENGTH];
	unsigned int smc_reg1_back[SMC_REG1_LENGTH];
	unsigned int smc_reg2_back[SMC_REG2_LENGTH];
};

#endif /* __MEM_SMC_SUN6IW3_H__ */
