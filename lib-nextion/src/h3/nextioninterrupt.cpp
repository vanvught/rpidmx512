/**
 * @file nextioninterrupt.cpp
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "nextion.h"

#include "board/h3_opi_zero.h"
#include "h3_gpio.h"

#include "debug.h"

#include "nextion.h"

#define GPIO_INT	GPIO_EXT_26 // PA10
#define INT_MASK	(1 << GPIO_INT)

void Nextion::InitInterrupt(void) {
	DEBUG_ENTRY

	h3_gpio_fsel(GPIO_INT, GPIO_FSEL_EINT);

	uint32_t value = H3_PIO_PORTA->PUL0;
	value &= ~(GPIO_PULL_MASK << 20);
	value |= (GPIO_PULL_UP << 20);
	H3_PIO_PORTA->PUL0 = value;

	value = H3_PIO_PA_INT->CFG0;
	value &= ~(GPIO_INT_CFG_MASK << 8);
	value |= (GPIO_INT_CFG_NEG_EDGE << 8);
	H3_PIO_PA_INT->CFG0 = value;

	H3_PIO_PA_INT->CTL |= INT_MASK;
	H3_PIO_PA_INT->STA = INT_MASK;

	DEBUG_EXIT
}

bool Nextion::IsInterrupt(void) {
	const uint32_t nValue = H3_PIO_PA_INT->STA & INT_MASK;

	if (__builtin_expect((nValue != 0), 0)) {
		H3_PIO_PA_INT->STA = INT_MASK;
		return true;
	}

	return false;
}
