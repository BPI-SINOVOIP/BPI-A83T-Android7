/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <seos.h>
#include <plat/inc/include.h>

#define VEC_(nm, pfx)    void nm##pfx(void) __attribute__ ((weak, alias ("IntDefaultHandler")))
#define VEC(nm)        VEC_(nm, _Handler)
#define VECI(nm)    VEC_(nm, _IRQHandler)



#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE 10240
#endif

void __attribute__ ((weak)) IntDefaultHandler(void);
VEC(NMI);
VEC(HardFault);
VEC(MemoryManagemntFault);
VEC(BusFault);
VEC(UsageFault);
VEC(SVC);
VEC(DebugMonitor);
VEC(PendSV);
VEC(SysTick);

VECI(UART0);
VECI(UART1);
VECI(UART2);
VECI(UART3);
VECI(I2C0);
VECI(I2C1);
VECI(I2C2);
VECI(I2C3);
VECI(I2C4);
VECI(SPI0);
VECI(SPI1);
VECI(GPADC);
VECI(THS);
VECI(LRADC);
VECI(I2S0);
VECI(I2S1);
VECI(I2S2);
VECI(DMIC);
VECI(ACDET);
VECI(AC);
VECI(USB_DRD_DEV);
VECI(USB_DRD_EHCI);
VECI(USB_DRD_OHCI);
VECI(USB_HOST_EHCI);
VECI(USB_HOST_OHCI);
VECI(DRAM);
VECI(NAND);
VECI(SMHC0);
VECI(SMHC1);
VECI(SMHC2);
VECI(IOMMU);
VECI(PSI);
VECI(DMA);
VECI(MBOX);
VECI(SPINLOCK);
VECI(HSTIMER0);
VECI(SMC);
VECI(TIMER0);
VECI(TIMER1);
VECI(WDG);
VECI(GPIOB);
VECI(GPIOF);
VECI(GPIOG);
VECI(GPIOH);
VECI(CLK_DET);
VECI(BUS_TIMEOUT);
VECI(MIPI_DSI0);
VECI(MIPI_DSI1);
VECI(TCON_LCD0);
VECI(TCON_LCD1);
VECI(EDP_TX0);
VECI(CSI0_DMA0);
VECI(CSI0_DMA1);
VECI(CSI0_PARSER0);
VECI(CSI0_CCI0);
VECI(DE);
VECI(DE_ROT);
VECI(GPU0_IRQGPU);
VECI(GPU1_IRQJOB);
VECI(GPU2_IRQMMU);
VECI(CE_NS);
VECI(CE_S);
VECI(VE);
VECI(VP9);
VECI(CPUIDLE);
VECI(R_GPIOM);
VECI(EXT_NMI);
VECI(R_TIMER0);
VECI(R_TIMER1);
VECI(R_TIMER2);
VECI(R_TIMER3);
VECI(R_ALARM0);
VECI(R_ALARM1);
VECI(R_WDG);
VECI(R_TWDG);
VECI(R_GPIOL);
VECI(R_UART0);
VECI(R_I2C0);
VECI(R_I2C1);
VECI(R_I2C2);
VECI(R_RSB);
VECI(R_SPI);
VECI(GIC_OUT_C0);


//stack top (provided by linker)
extern uint32_t __stack_top[];
extern uint32_t __data_data[];
extern uint32_t __data_start[];
extern uint32_t __data_end[];
extern uint32_t __bss_start[];
extern uint32_t __bss_end[];




//OS stack
uint64_t __attribute__ ((section (".stack"))) _STACK[OS_STACK_SIZE / sizeof(uint64_t)];

void __attribute__((noreturn)) IntDefaultHandler(void)
{
    while (1) {
        //ints off
        asm("cpsid i");

        //spin/sleep/whatever forever
        asm("wfi":::"memory");
    }
}

void __attribute__((noreturn)) ResetISR(void);
void __attribute__((noreturn)) ResetISR(void)
{
    uint32_t *dst, *src, *end;

    //copy data
    dst = __data_start;
    src = __data_data;
    end = __data_end;
    while(dst != end)
        *dst++ = *src++;

    //init bss
    dst = __bss_start;
    end = __bss_end;
    while(dst != end)
        *dst++ = 0;

    //call code
    osMain();

    //if main returns => bad
    while(1);
}


//vector table
__attribute__ ((section(".vectors"))) __attribute__((naked)) void __VECTORS(void);
__attribute__ ((section(".vectors"))) __attribute__((naked)) void __VECTORS(void)
{
    asm volatile (
        ".word __stack_top                      \n"
        ".word ResetISR + 1                     \n"
        ".word NMI_Handler + 1                  \n"
        ".word HardFault_Handler + 1            \n"
        ".word MemoryManagemntFault_Handler + 1 \n"
        ".word BusFault_Handler + 1             \n"
        ".word UsageFault_Handler + 1           \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word SVC_Handler + 1                  \n"
        ".word DebugMonitor_Handler + 1         \n"
        ".word 0                                \n"
        ".word PendSV_Handler + 1               \n"
        ".word SysTick_Handler + 1              \n"

        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry                  \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word 0                                \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
        ".word interrupt_entry + 1              \n"
    );
};


