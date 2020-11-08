/**
 * @file arm_dump_memmap.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

extern int uart0_printf(const char* fmt, ...);
#define printf uart0_printf

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
extern uint32_t __svc_stack_top_core1;
extern uint32_t __svc_stack_top_core2;
extern uint32_t __svc_stack_top_core3;

void __attribute__((cold)) arm_dump_memmap(void) {
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
	printf("    __svc_stack_top_core1 = %p\n", &__svc_stack_top_core1);
	printf("    __svc_stack_top_core2 = %p\n", &__svc_stack_top_core2);
	printf("    __svc_stack_top_core3 = %p\n", &__svc_stack_top_core3);
	printf("  __heap_start = %p\n", &__heap_start);
	printf("    heap_low = %p\n", &heap_low);
	printf("    heap_top = %p\n", &heap_top);
	printf("__ram_end = %p\n", &__ram_end);
}
