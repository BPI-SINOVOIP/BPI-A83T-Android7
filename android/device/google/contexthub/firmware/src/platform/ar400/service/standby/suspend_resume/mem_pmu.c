#include <plat/inc/include.h>

#if MEM_USED
#include "mem_pmu.h"

/* backup pmu interrupt enable registers */
static u8 int_enable_regs[6];

s32 mem_pmu_standby_init(struct super_standby_para *para)
{
	u8  devaddr[6];
	u8  regaddr[6];
	u8  enable[6];
	u32 len = 6;
	pmu_paras_t pmu_para;
	u32 event = para->event;
	u32 gpio_bitmap = para->gpio_enable_bitmap;

	devaddr[0] = RSB_RTSADDR_AW1660;
	devaddr[1] = RSB_RTSADDR_AW1660;
	devaddr[2] = RSB_RTSADDR_AW1660;
	devaddr[3] = RSB_RTSADDR_AW1660;
	devaddr[4] = RSB_RTSADDR_AW1660;
	devaddr[5] = RSB_RTSADDR_AW1660;
	//read pmu irq enable registers
	regaddr[0] = AW1660_IRQ_ENABLE1;
	regaddr[1] = AW1660_IRQ_ENABLE2;
	regaddr[2] = AW1660_IRQ_ENABLE3;
	regaddr[3] = AW1660_IRQ_ENABLE4;
	regaddr[4] = AW1660_IRQ_ENABLE5;
	regaddr[5] = AW1660_IRQ_ENABLE6;

	pmu_para.devaddr = devaddr;
	pmu_para.regaddr = regaddr;
	pmu_para.data = enable;
	pmu_para.len = len;
	//mem_long_jump((mem_long_jump_fn)pmu_reg_read_para, (u32)&pmu_para);
	pmu_reg_read_para(&pmu_para);

	//backup enable registers
	mem_memcpy(int_enable_regs, enable, len);

	//clear enable array firstly
	mem_memset(enable, 0, len);

	//initialize pmu wakeup events
	if (event & CPUS_WAKEUP_USB) {
		//AW1660_IRQ_ENABLE1,
		//BIT2 : VBUS from high to low (USB plugin).
		//BIT3 : VBUS from low to high (USB plugout).
		enable[0] |= (0x1 << 2);
		enable[0] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_AC) {
		//AW1660_IRQ_ENABLE1,
		//BIT5 : ACIN from high to low (ACIN plugin).
		//BIT6 : ACIN from low to high (ACIN plugout).
		enable[0] |= (0x1 << 5);
		enable[0] |= (0x1 << 6);
	}
	if (event & CPUS_WAKEUP_FULLBATT) {
		//AW1660_IRQ_ENABLE2,
		//BIT2 : battary is full/charge done.
		enable[1] |= (0x1 << 2);
	}
	if (event & CPUS_WAKEUP_BAT_TEMP) {
		//AW1660_IRQ_ENABLE3,
		//BIT1 : battery temperature under the safe range IRQ.
		//BIT3 : battery temperature over the safe range IRQ.
		enable[2] |= (0x1 << 1);
		enable[2] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_LOWBATT) {
		//AW1660_IRQ_ENABLE4,
		//BIT0 : level2 low battary,
		//BIT1 : level1 low battary.
		enable[3] |= (0x1 << 0);
		enable[3] |= (0x1 << 1);
	}
	if (gpio_bitmap & CPUS_GPIO_AXP(0)) {
		//AW1660_IRQ_ENABLE5,
		//BIT0 : GPIO0 input edge.
		enable[4] |= (0x1 << 0);
	}
	if (gpio_bitmap & CPUS_GPIO_AXP(1)) {
		//AW1660_IRQ_ENABLE5,
		//BIT1 : GPIO1 input edge.
		enable[4] |= (0x1 << 1);
	}
	if (event & CPUS_WAKEUP_LONG_KEY) {
		//AW1660_IRQ_ENABLE5,
		//BIT3 : long key.
		enable[4] |= (0x1 << 3);
	}
	if (event & CPUS_WAKEUP_SHORT_KEY) {
		//AW1660_IRQ_ENABLE5,
		//BIT4 : short key.
		enable[4] |= (0x1 << 4);
	}
	if (event & CPUS_WAKEUP_DESCEND) {
		//AW1660_IRQ_ENABLE5,
		//BIT5 : POK negative.
		enable[4] |= (0x1 << 5);
	}
	if (event & CPUS_WAKEUP_ASCEND) {
		//AW1660_IRQ_ENABLE5,
		//BIT6 : POK postive.
		enable[4] |= (0x1 << 6);
	}

	//write pmu enable registers
	//mem_long_jump((mem_long_jump_fn)pmu_reg_write_para, (u32)&pmu_para);
	pmu_reg_write_para(&pmu_para);

	return OK;
}

s32 mem_pmu_standby_exit(void)
{
	u8 devaddr[6];
	u8 regaddr[6];
	u32 len = 6;
	pmu_paras_t pmu_para;

	//restore pmu irq enable registers
	devaddr[0] = RSB_RTSADDR_AW1660;
	devaddr[1] = RSB_RTSADDR_AW1660;
	devaddr[2] = RSB_RTSADDR_AW1660;
	devaddr[3] = RSB_RTSADDR_AW1660;
	devaddr[4] = RSB_RTSADDR_AW1660;
	devaddr[5] = RSB_RTSADDR_AW1660;
	regaddr[0] = AW1660_IRQ_ENABLE1;
	regaddr[1] = AW1660_IRQ_ENABLE2;
	regaddr[2] = AW1660_IRQ_ENABLE3;
	regaddr[3] = AW1660_IRQ_ENABLE4;
	regaddr[4] = AW1660_IRQ_ENABLE5;
	regaddr[5] = AW1660_IRQ_ENABLE6;

	pmu_para.devaddr = devaddr;
	pmu_para.regaddr = regaddr;
	pmu_para.data = int_enable_regs;
	pmu_para.len = len;
	//mem_long_jump((mem_long_jump_fn)pmu_reg_write_para, (u32)&pmu_para);
	pmu_reg_write_para(&pmu_para);

	return OK;
}
#endif /* MEM_USED */
