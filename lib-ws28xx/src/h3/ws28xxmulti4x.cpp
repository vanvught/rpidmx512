/**
 * @file ws28xxmulti4x.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "ws28xxmulti.h"

#include "h3_gpio.h"

#include "debug.h"

#define PULSE 	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 6)	// Pin 7
#define ENABLE	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 18)	// Pin 18

#define OUT0	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 0)	// Pin 13
#define OUT1	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 1)	// Pin 11
#define OUT2	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 2)	// Pin 22
#define OUT3	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 3)	// Pin 15

#define DATA_MASK	((1U << PULSE) | (1U << ENABLE) | (1U << OUT3) | (1U << OUT2) | (1U << OUT1) | (1U << OUT0))

void WS28xxMulti::SetupGPIO() {
	h3_gpio_fsel(OUT0, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT0);
	h3_gpio_fsel(OUT1, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT1);
	h3_gpio_fsel(OUT2, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT2);
	h3_gpio_fsel(OUT3, GPIO_FSEL_OUTPUT);
	h3_gpio_clr(OUT3);

	h3_gpio_fsel(PULSE, GPIO_FSEL_OUTPUT);
	h3_gpio_set(PULSE);

	h3_gpio_fsel(ENABLE, GPIO_FSEL_OUTPUT);
	h3_gpio_set(ENABLE);
}

void WS28xxMulti::SetupBuffers4x() {
	DEBUG_ENTRY

	m_pBuffer4x = new uint32_t[m_nBufSize];
	assert(m_pBuffer4x != nullptr);

	m_pBlackoutBuffer4x = new uint32_t[m_nBufSize];
	assert(m_pBlackoutBuffer4x != nullptr);

	for (uint32_t i = 0; i < m_nBufSize; i++) {
		uint32_t d = (i & 0x1) ? (1 << PULSE) : 0;
		m_pBuffer4x[i] = d;
		m_pBlackoutBuffer4x[i] = d;
	}

	DEBUG_EXIT
}

void WS28xxMulti::Generate800kHz(const uint32_t *pBuffer) {
	uint32_t i = 0;
	constexpr uint32_t d = (125 * 24) / 100;
	uint32_t dat;

	do {
		uint64_t cval;
		asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));

		dat = H3_PIO_PORTA->DAT; // @suppress("C-Style cast instead of C++ cast")
		dat &= (~(DATA_MASK));
		dat |= pBuffer[i];
		H3_PIO_PORTA->DAT = dat; // @suppress("C-Style cast instead of C++ cast")

		uint32_t t1 = (cval & 0xFFFFFFFF);
		const uint32_t t2 = t1 + d;
		i++;

		__builtin_prefetch(&pBuffer[i]);

		do {
			asm volatile("mrrc p15, 1, %Q0, %R0, c14" : "=r" (cval));
			t1 = (cval & 0xFFFFFFFF);
		} while (t1 < t2);

	} while (i < m_nBufSize);

	dat |= (1 << ENABLE);
	H3_PIO_PORTA->DAT = dat; // @suppress("C-Style cast instead of C++ cast")
}
