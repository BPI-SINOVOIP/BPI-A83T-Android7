#ifndef __MEM_TIMER_SUN50IW3_H__
#define __MEM_TIMER_SUN50IW3_H__

#if defined CONFIG_ARCH_SUN50IW3P1

typedef struct __MEM_TIMER_REG
{
	// offset:0x00
	volatile unsigned int   IntCtl;
	volatile unsigned int   IntSta;
	volatile unsigned int   reserved0[2];
	// offset:0x10
	volatile unsigned int   Tmr0Ctl;
	volatile unsigned int   Tmr0IntVal;
	volatile unsigned int   Tmr0CntVal;
	volatile unsigned int   reserved1;
	// offset:0x20
	volatile unsigned int   Tmr1Ctl;
	volatile unsigned int   Tmr1IntVal;
	volatile unsigned int   Tmr1CntVal;
	volatile unsigned int   reserved2;
	// offset:0x30
	volatile unsigned int   Tmr2Ctl;
	volatile unsigned int   Tmr2IntVal;
	volatile unsigned int   Tmr2CntVal;
	volatile unsigned int   reserved3;
	// offset:0x40
	volatile unsigned int   Tmr3Ctl;
	volatile unsigned int   Tmr3IntVal;
	volatile unsigned int   reserved4[2];
	// offset:0x50
	volatile unsigned int   Tmr4Ctl;
	volatile unsigned int   Tmr4IntVal;
	volatile unsigned int   Tmr4CntVal;
	volatile unsigned int   reserved5;
	// offset:0x60
	volatile unsigned int   Tmr5Ctl;
	volatile unsigned int   Tmr5IntVal;
	volatile unsigned int   Tmr5CntVal;
	volatile unsigned int   reserved6[5];
	// offset:0x80
	volatile unsigned int   AvsCtl;
	volatile unsigned int   Avs0Cnt;
	volatile unsigned int   Avs1Cnt;
	volatile unsigned int   AvsDiv;

	// offset:0x90: reserved
	volatile unsigned int   reserved7[4];

	// offset:0xa0
	volatile unsigned int   WDog1_Irq_En;
	volatile unsigned int   WDog1_Irq_Sta;
	volatile unsigned int   reserved8[2];

	// offset:0xb0
	volatile unsigned int   WDog1_Ctrl_Reg;
	volatile unsigned int   WDog1_Cfg_Reg;
	volatile unsigned int   WDog1_Mode_Reg;

} mem_timer_reg_t;
#endif /* CONFIG_ARCH_SUN50IW3P1 */
#endif /* __MEM_TIMER_SUN50IW3_H__ */
