/**
 * @file h3_dump_memory_mapping.cpp
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

namespace uart0
{
int Printf(const char* fmt, ...);
}

void __attribute__((cold)) h3_dump_memory_mapping()
{
    uart0::Printf("4.1 Memory Mapping\n");
    uart0::Printf("DE        %p\n", (void*)H3_DE_BASE);
    uart0::Printf("DMA       %p\n", (void*)H3_DMA_BASE);
    uart0::Printf("LCD0      %p\n", (void*)H3_LCD0_BASE);
    uart0::Printf("LCD1      %p\n", (void*)H3_LCD1_BASE);
    uart0::Printf("SD/MMC0   %p\n", (void*)H3_SD_MMC0_BASE);
    uart0::Printf("SD/MMC1   %p\n", (void*)H3_SD_MMC1_BASE);
    uart0::Printf("SD/MMC2   %p\n", (void*)H3_SD_MMC2_BASE);
    uart0::Printf("SID       %p\n", (void*)H3_SID_BASE);
    uart0::Printf("CCU       %p\n", (void*)H3_CCU_BASE);
    uart0::Printf("PIO       %p\n", (void*)H3_PIO_BASE);
    uart0::Printf("TIMER     %p\n", (void*)H3_TIMER_BASE);
    uart0::Printf("AC        %p\n", (void*)H3_AC_BASE);
    uart0::Printf("THS       %p\n", (void*)H3_THS_BASE);
    uart0::Printf("UART0     %p\n", (void*)H3_UART0_BASE);
    uart0::Printf("UART1     %p\n", (void*)H3_UART1_BASE);
    uart0::Printf("UART2     %p\n", (void*)H3_UART2_BASE);
    uart0::Printf("UART3     %p\n", (void*)H3_UART3_BASE);
    uart0::Printf("TWI0      %p\n", (void*)H3_TWI0_BASE);
    uart0::Printf("TWI1      %p\n", (void*)H3_TWI1_BASE);
    uart0::Printf("TWI2      %p\n", (void*)H3_TWI2_BASE);
    uart0::Printf("EMAC      %p\n", (void*)H3_EMAC_BASE);
    uart0::Printf("GPU       %p\n", (void*)H3_GPU_BASE);
    uart0::Printf("SPI0      %p\n", (void*)H3_SPI0_BASE);
    uart0::Printf("SPI1      %p\n", (void*)H3_SPI1_BASE);
    uart0::Printf("HDMI      %p\n", (void*)H3_HDMI_BASE);
    uart0::Printf("RTC       %p\n", (void*)H3_RTC_BASE);
    //
    uart0::Printf("GIC       %p\n", (void*)H3_GIC_BASE);
    uart0::Printf("PRCM      %p\n", (void*)H3_PRCM_BASE);
    uart0::Printf("CPUCFG    %p\n", (void*)H3_CPUCFG_BASE);
    uart0::Printf("PIO_PORTL %p\n", (void*)H3_PIO_PORTL_BASE);
}
