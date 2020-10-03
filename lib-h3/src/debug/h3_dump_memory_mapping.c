/**
 * @file h3_dump_memory_mapping.c
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

#include "h3.h"
#include "h3_uart0_debug.h"

void __attribute__((cold)) h3_dump_memory_mapping(void) {
	uart0_printf("4.1 Memory Mapping\n");
	uart0_printf("DE        %p\n", (void *)H3_DE_BASE);
	uart0_printf("DMA       %p\n", (void *)H3_DMA_BASE);
	uart0_printf("LCD0      %p\n", (void *)H3_LCD0_BASE);
	uart0_printf("LCD1      %p\n", (void *)H3_LCD1_BASE);
	uart0_printf("SD/MMC0   %p\n", (void *)H3_SD_MMC0_BASE);
	uart0_printf("SD/MMC1   %p\n", (void *)H3_SD_MMC1_BASE);
	uart0_printf("SD/MMC2   %p\n", (void *)H3_SD_MMC2_BASE);
	uart0_printf("SID       %p\n", (void *)H3_SID_BASE);
	uart0_printf("CCU       %p\n", (void *)H3_CCU_BASE);
	uart0_printf("PIO       %p\n", (void *)H3_PIO_BASE);
	uart0_printf("TIMER     %p\n", (void *)H3_TIMER_BASE);
	uart0_printf("AC        %p\n", (void *)H3_AC_BASE);
	uart0_printf("THS       %p\n", (void *)H3_THS_BASE);
	uart0_printf("UART0     %p\n", (void *)H3_UART0_BASE);
	uart0_printf("UART1     %p\n", (void *)H3_UART1_BASE);
	uart0_printf("UART2     %p\n", (void *)H3_UART2_BASE);
	uart0_printf("UART3     %p\n", (void *)H3_UART3_BASE);
	uart0_printf("TWI0      %p\n", (void *)H3_TWI0_BASE);
	uart0_printf("TWI1      %p\n", (void *)H3_TWI1_BASE);
	uart0_printf("TWI2      %p\n", (void *)H3_TWI2_BASE);
	uart0_printf("EMAC      %p\n", (void *)H3_EMAC_BASE);
	uart0_printf("GPU       %p\n", (void *)H3_GPU_BASE);
	uart0_printf("SPI0      %p\n", (void *)H3_SPI0_BASE);
	uart0_printf("SPI1      %p\n", (void *)H3_SPI1_BASE);
	uart0_printf("HDMI      %p\n", (void *)H3_HDMI_BASE);
	uart0_printf("RTC       %p\n", (void *)H3_RTC_BASE);
	//
	uart0_printf("GIC       %p\n", (void *)H3_GIC_BASE);
	uart0_printf("PRCM      %p\n", (void *)H3_PRCM_BASE);
	uart0_printf("CPUCFG    %p\n", (void *)H3_CPUCFG_BASE);
	uart0_printf("PIO_PORTL %p\n", (void *)H3_PIO_PORTL_BASE);
}
