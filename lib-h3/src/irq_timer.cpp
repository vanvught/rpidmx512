/**
 * @file irq_timer.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#define CONFIG_SYSTEM_CMSIS_IRQ_HANDLER

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# if __GNUC__ > 8
#  pragma GCC target ("general-regs-only")
# endif
#endif

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/gic.h"

#include "h3.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

#define ARM_TIMER_ENABLE			(1U << 0)
#define ARM_PHYSICAL_TIMER_IRQ		H3_PPI13_IRQn
#define ARM_VIRTUAL_TIMER_IRQ		H3_PPI11_IRQn

/**
 * H3 Timers
 */
static thunk_irq_timer_t h3_timer0_func;
static thunk_irq_timer_t h3_timer1_func;

static void TIMER0_IRQHandler() {
	H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */

	assert(h3_timer0_func != nullptr);

	if (__builtin_expect((h3_timer0_func != nullptr), 1)) {
		h3_timer0_func(H3_HS_TIMER->CURNT_LO);
	}
}

static void TIMER1_IRQHandler() {
	H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */

	assert(h3_timer1_func != nullptr);

	if (__builtin_expect((h3_timer1_func != nullptr), 1)) {
		h3_timer1_func(H3_HS_TIMER->CURNT_LO);
	}
}

/**
 * Generic ARM Timer
 */
static volatile thunk_irq_timer_arm_t arm_physical_timer_func;
static volatile thunk_irq_timer_arm_t arm_virtual_timer_func;

static volatile uint32_t timer_value;

static void ARM_Physical_Timer_IRQHandler() {
	__asm volatile ("mcr p15, 0, %0, c14, c2, 0" : : "r" (H3_F_24M));
	__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (ARM_TIMER_ENABLE));

	assert(arm_physical_timer_func != nullptr);

	if (__builtin_expect((arm_physical_timer_func != nullptr), 1)) {
		arm_physical_timer_func();
	}
}

static void ARM_Virtual_Timer_IRQHandler() {
	__asm volatile ("mcr p15, 0, %0, c14, c3, 0" : : "r" (timer_value));
	__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (ARM_TIMER_ENABLE));

	assert(arm_virtual_timer_func != nullptr);

	if (__builtin_expect((arm_virtual_timer_func != nullptr), 1)) {
		arm_virtual_timer_func();
	}
}

void irq_timer_set(_irq_timers nIrqTimer, thunk_irq_timer_t func) {
	if (nIrqTimer == IRQ_TIMER_0) {
		h3_timer0_func = func;

		if (func != nullptr) {
			H3_TIMER->TMR0_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;	/* Enable Timer 0 Interrupts */
			IRQ_SetHandler(H3_TIMER0_IRQn, TIMER0_IRQHandler);
			gic_irq_config(H3_TIMER0_IRQn, GIC_CORE0);
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR0;	/* Disable Timer 0 Interrupts */
			IRQ_SetHandler(H3_TIMER0_IRQn, nullptr);
		}

		__ISB();
		return;
	}

	if (nIrqTimer == IRQ_TIMER_1) {
		h3_timer1_func = func;

		if (func != nullptr) {
			H3_TIMER->TMR1_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;	/* Enable Timer 1 Interrupts */
			IRQ_SetHandler(H3_TIMER1_IRQn, TIMER1_IRQHandler);
			gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR1;	/* Disable Timer 1 Interrupts */
			IRQ_SetHandler(H3_TIMER1_IRQn, nullptr);
		}

		__ISB();
		return;
	}

	assert(0);
}

void irq_timer_arm_physical_set(thunk_irq_timer_arm_t func) {
	arm_physical_timer_func = func;

	if (func != nullptr) {
		__asm volatile ("mcr p15, 0, %0, c14, c2, 0" : : "r" (H3_F_24M));
		__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (ARM_TIMER_ENABLE));
		IRQ_SetHandler(ARM_PHYSICAL_TIMER_IRQ, ARM_Physical_Timer_IRQHandler);
		gic_irq_config(ARM_PHYSICAL_TIMER_IRQ, GIC_CORE0);
	} else {
		__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (0));
		IRQ_SetHandler(ARM_PHYSICAL_TIMER_IRQ, nullptr);
	}

	__ISB();
}

void irq_timer_arm_virtual_set(thunk_irq_timer_arm_t func, uint32_t value) {
	arm_virtual_timer_func = func;
	timer_value = value;

	if (func != nullptr) {
		__asm volatile ("mcr p15, 0, %0, c14, c3, 0" : : "r" (timer_value));
		__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (ARM_TIMER_ENABLE));
		IRQ_SetHandler(ARM_VIRTUAL_TIMER_IRQ, ARM_Virtual_Timer_IRQHandler);
		gic_irq_config(ARM_VIRTUAL_TIMER_IRQ, GIC_CORE0);
		gic_irq_config(static_cast<H3_IRQn_TypeDef>(26), GIC_CORE0);
		gic_irq_config(static_cast<H3_IRQn_TypeDef>(28), GIC_CORE0);
	} else {
		__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (0));
		IRQ_SetHandler(ARM_VIRTUAL_TIMER_IRQ, nullptr);
	}

	__ISB();
}

#if !defined(CONFIG_SYSTEM_CMSIS_IRQ_HANDLER)
static void __attribute__((interrupt("IRQ"))) irq_timer_handler(void) {
	__DMB();

	const auto irq = H3_GIC_CPUIF->AIA;

	if ((h3_timer0_func != NULL) && (irq == H3_TIMER0_IRQn)) {
		TIMER0_IRQHandler();
	} else if ((h3_timer1_func != NULL) && (irq == H3_TIMER1_IRQn)) {
		TIMER1_IRQHandler();
	} else if (irq == ARM_PHYSICAL_TIMER_IRQ) {
		ARM_Physical_Timer_IRQHandler();
	} else if (irq == ARM_VIRTUAL_TIMER_IRQ) {
		ARM_Virtual_Timer_IRQHandler();
	}

	__DMB();
}

void __attribute__((cold)) irq_handler_init(void) {
	arm_install_handler(reinterpret_cast<unsigned>(irq_timer_handler), ARM_VECTOR(ARM_VECTOR_IRQ));

	__enable_irq();
}
#else
void __attribute__((cold)) irq_handler_init() {
	__enable_irq();
	__DMB();
}
#endif
