#if defined (RASPPI)
/**
 * @file buttonsadafruit.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>

#include "hal_gpio.h"

#include "input.h"

#include "buttonsadafruit.h"

#define PIN_L GPIO_EXT_13
#define PIN_R GPIO_EXT_16
#define PIN_C GPIO_EXT_7
#define PIN_U GPIO_EXT_11
#define PIN_D GPIO_EXT_15
#define PIN_A GPIO_EXT_29
#define PIN_B GPIO_EXT_31

#define MASK_GPLEV0  static_cast<uint32_t>((1 << PIN_L) | (1 << PIN_R)  | (1 << PIN_C) | (1 << PIN_U) | (1 << PIN_D) | (1 << PIN_A) | (1 << PIN_B))

static void init_gpio_pin(const uint8_t nPin) {
	FUNC_PREFIX(gpio_fsel(nPin, GPIO_FSEL_INPUT));
	FUNC_PREFIX(gpio_set_pud(nPin, GPIO_PULL_UP));
}

ButtonsAdafruit::ButtonsAdafruit() {
	init_gpio_pin(PIN_L);
	init_gpio_pin(PIN_R);
	init_gpio_pin(PIN_C);
	init_gpio_pin(PIN_U);
	init_gpio_pin(PIN_D);
	init_gpio_pin(PIN_A);
	init_gpio_pin(PIN_B);
}

bool ButtonsAdafruit::IsAvailable() {
# if defined(__linux__)
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEV0/4;
	const auto reg = bcm2835_peri_read(paddr);
# else
	__sync_synchronize();
	const auto reg = BCM2835_GPIO->GPLEV0;
# endif

	m_rMaskedBits = ~reg & MASK_GPLEV0;

	if (m_rMaskedBits != 0) {
		return true;
	}

	m_PrevChar = input::KEY_NOT_DEFINED;

	return false;
}

int ButtonsAdafruit::GetChar() {
	int ch = input::KEY_NOT_DEFINED;

	if ((m_rMaskedBits & (1 << PIN_L)) == (1 << PIN_L)) {
		ch = input::KEY_LEFT;
	} else if ((m_rMaskedBits & (1 << PIN_R)) == (1 << PIN_R)) {
		ch = input::KEY_RIGHT;
	} else if ((m_rMaskedBits & (1 << PIN_C)) == (1 << PIN_C)) {
		ch = input::KEY_ENTER;
	} else if ((m_rMaskedBits & (1 << PIN_U)) == (1 << PIN_U)) {
		ch = input::KEY_UP;
	} else if ((m_rMaskedBits & (1 << PIN_D)) == (1 << PIN_D)) {
		ch = input::KEY_DOWN;
	} else if ((m_rMaskedBits & (1 << PIN_A)) == (1 << PIN_A)) {
		ch = input::KEY_ENTER;
	} else if ((m_rMaskedBits & (1 << PIN_B)) == (1 << PIN_B)) {
		ch = input::KEY_ESC;
	}

	if (m_PrevChar == ch) {
		return input::KEY_NOT_DEFINED;
	}

	m_PrevChar = ch;

	return ch;
}
#endif
