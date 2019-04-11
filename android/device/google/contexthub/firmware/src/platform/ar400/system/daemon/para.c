/*
*********************************************************************************************************
*                                                AR200 SYSTEM
*                                     AR200 Software System Develop Kits
*                                                startup module
*
*                                    (c) Copyright 2012-2016, superm China
*                                             All Rights Reserved
*
* File    : para.c
* By      : superm
* Version : v1.0
* Date    : 2012-5-13
* Descript: startup module.
* Update  : date                auther      ver     notes
*           2013-5-23 9:32:20   superm      1.0     Create this file.
*********************************************************************************************************
*/

#include "daemon_i.h"
//#include "para.h"


#if defined TF_USED
struct arisc_para arisc_para __attribute__ ((section("dts_paras"))) = {
};
extern char paras_start_addr, paras_end_addr;
#define PARAS_START_ADDR  (char  *)(&paras_start_addr)
#define PARAS_END_ADDR  (char  *)(&paras_end_addr)
#elif defined KERNEL_USED
struct arisc_para arisc_para;
#endif
/*
*********************************************************************************************************
*                                       C STARTUP ENTRY
*
* Description:  the c entry of startup.
*
* Arguments  :  none.
*
* Returns    :  none.
*********************************************************************************************************
*/
void arisc_para_init(void)
{
#if defined TF_USED
	//cpucfg_set_little_endian_address((void *)PARAS_START_ADDR, (void *)PARAS_END_ADDR);
	//cpucfg_set_little_endian_address((void *)arisc_para.standby_base, (void *)arisc_para.standby_base+arisc_para.standby_size);
	//copy arisc initialize parameters from cpux system
#elif defined KERNEL_USED
	//copy arisc initialize parameters from cpux system
	memcpy((void *)(&arisc_para), (void *)ARISC_PARA_ADDR, ARISC_PARA_SIZE);
#else
	arisc_para.suart_status = 0;
#endif
}

void set_paras(void)
{
#if defined TF_USED
	//copy arisc initialize parameters from cpux system
	//hexdump("para", (char *)(&arisc_para), ARISC_PARA_SIZE);

	//init uart defaultly, if sys_config.fex disable s_uart, should disable s_uart here.
//	if (!arisc_para_get_uart_pin_info())
//		uart_exit();
#if IR_USED
	ir_set_paras(&arisc_para.ir_key);
#endif

#elif defined KERNEL_USED
	//copy arisc initialize parameters from cpux system
	memcpy((void *)(&arisc_para), (void *)ARISC_PARA_ADDR, ARISC_PARA_SIZE);
#endif
}

int arisc_para_get_message_pool_info(unsigned int *addr, unsigned int *size)
{
	*addr = arisc_para.message_pool_phys;
	*size = arisc_para.message_pool_size;
	*addr = 0x48105000;
	*size = 0x00001000;
	LOG("%s: %x %x\n", __func__, *addr, *size);

	return OK;
}

#if defined TF_USED
int arisc_para_get_standby_para_info(unsigned int *addr, unsigned int *size)
{
	*addr = arisc_para.standby_base;
	*size = arisc_para.standby_size;
	LOG("%s: %x %x\n", __func__, *addr, *size);

	return OK;
}
#endif // TF_USED

int arisc_para_get_uart_pin_info(void)
{
	return arisc_para.suart_status;
}
