/**
 * @file h3_ccu.c
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "h3_ccu.h"
#include "h3_uart0_debug.h"

void __attribute__((cold)) h3_ccu_pll_dump(void) {
	uart0_puts("PLL (Hz)\n");
	uart0_printf("CPUX=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_CPUX));
	uart0_printf("AUDIO=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_AUDIO));
	uart0_printf("VIDEO=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_VIDEO));
	uart0_printf("VE=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_VE));
	uart0_printf("DDR=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_DDR));
	uart0_printf("PERIPH0=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_PERIPH0));
	uart0_printf("GPU=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_GPU));
	uart0_printf("PERIPH1=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_PERIPH1));
	uart0_printf("DE=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_DE));
}
