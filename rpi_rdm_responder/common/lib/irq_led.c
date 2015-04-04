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

#include <stdint.h>

#include "bcm2835.h"
#include "hardware.h"

static uint32_t ticks_per_second = 1E6 / 2;	///< Blinking at 1Hz
static uint32_t irq_counter;				///<

/**
 *
 * @param ticks
 */
void ticks_per_second_set(uint32_t ticks)
{
	ticks_per_second = ticks;
}

/**
 *
 * @return
 */
uint32_t ticks_per_second_get(void)
{
	return ticks_per_second;
}

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
	hardware_led_set(irq_counter++ & 0x01);
	dmb();
}
