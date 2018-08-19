/**
 * @file h3_ccu.c
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
#include "h3_ccu.h"

#define PLL_ENABLE			(1 << 31)	///< The PLL output is (24MHz*N*K)/M
#define PLL_LOCK			(1 << 28)	///< Locked indicates that the PLL has been stable
#define PLL_BYPASS_ENABLE	(1 << 25)	///< If the bypass is enabled, PLL output is 24MHz

#define PLL_FACTOR_N_SHIFT	8
#define PLL_FACTOR_N_MASK 	(0x1F << PLL_FACTOR_N_SHIFT)

#define PLL_FACTOR_K_SHIFT	4
#define PLL_FACTOR_K_MASK 	(0x03 << PLL_FACTOR_K_SHIFT)

#define PLL_FACTOR_M_SHIFT	0
#define PLL_FACTOR_M_MASK 	(0x03 << PLL_FACTOR_M_SHIFT)

uint64_t h3_ccu_get_pll_rate(ccu_pll_t pll) {
	uint32_t source;
	uint32_t value;

	switch (pll) {
		case CCU_PLL_CPUX:
			source = H3_CCU->CPU_AXI_CFG & 0x3;
			if (source == 0) {
				return H3_LOSC;
			} else if (source == 1) {
				return H3_F_24M;
			}
			value = H3_CCU->PLL_CPUX_CTRL;
			break;
		case CCU_PLL_AUDIO:
			value = H3_CCU->PLL_AUDIO_CTRL;
			break;
		case CCU_PLL_VIDEO:
			value = H3_CCU->PLL_VIDEO_CTRL;
			break;
		case CCU_PLL_VE:
			value = H3_CCU->PLL_VE_CTRL;
			break;
		case CCU_PLL_DDR:
			value = H3_CCU->PLL_DDR_CTRL;
			break;
		case CCU_PLL_PERIPH0:
			value = H3_CCU->PLL_PERIPH0_CTRL;
			if ((value & PLL_BYPASS_ENABLE) == PLL_BYPASS_ENABLE) {
				return H3_F_24M;
			}
			break;
		case CCU_PLL_GPU:
			value = H3_CCU->PLL_GPU_CTRL;
			break;
		case CCU_PLL_PERIPH1:
			value = H3_CCU->PLL_PERIPH1_CTRL;
			if ((value & PLL_BYPASS_ENABLE) == PLL_BYPASS_ENABLE) {
				return H3_F_24M;
			}
			break;
		case CCU_PLL_DE:
			value = H3_CCU->PLL_DE_CTRL;
			break;
		default:
#ifndef NDEBUG
			printf("Invalid PLL\n");
#endif
			return 0;
			break;
	}

	const uint32_t n = (value & PLL_FACTOR_N_MASK) >> PLL_FACTOR_N_SHIFT;
	const uint32_t k = (value & PLL_FACTOR_K_MASK) >> PLL_FACTOR_K_SHIFT;
	const uint32_t m = (value & PLL_FACTOR_M_MASK) >> PLL_FACTOR_M_SHIFT;

	uint64_t freq = H3_F_24M;
	freq *= n + 1;
	freq *= k + 1;
	freq /= m + 1;

	return freq;
}

void h3_ccu_pll_dump(void) {
#ifndef NDEBUG
	printf("CCU_PLL_CPUX=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_CPUX));
	printf("CCU_PLL_AUDIO=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_AUDIO));
	printf("CCU_PLL_VIDEO=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_VIDEO));
	printf("CCU_PLL_VE=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_VE));
	printf("CCU_PLL_DDR=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_DDR));
	printf("CCU_PLL_PERIPH0=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_PERIPH0));
	printf("CCU_PLL_GPU=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_GPU));
	printf("CCU_PLL_PERIPH1=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_PERIPH1));
	printf("CCU_PLL_DE=%ld\n", (long int) h3_ccu_get_pll_rate(CCU_PLL_DE));
#endif
}
