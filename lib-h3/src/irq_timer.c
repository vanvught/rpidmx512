/**
 * @file irq_timer.c
 *
 */
/* Copyright (C) 2018-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if __GNUC__ > 8
# pragma GCC target ("general-regs-only")
#endif

#include <stdint.h>
#include <stddef.h>

#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

/**
 * Generic ARM Timer
 */
static volatile thunk_irq_timer_arm_t arm_physical_timer_func = NULL;
static volatile thunk_irq_timer_arm_t arm_virtual_timer_func = NULL;

static volatile uint32_t timer_value;

#define ARM_TIMER_ENABLE			(1U<<0)
#define ARM_PHYSICAL_TIMER_IRQ		H3_PPI13_IRQn
#define ARM_VIRTUAL_TIMER_IRQ		H3_PPI11_IRQn

/**
 * H3 Timers
 */
static thunk_irq_timer_t h3_timer0_func = NULL;
static thunk_irq_timer_t h3_timer1_func = NULL;

static void arm_physical_timer_handler(void) {
	__asm volatile ("mcr p15, 0, %0, c14, c2, 0" : : "r" (H3_F_24M));
	__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (ARM_TIMER_ENABLE));

	if (arm_physical_timer_func != NULL) {
		arm_physical_timer_func();
	}

	H3_GIC_CPUIF->AEOI = ARM_PHYSICAL_TIMER_IRQ;
	H3_GIC_DIST->ICPEND[ARM_PHYSICAL_TIMER_IRQ / 32] = 1 << (ARM_PHYSICAL_TIMER_IRQ % 32);
}

static void arm_virtual_timer_handler(void) {
	__asm volatile ("mcr p15, 0, %0, c14, c3, 0" : : "r" (timer_value));
	__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (ARM_TIMER_ENABLE));

	if (arm_virtual_timer_func != NULL) {
		arm_virtual_timer_func();
	}

	H3_GIC_CPUIF->AEOI = ARM_VIRTUAL_TIMER_IRQ;
	H3_GIC_DIST->ICPEND[ARM_VIRTUAL_TIMER_IRQ / 32] = 1 << (ARM_VIRTUAL_TIMER_IRQ % 32);
}

static void __attribute__((interrupt("IRQ"))) irq_timer_handler(void) {
	dmb();

	const uint32_t irq_timer_micros = h3_hs_timer_lo_us();
	const uint32_t irq = H3_GIC_CPUIF->AIA;

	if ((h3_timer0_func != NULL) && (irq == H3_TIMER0_IRQn)) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */
		h3_timer0_func(irq_timer_micros);
		H3_GIC_CPUIF->AEOI = H3_TIMER0_IRQn;
		H3_GIC_DIST->ICPEND[H3_TIMER0_IRQn / 32] = 1 << (H3_TIMER0_IRQn % 32);
	} else if ((h3_timer1_func != NULL) && (irq == H3_TIMER1_IRQn)) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */
		h3_timer1_func(irq_timer_micros);
		H3_GIC_CPUIF->AEOI = H3_TIMER1_IRQn;
		H3_GIC_DIST->ICPEND[H3_TIMER1_IRQn / 32] = 1 << (H3_TIMER1_IRQn % 32);
	} else if (irq == ARM_PHYSICAL_TIMER_IRQ) {
		arm_physical_timer_handler();
	} else if (irq == ARM_VIRTUAL_TIMER_IRQ) {
		arm_virtual_timer_handler();
	}

	dmb();
}

void irq_timer_arm_physical_set(thunk_irq_timer_arm_t func) {
	arm_physical_timer_func = func;

	if (func != NULL) {
		__asm volatile ("mcr p15, 0, %0, c14, c2, 0" : : "r" (H3_F_24M));
		__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (ARM_TIMER_ENABLE));
		gic_irq_config(ARM_PHYSICAL_TIMER_IRQ, GIC_CORE0);
	} else {
		__asm volatile ("mcr p15, 0, %0, c14, c2, 1" : : "r" (0));
	}

	isb();
}

void irq_timer_arm_virtual_set(thunk_irq_timer_arm_t func, uint32_t value) {
	arm_virtual_timer_func = func;
	timer_value = value;

	if (func != NULL) {
		__asm volatile ("mcr p15, 0, %0, c14, c3, 0" : : "r" (timer_value));
		__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (ARM_TIMER_ENABLE));
		gic_irq_config(ARM_VIRTUAL_TIMER_IRQ, GIC_CORE0);

		gic_irq_config(26, GIC_CORE0);
		gic_irq_config(28, GIC_CORE0);
	} else {
		__asm volatile ("mcr p15, 0, %0, c14, c3, 1" : : "r" (0));
	}

	isb();
}

void irq_timer_set(_irq_timers timer, thunk_irq_timer_t func) {

	if (timer == IRQ_TIMER_0) {
		h3_timer0_func = func;
		if (func != NULL) {
			H3_TIMER->TMR0_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;	/* Enable Timer 0 Interrupts */
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR0;	/* Disable Timer 0 Interrupts */
		}
	} else if (timer == IRQ_TIMER_1) {
		h3_timer1_func = func;
		if (func != NULL) {
			H3_TIMER->TMR1_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;	/* Enable Timer 1 Interrupts */
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR1;	/* Disable Timer 1 Interrupts */
		}
	}

	isb();
}

void __attribute__((cold)) irq_timer_init(void) {
	arm_install_handler((unsigned) irq_timer_handler, ARM_VECTOR(ARM_VECTOR_IRQ));

	gic_irq_config(H3_TIMER0_IRQn, GIC_CORE0);
	gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

	__enable_irq();
}
