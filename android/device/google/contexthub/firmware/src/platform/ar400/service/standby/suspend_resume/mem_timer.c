#include <plat/inc/include.h>

#if  0/*mem timer no use*/
#if  MEM_USED
#if defined CONFIG_ARCH_SUN8IW6P1
#include "mem_timer_sun8iw6.h"
#elif defined CONFIG_ARCH_SUN50IW1P1
#include "mem_timer_sun50iw1.h"
#elif defined CONFIG_ARCH_SUN50IW2P1
#include "mem_timer_sun50iw2.h"
#elif defined CONFIG_ARCH_SUN50IW3P1
#include "mem_timer_sun50iw3.h"
#endif


static mem_timer_reg_t  *TmrReg;

//static unsigned int WatchDog1_Config_Reg_Bak, WatchDog1_Mod_Reg_Bak, WatchDog1_Irq_En_Bak;

int mem_timer_save(void)
{
	TmrReg = (mem_timer_reg_t *)IO_ADDRESS(SUNXI_TIMER_PBASE);

	system_back->timer_back.IntCtl   = TmrReg->IntCtl;
	system_back->timer_back.Tmr0Ctl     = TmrReg->Tmr0Ctl;
	system_back->timer_back.Tmr0IntVal  = TmrReg->Tmr0IntVal;
	system_back->timer_back.Tmr0CntVal  = TmrReg->Tmr0CntVal;
	system_back->timer_back.Tmr1Ctl     = TmrReg->Tmr1Ctl;
	system_back->timer_back.Tmr1IntVal  = TmrReg->Tmr1IntVal;
	system_back->timer_back.Tmr1CntVal  = TmrReg->Tmr1CntVal;

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_TIMER_PBASE), sizeof(__mem_timer_reg_t)/4);

	return OK;
}

int mem_timer_restore(void)
{
	TmrReg->Tmr0IntVal  = system_back->timer_back.Tmr0IntVal;
	TmrReg->Tmr0CntVal  = system_back->timer_back.Tmr0CntVal;
	TmrReg->Tmr0Ctl     = system_back->timer_back.Tmr0Ctl;
	TmrReg->Tmr1IntVal  = system_back->timer_back.Tmr1IntVal;
	TmrReg->Tmr1CntVal  = system_back->timer_back.Tmr1CntVal;
	TmrReg->Tmr1Ctl     = system_back->timer_back.Tmr1Ctl;
	TmrReg->IntCtl      = system_back->timer_back.IntCtl;

	//mem_reg_debug(__func__, IO_ADDRESS(SUNXI_TIMER_PBASE), sizeof(__mem_timer_reg_t)/4);

	return OK;
}

#endif /* MEM_USED */
#endif
