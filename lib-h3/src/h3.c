/**
 * @file h3.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3.h"

#define MCTL_COM_BASE	0x01c62000
#define MC_CR			(MCTL_COM_BASE + 0x0)

inline static uint32_t _mctl_read(uint32_t reg) {
	return *(volatile uint32_t *) (reg);
}

// https://github.com/allwinner-zh/bootloader/blob/e5ceeca211883d9f6b25f84e0e3c5fe2afaaf350/basic_loader/bsp/bsp_for_a33/init_dram/mctl_hal.c#L1107
uint32_t h3_get_dram_size(void) {
	uint32_t value;
	uint32_t dram_size;
	uint32_t temp;

	value = _mctl_read(MC_CR);

	temp = (value >> 8) & 0xf;	//page size code
	dram_size = (temp - 6);			//(1<<dram_size) * 512Bytes

	temp = (value >> 4) & 0xf;	//row width code
	dram_size += (temp + 1);		//(1<<dram_size) * 512Bytes

	temp = (value >> 2) & 0x3;	//bank number code
	dram_size += (temp + 2);		//(1<<dram_size) * 512Bytes

	temp = value & 0x3;			//rank number code
	dram_size += temp;				//(1<<dram_size) * 512Bytes

	dram_size = dram_size - 11;		//(1<<dram_size)MBytes

	return (1 << dram_size);
}

//https://github.com/linux-sunxi/sunxi-tools/blob/master/uart0-helloworld-sdboot.c#L458
h3_boot_device_t h3_get_boot_device(void) {
	uint32_t *spl_signature = (void *) 0x4;

	/* Check the eGON.BT0 magic in the SPL header */
	if (spl_signature[0] != 0x4E4F4765 || spl_signature[1] != 0x3054422E)
		return H3_BOOT_DEVICE_FEL;

	const uint32_t boot_dev = spl_signature[9] & 0xFF; /* offset into SPL = 0x28 */

	if (boot_dev == 0) {
		return H3_BOOT_DEVICE_MMC0;
	}

	if (boot_dev == 3) {
		return H3_BOOT_DEVICE_SPI;
	}

	return H3_BOOT_DEVICE_UNK;
}

extern uint32_t __ram_start; /* Defined by the linker */
extern uint32_t __ram_end; /* Defined by the linker */
extern uint32_t __data_start; /* Defined by the linker */
extern uint32_t __data_end; /* Defined by the linker */
extern uint32_t __bss_start; /* Defined by the linker */
extern uint32_t __bss_end; /* Defined by the linker */
extern uint32_t __stack_start; /* Defined by the linker */
extern uint32_t __heap_start; /* Defined by the linker */
extern uint32_t heap_low; /* Defined by the linker */
extern uint32_t heap_top; /* Defined by the linker */
extern uint32_t __und_stack_top; /* Defined by the linker */
extern uint32_t __abt_stack_top; /* Defined by the linker */
extern uint32_t __fiq_stack_top; /* Defined by the linker */
extern uint32_t __irq_stack_top; /* Defined by the linker */
extern uint32_t __svc_stack_top; /* Defined by the linker */
extern uint32_t __sys_stack_top; /* Defined by the linker */

void h3_memory_map_dump(void) {
#ifndef NDEBUG
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
	printf("CPUCFG    %p\n", (void *)H3_CPUCFG_BASE);
	printf("PIO_PORTL %p\n", (void *)H3_PIO_PORTL_BASE);
#endif
}
