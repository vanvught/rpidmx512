/**
 * @file hardware.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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

#if defined (DEBUG_HAL)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include "hardware.h"

#include "h3.h"
#include "h3_watchdog.h"
#include "h3_gpio.h"
#include "h3_board.h"
#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/gic.h"
#include "arm/synchronize.h"

#include "debug.h"

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# if __GNUC__ > 8
#  pragma GCC target ("general-regs-only")
# endif
#endif

static void EXTIA_IRQHandler() {
	DEBUG_PUTS("EXTIA_IRQHandler");

	H3_PIO_PA_INT->STA = ~0;
}

static void EXTIG_IRQHandler() {
	DEBUG_PUTS("EXTIG_IRQHandler");

	H3_PIO_PG_INT->STA = ~0;
}

static void __attribute__((interrupt("IRQ"))) IRQ_Handler() {
	__DMB();

	const auto nIRQ = GICInterface->AIAR;
	IRQHandler_t const handler = IRQ_GetHandler(nIRQ);

	if (handler != nullptr) {
		handler();
	}

	GICInterface->AEOIR = nIRQ;
	const auto nIndex = nIRQ / 32;
	const auto nMask = 1U << (nIRQ % 32);
	GICDistributor->ICPENDR[nIndex] = nMask;

	__DMB();
}

#pragma GCC pop_options

void hal_init();

Hardware::Hardware() {
	assert(s_pThis == nullptr);
	s_pThis = this;

	hal_init();

	IRQ_SetHandler(H3_PA_EINT_IRQn, EXTIA_IRQHandler);
//	gic_irq_config(H3_PA_EINT_IRQn, GIC_CORE0);
//
	IRQ_SetHandler(H3_PG_EINT_IRQn, EXTIG_IRQHandler);
//	gic_irq_config(H3_PG_EINT_IRQn, GIC_CORE0);

	arm_install_handler((unsigned) IRQ_Handler, ARM_VECTOR(ARM_VECTOR_IRQ));
}
