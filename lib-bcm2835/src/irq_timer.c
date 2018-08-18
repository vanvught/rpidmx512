/**
 * @file irq_timer.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/arm.h"
#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_st.h"

#include "irq_timer.h"

static /*@null@*/ thunk_irq_timer_t timer1_func = NULL;
static /*@null@*/ thunk_irq_timer_t timer3_func = NULL;

static void __attribute__((interrupt("IRQ"))) irq_timer_handler(void) {
	dmb();

	const uint32_t irq_timer_micros = BCM2835_ST->CLO;

	if ((timer1_func != NULL) && (BCM2835_ST->CS & BCM2835_ST_CS_M1) ) {
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		timer1_func(irq_timer_micros);
	}
	
	if ((timer3_func != NULL) && (BCM2835_ST->CS & BCM2835_ST_CS_M3) ) {
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		timer3_func(irq_timer_micros);
	}

	dmb();
}

void irq_timer_set(const _irq_timers timer, thunk_irq_timer_t func) {
	dmb();

	if (timer == IRQ_TIMER_1) {
		timer1_func = func;
		BCM2835_ST->CS = BCM2835_ST_CS_M1;
		if (func != NULL) {
			BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn;
		} else {
			BCM2835_IRQ->IRQ_DISABLE1 = BCM2835_TIMER1_IRQn;
		}
	} else if (timer == IRQ_TIMER_3) {
		timer3_func = func;
		BCM2835_ST->CS = BCM2835_ST_CS_M3;
		if (func != NULL) {
			BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER3_IRQn;
		} else {
			BCM2835_IRQ->IRQ_DISABLE1 = BCM2835_TIMER3_IRQn;
		}
	}

	dmb();
}

void irq_timer_init(void) {
	arm_install_handler((unsigned)irq_timer_handler, ARM_VECTOR(ARM_VECTOR_IRQ));

	BCM2835_ST->CS = BCM2835_ST_CS_M1 + BCM2835_ST_CS_M3;

	dmb();
	__enable_irq();
}
