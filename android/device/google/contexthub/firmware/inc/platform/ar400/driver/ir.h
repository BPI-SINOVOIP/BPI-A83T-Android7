/*
 * include/drivers/ir.h
 *
 * Copyright (C) 2012-2016 AllWinnertech Ltd.
 * Author: Sunny <Sunny@allwinnertech.com>
 *
 */
#ifndef __IR_H__
#define __IR_H__

typedef struct ir_code
{
	u32 key_code;
	u32 addr_code;
} ir_code_t;

typedef struct ir_key
{
	u32 num;
	ir_code_t ir_code_depot[IR_NUM_KEY_SUP];
} ir_key_t;

#if IR_USED

extern s32 ir_init(void);
extern s32 ir_exit(void);
extern u32 ir_is_power_key(void);

#if (defined KERNEL_USED)
extern s32 ir_set_paras(struct message *pmessage);
#elif (defined TF_USED)
extern s32 ir_set_paras(ir_key_t *ir_key);
extern void ir_sysconfig_cfg(void);
extern u32 ir_is_used(void);
#else
extern s32 ir_set_paras(ir_code_t *ir_code, int index);
extern void ir_sysconfig_cfg(void);
extern u32 ir_is_used(void);
#endif

#endif
#endif  //__IR_H__
