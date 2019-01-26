/**
 * @file ltcleds.cpp
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

#include "ltcleds.h"

#include "ltc.h"

#include "h3_gpio.h"

#define ENABLE		2  // GPIO_EXT_22
#define SELECT_A	18 // GPIO_EXT_18
#define SELECT_B	19 // GPIO_EXT_16

LtcLeds *LtcLeds::s_pThis = 0;

LtcLeds::LtcLeds(void) {
	s_pThis = this;

	h3_gpio_fsel(ENABLE, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(SELECT_A, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(SELECT_B, GPIO_FSEL_OUTPUT);

	h3_gpio_set(ENABLE);
}

LtcLeds::~LtcLeds(void) {
	h3_gpio_set(ENABLE);
}

void LtcLeds::Show(TTimecodeTypes tTimecodeType) {
	uint32_t gpioa = H3_PIO_PORTA->DAT;
	gpioa &= ~((1 << ENABLE) | (1 << SELECT_B) | (1 << SELECT_A));

	switch (tTimecodeType) {
	case TC_TYPE_FILM:
		/* Nothing to here */
		break;
	case TC_TYPE_EBU:
		gpioa |= (1 << SELECT_A);
		break;
	case TC_TYPE_DF:
		gpioa |= (1 << SELECT_B);
		break;
	case TC_TYPE_SMPTE:
		gpioa |= ((1 << SELECT_B) | (1 << SELECT_A));
		break;
	default:
		gpioa |= (1 << ENABLE);
		break;
	}

	H3_PIO_PORTA->DAT = gpioa;
}
