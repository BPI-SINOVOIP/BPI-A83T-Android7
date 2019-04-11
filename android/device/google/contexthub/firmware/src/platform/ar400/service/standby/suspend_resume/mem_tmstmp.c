#include <plat/inc/include.h>

#if MEM_USED
#include "mem_tmstmp.h"

static mem_tmstmp_state_reg_t  *PTmstmpStateReg;
static mem_tmstmp_ctrl_reg_t  *PTmstmpCtrlReg;

s32 mem_tmstmp_save(void)
{
	PTmstmpStateReg = (mem_tmstmp_state_reg_t *)IO_ADDRESS(SUNXI_TIMESTAMP_STATE_PBASE);
	PTmstmpCtrlReg = (mem_tmstmp_ctrl_reg_t *)IO_ADDRESS(SUNXI_TIMESTAMP_CTRL_PBASE);

	system_back->tmstmp_back.low = PTmstmpStateReg->low;
	system_back->tmstmp_back.high = PTmstmpStateReg->high;

	system_back->tmstmp_back.freq = PTmstmpCtrlReg->freq;

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_TIMESTAMP_CTRL_PBASE), TMSTMP_REG_LENGTH);

	return OK;
}

s32 mem_tmstmp_restore(void)
{
	PTmstmpCtrlReg->ctrl = 0;                             //disable timestamp firstly
	PTmstmpCtrlReg->freq = system_back->tmstmp_back.freq; //restore freq
	PTmstmpCtrlReg->low = system_back->tmstmp_back.low;   //restore low
	PTmstmpCtrlReg->high = system_back->tmstmp_back.high; //restore high
	PTmstmpCtrlReg->ctrl = 1;                             //enable timestamp firstly
	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_TIMESTAMP_CTRL_PBASE), TMSTMP_REG_LENGTH);

	return OK;
}

#endif /* STANDBY_USED */
