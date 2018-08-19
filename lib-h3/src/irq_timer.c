/**
 * @file irq_timer.c
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
#include <stddef.h>

#include "irq_timer.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "h3.h"
#include "h3_timer.h"
#include "h3_hs_timer.h"

static /*@null@*/ thunk_irq_timer_t timer0_func = NULL;
static /*@null@*/ thunk_irq_timer_t timer1_func = NULL;

static void __attribute__((interrupt("IRQ"))) irq_timer_handler(void) {
	dmb();

	const uint32_t irq_timer_micros = h3_hs_timer_lo_us();
	const uint32_t irq = H3_GIC_CPUIF->AIA;

	if ((timer0_func != NULL) && (irq == H3_TIMER0_IRQn)) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR0;	/* Clear Timer 0 Pending bit */
		timer0_func(irq_timer_micros);
		H3_GIC_CPUIF->AEOI = H3_TIMER0_IRQn;
		H3_GIC_DIST->ICPEND[H3_TIMER0_IRQn / 32] = 1 << (H3_TIMER0_IRQn % 32);
	} else if ((timer1_func != NULL) && (irq == H3_TIMER1_IRQn)) {
		H3_TIMER->IRQ_STA = TIMER_IRQ_PEND_TMR1;	/* Clear Timer 1 Pending bit */
		timer1_func(irq_timer_micros);
		H3_GIC_CPUIF->AEOI = H3_TIMER1_IRQn;
		H3_GIC_DIST->ICPEND[H3_TIMER1_IRQn / 32] = 1 << (H3_TIMER1_IRQn % 32);
	}

	dmb();
}

void irq_timer_set(_irq_timers timer, thunk_irq_timer_t func) {

	if (timer == IRQ_TIMER_0) {
		timer0_func = func;
		if (func != NULL) {
			H3_TIMER->TMR0_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR0;	/* Enable Timer 0 Interrupts */
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR0;	/* Disable Timer 0 Interrupts */
		}
	} else if (timer == IRQ_TIMER_1) {
		timer1_func = func;
		if (func != NULL) {
			H3_TIMER->TMR1_CTRL = 0x14; 			/* Select continuous mode, 24MHz clock source, 2 pre-scale */
			H3_TIMER->IRQ_EN |= TIMER_IRQ_EN_TMR1;	/* Enable Timer 1 Interrupts */
		} else {
			H3_TIMER->IRQ_EN &= ~TIMER_IRQ_EN_TMR1;	/* Disable Timer 1 Interrupts */
		}
	}

	isb();
}

void irq_timer_init(void) {
	arm_install_handler((unsigned) irq_timer_handler, ARM_VECTOR(ARM_VECTOR_IRQ));

	gic_irq_config(H3_TIMER0_IRQn, GIC_CORE0);
	gic_irq_config(H3_TIMER1_IRQn, GIC_CORE0);

	__enable_irq();
}
