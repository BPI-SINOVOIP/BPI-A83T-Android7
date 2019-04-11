#ifndef __MEM_TMSTMP_SUN8IW6_H__
#define __MEM_TMSTMP_SUN8IW6_H__

#define TMSTMP_REG_LENGTH      (((0x20 + 0x04)>>2))

typedef struct mem_tmstmp_state_reg
{
	volatile u32   low;           // offset:0x00
	volatile u32   high;          // offset:0x04
} mem_tmstmp_state_reg_t;

typedef struct mem_tmstmp_ctrl_reg
{
	volatile u32   ctrl;          // offset:0x00
	volatile u32   reserved0[1];  // offset:0x04
	volatile u32   low;           // offset:0x08
	volatile u32   high;          // offset:0x0c
	volatile u32   reserved1[4];  // offset:0x10
	volatile u32   freq;          // offset:0x20
} mem_tmstmp_ctrl_reg_t;
#endif /* __MEM_TMSTMP_SUN8IW6_H__ */
