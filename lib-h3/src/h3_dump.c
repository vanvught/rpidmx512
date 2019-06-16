/**
 * @file h3_dump.c
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdio.h>

#include "h3.h"

/* Defined by the linker */
extern uint32_t __ram_start;
extern uint32_t __ram_end;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __stack_start;
extern uint32_t __heap_start;
extern uint32_t heap_low;
extern uint32_t heap_top;
extern uint32_t __und_stack_top;
extern uint32_t __abt_stack_top;
extern uint32_t __fiq_stack_top;
extern uint32_t __irq_stack_top;
extern uint32_t __svc_stack_top;
extern uint32_t __sys_stack_top;

void h3_dump_memory_map(void) {
	printf("memmap\n");
	printf("__ram_start = %p\n", &__ram_start);
	printf("  __data_start = %p\n", &__data_start);
	printf("   __data_end = %p\n", &__data_end);
	printf("  __bss_start = %p\n", &__bss_start);
	printf("   __bss_end = %p\n", &__bss_end);
	printf("  __stack_start = %p\n", &__stack_start);
	printf("    __und_stack_top = %p\n", &__und_stack_top);
	printf("    __abt_stack_top = %p\n", &__abt_stack_top);
	printf("    __fiq_stack_top = %p\n", &__fiq_stack_top);
	printf("    __irq_stack_top = %p\n", &__irq_stack_top);
	printf("    __svc_stack_top = %p\n", &__svc_stack_top);
	printf("    __sys_stack_top = %p\n", &__sys_stack_top);
	printf("  __heap_start = %p\n", &__heap_start);
	printf("    heap_low = %p\n", &heap_low);
	printf("    heap_top = %p\n", &heap_top);
	printf("__ram_end = %p\n", &__ram_end);
	//
	printf("4.1 Memory Mapping\n");
	printf("DMA       %p\n", (void *)H3_DMA_BASE);
	printf("SD/MMC0   %p\n", (void *)H3_SD_MMC0_BASE);
	printf("SD/MMC1   %p\n", (void *)H3_SD_MMC1_BASE);
	printf("SD/MMC2   %p\n", (void *)H3_SD_MMC2_BASE);
	printf("SID       %p\n", (void *)H3_SID_BASE);
	printf("CCU       %p\n", (void *)H3_CCU_BASE);
	printf("PIO       %p\n", (void *)H3_PIO_BASE);
	printf("TIMER     %p\n", (void *)H3_TIMER_BASE);
	printf("AC        %p\n", (void *)H3_AC_BASE);
	printf("THS       %p\n", (void *)H3_THS_BASE);
	printf("UART0     %p\n", (void *)H3_UART0_BASE);
	printf("UART1     %p\n", (void *)H3_UART1_BASE);
	printf("UART2     %p\n", (void *)H3_UART2_BASE);
	printf("UART3     %p\n", (void *)H3_UART3_BASE);
	printf("TWI0      %p\n", (void *)H3_TWI0_BASE);
	printf("TWI1      %p\n", (void *)H3_TWI1_BASE);
	printf("TWI2      %p\n", (void *)H3_TWI2_BASE);
	printf("EMAC      %p\n", (void *)H3_EMAC_BASE);
	printf("GPU       %p\n", (void *)H3_GPU_BASE);
	printf("SPI0      %p\n", (void *)H3_SPI0_BASE);
	printf("SPI1      %p\n", (void *)H3_SPI1_BASE);
	//
	printf("GIC       %p\n", (void *)H3_GIC_BASE);
	printf("PRCM      %p\n", (void *)H3_PRCM_BASE);
	printf("CPUCFG    %p\n", (void *)H3_CPUCFG_BASE);
	printf("PIO_PORTL %p\n", (void *)H3_PIO_PORTL_BASE);
}
