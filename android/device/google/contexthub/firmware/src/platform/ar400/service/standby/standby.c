/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                               standby module
*
*                                    (c) Copyright 2012-2016, Sunny China
*                                             All Rights Reserved
*
* File    : standby.c
* By      : Sunny
* Version : v1.0
* Date    : 2012-5-14
* Descript: standby module public header.
* Update  : date                auther      ver     notes
*           2012-5-14 8:55:32   Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#include "standby_i.h"

#if STANDBY_USED

//dram traning area backup
#if (defined CONFIG_ARCH_SUN8IW1P1) || \
	(defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1)
u8  dram_traning_area_back[DRAM_TRANING_SIZE];
#endif

#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
struct standby_info_para sst_info = {
	{0, 0, 0},
	{0, (unsigned int *)0x40000000, (1024 * 1024), 0, 0},
};
struct sst_power_info_para power_chk_back;
#endif

//dram crc result
u32 before_crc;
u32 after_crc;

/*
*********************************************************************************************************
*                                       INIT SUPER-STANDBY
*
* Description:  initialize super-standby module.
*
* Arguments  :  none.
*
* Returns    :  OK if initialize succeeded, others if failed.
*********************************************************************************************************
*/
s32 standby_init(void)
{
	before_crc = 0;
	after_crc = 0;
	return OK;
}

/*
*********************************************************************************************************
*                                       EXIT SUPER-STANDBY
*
* Description:  exit super-standby module.
*
* Arguments  :  none.
*
* Returns    :  OK if exit succeeded, others if failed.
*********************************************************************************************************
*/
#if 0
static s32 standby_exit(void)
{
	return OK;
}
#endif

#if (defined CONFIG_ARCH_SUN8IW1P1) || (defined CONFIG_ARCH_SUN8IW3P1) || \
	(defined CONFIG_ARCH_SUN8IW5P1)
s32 pll_regs_backup(struct pll_reg *plls)
{
	//plls->pll1      = readl(CCU_PLL1_REG);  //pll1 already restore
	plls->pll2      = readl(CCU_PLL2_REG);
	plls->pll3      = readl(CCU_PLL3_REG);
	plls->pll4      = readl(CCU_PLL4_REG);
	//plls->pll5      = readl(CCU_PLL5_REG);  //pll5 restore by dram driver
	plls->pll6      = readl(CCU_PLL6_REG);
#ifndef CONFIG_ARCH_SUN8IW5P1 //not define
	plls->pll7      = readl(CCU_PLL7_REG);
#endif
	plls->pll8      = readl(CCU_PLL8_REG);
	plls->pllmipi   = readl(CCU_PLL_MIPI_REG);
	plls->pll9      = readl(CCU_PLL9_REG);
	plls->pll10     = readl(CCU_PLL10_REG);

	return OK;
}

s32 pll_regs_restore(struct pll_reg *plls)
{
	//writel(plls->pll1,     CCU_PLL1_REG);  //pll1 already restore
	writel(plls->pll2,     CCU_PLL2_REG);
	writel(plls->pll3,     CCU_PLL3_REG);
	writel(plls->pll4,     CCU_PLL4_REG);
	//writel(plls->pll5,     CCU_PLL5_REG);  //pll5 restore by dram driver
	writel(plls->pll6,     CCU_PLL6_REG);
#ifndef CONFIG_ARCH_SUN8IW5P1 //not define
	writel(plls->pll7,     CCU_PLL7_REG);
#endif
	writel(plls->pll8,     CCU_PLL8_REG);
	writel(plls->pllmipi,  CCU_PLL_MIPI_REG);
	writel(plls->pll9,     CCU_PLL9_REG);
	writel(plls->pll10,    CCU_PLL10_REG);

	return OK;
}

#elif (defined CONFIG_ARCH_SUN8IW6P1) || (defined CONFIG_ARCH_SUN8IW9P1)
/*
s32 pll_regs_backup(struct pll_reg *plls)
{
	//plls->pll1      = READ_REG32(CCU_PLL1_REG);  //pll1 already restore
	//plls->pll2      = READ_REG32(CCU_PLL2_REG);  //pll1 already restore
	plls->pll3      = READ_REG32(CCU_PLL3_REG);
	plls->pll4      = READ_REG32(CCU_PLL4_REG);
	plls->pll5      = READ_REG32(CCU_PLL5_REG);
	//plls->pll6      = READ_REG32(CCU_PLL6_REG);//pll5 restore by dram driver
	plls->pll7      = READ_REG32(CCU_PLL7_REG);
	plls->pll8      = READ_REG32(CCU_PLL8_REG);
	plls->pll9      = READ_REG32(CCU_PLL9_REG);
	plls->pll10     = READ_REG32(CCU_PLL10_REG);
	plls->pll11     = READ_REG32(CCU_PLL11_REG);

	return OK;
}

s32 pll_regs_restore(struct pll_reg *plls)
{
	//WRITE_REG32(plls->pll1,     CCU_PLL1_REG);  //pll1 already restore
	//WRITE_REG32(plls->pll2,     CCU_PLL2_REG);  //pll2 already restore
	WRITE_REG32(plls->pll3,     CCU_PLL3_REG);
	WRITE_REG32(plls->pll4,     CCU_PLL4_REG);
	WRITE_REG32(plls->pll5,     CCU_PLL5_REG);
	//WRITE_REG32(plls->pll6,     CCU_PLL6_REG);  //pll6 restore by dram driver
	WRITE_REG32(plls->pll7,     CCU_PLL7_REG);
	WRITE_REG32(plls->pll8,     CCU_PLL8_REG);
	WRITE_REG32(plls->pll9,     CCU_PLL9_REG);
	WRITE_REG32(plls->pll10,    CCU_PLL10_REG);
	WRITE_REG32(plls->pll11,    CCU_PLL11_REG);
	return OK;
}
*/
int long_jump(int (*fn)(void *arg), void *arg)
{
	INF("fn:%x,arg:%x\n", fn, arg);

	return (*fn)(arg);
}

#elif (defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1)
s32 pll_regs_backup(struct pll_reg *plls)
{
	//plls->pll1      = readl(CCU_PLL1_REG);  //pll1 already restore
	plls->pll2      = readl(CCU_PLL2_REG);
	plls->pll3      = readl(CCU_PLL3_REG);
	plls->pll4      = readl(CCU_PLL4_REG);
	//plls->pll5      = readl(CCU_PLL5_REG); //pll5 restore by dram driver
	plls->pll6      = readl(CCU_PLL6_REG);
	plls->pll7      = readl(CCU_PLL7_REG);
	plls->pll8      = readl(CCU_PLL8_REG);
	plls->pll9      = readl(CCU_PLL9_REG);
	plls->pll10     = readl(CCU_PLL10_REG);
	plls->pll11     = readl(CCU_PLL11_REG);
	plls->pll12     = readl(CCU_PLL12_REG);
	//plls->pll13     = readl(CCU_PLL13_REG); //pll13 restore by dram driver

	return OK;
}

s32 pll_regs_restore(struct pll_reg *plls)
{
	//writel(plls->pll1,     CCU_PLL1_REG);  //pll1 already restore
	writel(plls->pll2,     CCU_PLL2_REG);
	writel(plls->pll3,     CCU_PLL3_REG);
	writel(plls->pll4,     CCU_PLL4_REG);
	//writel(plls->pll5,     CCU_PLL5_REG);  //pll5 restore by dram driver
	writel(plls->pll6,     CCU_PLL6_REG);
	writel(plls->pll7,     CCU_PLL7_REG);
	writel(plls->pll8,     CCU_PLL8_REG);
	writel(plls->pll9,     CCU_PLL9_REG);
	writel(plls->pll10,    CCU_PLL10_REG);
	writel(plls->pll11,    CCU_PLL11_REG);
	writel(plls->pll12,    CCU_PLL12_REG);
	//writel(plls->pll13,    CCU_PLL13_REG); //pll13 restore by dram driver

	return OK;
}

int long_jump(int (*fn)(void *arg), void *arg)
{
	INF("fn:%x,arg:%x\n", fn, arg);

	return (*fn)(arg);
}
#elif (defined CONFIG_ARCH_SUN50IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1)
int long_jump(int (*fn)(void *arg), void *arg)
{
	INF("fn:%p,arg:%p\n", fn, arg);

	return (*fn)(arg);
}
#endif

#if (defined CONFIG_ARCH_SUN8IW5P1) || \
	(defined CONFIG_ARCH_SUN8IW6P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
s32 sstandby_query_info(struct message *pmsg)
{
	/* DO NOT use memcpy for big/little transform */
	if (ARISC_WRITE == (unsigned int)pmsg->private) {/* write operation */
		/* the data from pmsg->paras[0] */
		//power_chk_back.enable = pmsg->paras[0];
		//power_chk_back.power_reg = pmsg->paras[1];
		//power_chk_back.system_power = pmsg->paras[2];
		//sst_info.power_state = power_chk_back;
		memcpy((void *)&power_chk_back, (void *)pmsg->paras, sizeof(struct sst_power_info_para));
		memcpy((void *)&sst_info.power_state, (void *)pmsg->paras, sizeof(struct sst_power_info_para));
		LOG("standby power check enable:0x%x, regs:0x%x, sys_power:%d\n", \
			sst_info.power_state.enable, \
			sst_info.power_state.power_reg, \
			sst_info.power_state.system_power);

		return 0;
	}
	/* read operation */
	pmsg->paras[0] = sst_info.power_state.enable;
	pmsg->paras[1] = sst_info.power_state.power_reg;
	pmsg->paras[2] = sst_info.power_state.system_power;

	return 0;
}
#endif
#endif /* STANDBY_USED */
