/**
 * @file h3_cpu.c
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
#include <assert.h>

#include "h3_cpu.h"
#include "h3.h"
#include "h3_ccu.h"

#include "arm/synchronize.h"

	#define PLL_FACTOR_M_MASK	0x03
	#define PLL_FACTOR_M_SHIFT	0
	#define PLL_CTRL_M(n)		((((n) - 1) & PLL_FACTOR_M_MASK) << PLL_FACTOR_M_SHIFT)

	#define PLL_FACTOR_K_MASK	0x03
	#define PLL_FACTOR_K_SHIFT	4
	#define PLL_CTRL_K(n)		((((n) - 1) & PLL_FACTOR_K_MASK) << PLL_FACTOR_K_SHIFT)

	#define PLL_FACTOR_N_MASK	0x1F
	#define PLL_FACTOR_N_SHIFT	8
	#define PLL_CTRL_N(n)		((((n) - 1) & PLL_FACTOR_N_MASK) << PLL_FACTOR_N_SHIFT)

	#define PLL_FACTOR_P_MASK	0x03
	#define PLL_FACTOR_P_SHIFT	16
	#define PLL_CTRL_P(n)		(((n) & PLL_FACTOR_P_MASK) << PLL_FACTOR_P_SHIFT)

#define PLL_LOCK					(1 << 28)	// Read only, 1 indicates that the PLL has been stable
#define PLL_ENABLE					(1 << 31)

#define CPU_CLK_SRC_OSC24M			(1 << 16)
#define CPU_CLK_SRC_PLL_CPUX		(2 << 16)
	#define CPU_CLK_SRC_MASK	0x03
	#define CPU_CLK_SRC_SHIFT	16

void h3_cpu_off(h3_cpu_t cpuid) {
	assert(H3_CPU0 != cpuid);
	assert(cpuid < H3_CPU_COUNT);

	const uint32_t cpu = cpuid & (H3_CPU_COUNT - 1); // Count is always power of 2

	// step1: set up power-off signal
	uint32_t value = H3_PRCM->CPU_PWROFF;
	value |= (1U << cpu);
	H3_PRCM->CPU_PWROFF = value;

	udelay(20);

	// step2: active the power output clamp
	switch (cpu) {
	case H3_CPU0:
		// Do nothing
		break;
	case H3_CPU1:
		H3_PRCM->CPU1_PWR_CLAMP = 0xFF;
		break;
	case H3_CPU2:
		H3_PRCM->CPU2_PWR_CLAMP = 0xFF;
		break;
	case H3_CPU3:
		H3_PRCM->CPU3_PWR_CLAMP = 0xFF;
		break;
	default:
		break;
	}

	udelay(30);

	switch (cpu) {
	case H3_CPU0:
		// Do nothing
		break;
	case H3_CPU1:
		while (H3_PRCM->CPU1_PWR_CLAMP != 0xFF)
			;
		break;
	case H3_CPU2:
		while (H3_PRCM->CPU2_PWR_CLAMP != 0xFF)
			;
		break;
	case H3_CPU3:
		while (H3_PRCM->CPU3_PWR_CLAMP != 0xFF)
			;
		break;
	default:
		break;
	}
}

void h3_cpu_on(h3_cpu_t cpuid) {
	assert(H3_CPU0 != cpuid);
	assert(cpuid < H3_CPU_COUNT);

	const uint32_t cpu = cpuid & H3_CPUS_MASK;

	/* Assert the CPU core in reset */
	H3_CPUCFG->CPU[cpu].RST = 0;

	/* Assert the L1 cache in reset */
	uint32_t reg = H3_CPUCFG->GEN_CTRL;
	reg &= ~(1U << cpu);
	H3_CPUCFG->GEN_CTRL = reg;

	udelay(10);

	volatile uint32_t *p = 0;

	switch (cpu) {
	case H3_CPU0:
		// Do nothing
		break;
	case H3_CPU1:
		p = (uint32_t *)H3_PRCM->CPU1_PWR_CLAMP;
		break;
	case H3_CPU2:
		p = (uint32_t *)H3_PRCM->CPU2_PWR_CLAMP;
		break;
	case H3_CPU3:
		p = (uint32_t *)H3_PRCM->CPU3_PWR_CLAMP;
		break;
	default:
		break;
	}

	assert(p != 0);

	*p = 0xFE;
	udelay(20);
	*p = 0xF8;
	udelay(10);
	*p = 0xE0;
	udelay(10);
	*p = 0x80;
	udelay(10);
	*p = 0x00;
	udelay(10);

	while (0x00 != *p);

	/* Clear CPU power-off gating */
	uint32_t value = H3_PRCM->CPU_PWROFF;
	value &= ~(1U << cpu);
	H3_PRCM->CPU_PWROFF = value;

	/* Deassert the CPU core reset */
	H3_CPUCFG->CPU[cpu].RST = 3;
}

void h3_cpu_set_clock(uint64_t clock) {
	uint32_t value;

	// Switch to 24MHz clock while changing CCU_PLL_CPUX
	value = H3_CCU->CPU_AXI_CFG;
	value &= (uint32_t) ~(CPU_CLK_SRC_MASK << CPU_CLK_SRC_SHIFT);
	value |= CPU_CLK_SRC_OSC24M;
	H3_CCU->CPU_AXI_CFG = value;

	if ((clock < CCU_PLL_CPUX_MIN_CLOCK_HZ) || (clock > CCU_PLL_CPUX_MAX_CLOCK_HZ)) {
		// Set default
		value = H3_CCU->PLL_CPUX_CTRL;
		value &= (uint32_t) ~(PLL_FACTOR_N_MASK << PLL_FACTOR_N_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_K_MASK << PLL_FACTOR_K_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_M_MASK << PLL_FACTOR_M_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_P_MASK << PLL_FACTOR_P_SHIFT);
		value |= (0x10 << PLL_FACTOR_N_SHIFT);
		H3_CCU->PLL_CPUX_CTRL = value;
	} else {
		const uint32_t p = 0;
		uint32_t k = 1;
		uint32_t m = 1;

		if (clock > 1152000000) {
			k = 2;
		} else if (clock > 768000000) {
			k = 4;
			m = 2;
		}

		value = H3_CCU->PLL_CPUX_CTRL;
		value &= (uint32_t) ~(PLL_FACTOR_N_MASK << PLL_FACTOR_N_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_K_MASK << PLL_FACTOR_K_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_M_MASK << PLL_FACTOR_M_SHIFT);
		value &= (uint32_t) ~(PLL_FACTOR_P_MASK << PLL_FACTOR_P_SHIFT);
		value |= PLL_CTRL_N(clock / (24000000 * k / m));
		value |= PLL_CTRL_K(k);
		value |= PLL_CTRL_M(m);
		value |= PLL_CTRL_P(p);
		H3_CCU->PLL_CPUX_CTRL = value;
	}

	do {
		value = H3_CCU->PLL_CPUX_CTRL;
	} while (!(value & PLL_LOCK));

	value = H3_CCU->CPU_AXI_CFG;
	value &= (uint32_t) ~(CPU_CLK_SRC_MASK << CPU_CLK_SRC_SHIFT);
	value |= CPU_CLK_SRC_PLL_CPUX;
	H3_CCU->CPU_AXI_CFG = value;
}
