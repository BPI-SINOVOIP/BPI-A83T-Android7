#include <plat/inc/include.h>

#if MEM_USED
#include "mem_spc.h"

int mem_spc_save(void)
{
	mem_reg_save(system_back->spc_back.spc_reg_back, \
	             (const unsigned int *)IO_ADDRESS(SUNXI_SPC_PBASE + SPC_REG_START), SPC_REG_LENGTH, \
	             SPC_SKIP);

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_SPC_PBASE), SPC_REG_LENGTH * SPC_SKIP);

	return OK;
}

int mem_spc_restore(void)
{
	/*
	 * default is secure, so we only need restore no-secure
	 */
	mem_reg_restore((unsigned int *)IO_ADDRESS(SUNXI_SPC_PBASE + SPC_REG_START + 0x04), \
	                (const unsigned int *)system_back->spc_back.spc_reg_back, SPC_REG_LENGTH, \
	                SPC_SKIP);

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_SPC_PBASE), SPC_REG_LENGTH * SPC_SKIP);

	return OK;
}

#endif /* STANDBY_USED */
