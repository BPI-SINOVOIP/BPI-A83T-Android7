#include <plat/inc/include.h>
#include <plat/inc/cmsis.h>
#if MEM_USED

s32 mem_int_enable(u32 intno)
{
	if (intno == EXT_NMI_IRQn) {
		NVIC_ClearPendingIRQ(intno);
	}

	//enable interrupt which number is intno
	if (intno < NUM_INTERRUPTS) {
		NVIC_EnableIRQ(intno);
	}

	return OK;
}

s32 mem_int_disable(u32 intno)
{
	if (intno < NUM_INTERRUPTS) {
		NVIC_DisableIRQ(intno);
	}

	return OK;
}

int mem_int_suspend_cfg(struct super_standby_para *para)
{
	mem_gpio_suspend_cfg(para);

#if  IR_USED
	if (para->event & CPUS_WAKEUP_IR) {
		//enable cir intterrupt wakeup
		//mem_long_jump((mem_long_jump_fn)ir_init, 0);
		ir_init();
		mem_int_enable(INTC_R_CIR_IRQ);
	}
#endif

#if (defined CONFIG_ARCH_SUN8IW9P1) || \
	(defined CONFIG_ARCH_SUN50IW1P1) || \
	(defined CONFIG_ARCH_SUN50IW2P1) || \
	(defined CONFIG_ARCH_SUN50IW3P1) || \
	(defined CONFIG_ARCH_SUN50IW6P1)
	if (para->event & CPUS_WAKEUP_ALM0) {
		//enable alarm0 intterrupt wakeup
		mem_int_enable(R_ALARM0_IRQn);
	}
	if (para->event & CPUS_WAKEUP_ALM1) {
		//enable alarm1 intterrupt wakeup
		mem_int_enable(R_ALARM1_IRQn);
	}
#endif

	if (para->event & CPUS_WAKEUP_TIMEOUT) {
		//init timer standby process
		//mem_long_jump((mem_long_jump_fn)timer_standby_init, 0);
		timer_standby_init();
	}
	if (para->event & CPUS_WAKEUP_USBMOUSE) {
		mem_int_enable(USB_DRD_DEV_IRQn);
		mem_int_enable(USB_DRD_EHCI_IRQn);
		mem_int_enable(USB_DRD_OHCI_IRQn);
		mem_int_enable(USB_HOST_EHCI_IRQn);
		mem_int_enable(USB_HOST_OHCI_IRQn);
	}
	if (para->event & CPUS_WAKEUP_CODEC) {
		mem_int_enable(AC_IRQn);
		mem_int_enable(ACDET_IRQn);
	}
	if (para->event & CPUS_WAKEUP_LRADC) {
		mem_int_enable(LRADC_IRQn);
	}

	return OK;
}

int mem_int_resume_cfg(struct super_standby_para *para)
{
#if  IR_USED
	if (para->event & CPUS_WAKEUP_IR) {
		//exit ir standby process
		//mem_long_jump((mem_long_jump_fn)ir_exit, 0);
		ir_exit();
		mem_int_disable(INTC_R_CIR_IRQ);
	}
#endif // IR_USED
	if (para->event & CPUS_WAKEUP_TIMEOUT) {
		//exit timer standby process
		//mem_long_jump((mem_long_jump_fn)timer_standby_exit, 0);
		timer_standby_exit();
	}
	mem_gpio_resume_cfg(para);

	if ((para->event & CPUS_WAKEUP_NMI) || (para->event & CPUS_WAKEUP_GPIO)) {
		//exit exteral intterrupt standby process
		mem_pmu_standby_exit();
	}
	if (para->event & CPUS_WAKEUP_USBMOUSE) {
		mem_int_disable(USB_DRD_DEV_IRQn);
		mem_int_disable(USB_DRD_EHCI_IRQn);
		mem_int_disable(USB_DRD_OHCI_IRQn);
		mem_int_disable(USB_HOST_EHCI_IRQn);
		mem_int_disable(USB_HOST_OHCI_IRQn);
	}
	if (para->event & CPUS_WAKEUP_CODEC) {
		mem_int_disable(AC_IRQn);
		mem_int_disable(ACDET_IRQn);
	}
	if (para->event & CPUS_WAKEUP_LRADC) {
		mem_int_disable(LRADC_IRQn);
	}

	//restore interrupt module
	//mem_long_jump((mem_long_jump_fn)interrupt_standby_exit, 0);
	interrupt_standby_exit();

	return OK;
}

#endif /* MEM_USED */
