/**
 * @file irq_led.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#include "bcm2835.h"

uint32_t ticks_per_second = 1E6 / 2;	///< Blinking at 1Hz
static uint32_t irq_counter;			///<

#define PIN		16

/**
 * @ingroup led
 *
 */
void irq_init(void) {
    irq_counter = 0;
    BCM2835_ST->C1 = BCM2835_ST->CLO + ticks_per_second;
    BCM2835_ST->CS = BCM2835_ST_CS_M1;
	BCM2835_IRQ->IRQ_ENABLE1 = BCM2835_TIMER1_IRQn;
}

/**
 * @ingroup led
 *
 */
void __attribute__((interrupt("IRQ"))) c_irq_handler(void) {
	dmb();
	BCM2835_ST ->CS = BCM2835_ST_CS_M1;
	BCM2835_ST ->C1 = BCM2835_ST ->CLO + ticks_per_second;
	if (irq_counter++ & 0x01)
		BCM2835_GPIO ->GPSET0 = 1 << PIN;
	else
		BCM2835_GPIO ->GPCLR0 = 1 << PIN;
	dmb();
}
