/*
*********************************************************************************************************
*                                                AR100 SYSTEM
*                                     AR100 Software System Develop Kits
*                                                check module
*
*                                    (c) Copyright 2012-2016, Superm China
*                                             All Rights Reserved
*
* File    : dvfs.c
* By      : Superm
* Version : v1.0
* Date    : 2015-1-19
* Descript: check module.
* Update  : date                auther      ver     notes
*           2015-1-19 8:58:37   Superm      1.0     Create this file.
*********************************************************************************************************
*/

#include "check_i.h"

#if CHECK_USED
s32 get_pmu_package(u32 *package)
{
	u8 regaddr;
	u32 data;

	data = 1;
	rsb_write(RSB_RTSADDR_AW1660, 0xff, &data, RSB_DATA_TYPE_BYTE);
	rsb_read(RSB_RTSADDR_AW1660, PMU_PACKAGE_REG, &package, RSB_DATA_TYPE_BYTE);
	data = 0;
	rsb_write(RSB_RTSADDR_AW1660, 0xff, &data, RSB_DATA_TYPE_BYTE);

	return OK;
}

static int check_soc_package(void)
{
	int package = 0;

	package = ((readl(IC_CHIP_ID_ADDR) >> 10) & 0x3F);			//[15:10] 6bit
	if((package == IC_CHIP_ID) || (package == IC_CHIP_ID2))
	{
		INF("using ic %s\n", IC_NAME);
		return OK;
	}
	else if(package == IC_DEFAULT_CHIP_ID)
	{
		INF("ic use default id\n");
		return OK;
	}

	INF("ic check fail,IC must=%s,but id=0x%x\n", IC_NAME, package);
	return -EFAIL;
}

static int check_pmu_package(void)
{
	u32 package = 0;

	if(get_pmu_package(&package))
	{
		return -EFAIL;
	}

	if (AXP_CHIP_ID == package)
	{
		INF("using axp %s\n", AXP_NAME);
		return OK;
	} else if (AXP_DEFAULT_CHIP_ID == package) {
		INF("axp use default id\n");
		return OK;
	}
	INF("axp check fail:axp must %s, but id=0x%x\n", AXP_NAME, package);

	return -EFAIL;
}

/* H8 binding axp818, A83T bingding AXP813 */
s32 check_board(void)
{
	INF("ID CHECK VERSION: %s\n", ID_VERSION);
	if(!check_soc_package())
	{
		if(!check_pmu_package()) {
			return OK;
		}
	}

	save_state_flag(REC_SHUTDOWN);
	pmu_shutdown();
	while(1);

	return -EFAIL;
}

#endif /* CHECK_USED */
