#if !defined (__CYGWIN__)
/**
 * @file buttonsadafruit.cpp
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>

#include "bcm2835.h"
#if defined(__linux__)
#else
#include "bcm2835_gpio.h"
#endif

#include "input.h"

#include "buttonsadafruit.h"

#define PIN_L RPI_V2_GPIO_P1_13	///< BCM 27
#define PIN_R RPI_V2_GPIO_P1_16	///< BCM 23
#define PIN_C RPI_V2_GPIO_P1_07	///< BCM 4
#define PIN_U RPI_V2_GPIO_P1_11	///< BCM 17
#define PIN_D RPI_V2_GPIO_P1_15	///< BCM 22
#define PIN_A RPI_V2_GPIO_P1_29	///< BCM 5
#define PIN_B RPI_V2_GPIO_P1_31 ///< BCM 6

#define MASK_GPLEV0  (uint32_t)((1 << PIN_L) | (1 << PIN_R)  | (1 << PIN_C) | (1 << PIN_U) | (1 << PIN_D) | (1 << PIN_A) | (1 << PIN_B))

ButtonsAdafruit::ButtonsAdafruit(void): m_rMaskedBits(0), m_PrevChar(INPUT_KEY_NOT_DEFINED) {
}

ButtonsAdafruit::~ButtonsAdafruit(void) {
}

bool ButtonsAdafruit::Start(void) {
#if defined(__linux__)
	bcm2835_init();
#endif
	InitGpioPin(PIN_L);
	InitGpioPin(PIN_R);
	InitGpioPin(PIN_C);
	InitGpioPin(PIN_U);
	InitGpioPin(PIN_D);
	InitGpioPin(PIN_A);
	InitGpioPin(PIN_B);

	return true;
}

bool ButtonsAdafruit::IsAvailable(void) {
#if defined(__linux__)
	volatile uint32_t* paddr = bcm2835_gpio + BCM2835_GPLEV0/4;
	const uint32_t reg = bcm2835_peri_read(paddr);
#else
	__sync_synchronize();
	const uint32_t reg = BCM2835_GPIO->GPLEV0;
#endif

	m_rMaskedBits = ~reg & MASK_GPLEV0;

	if (m_rMaskedBits != 0) {
		return true;
	}

	m_PrevChar = INPUT_KEY_NOT_DEFINED;

	return false;
}

int ButtonsAdafruit::GetChar(void) {
	int ch = INPUT_KEY_NOT_DEFINED;

	if ((m_rMaskedBits & (1 << PIN_L)) == (1 << PIN_L)) {
		ch = INPUT_KEY_LEFT;
	} else if ((m_rMaskedBits & (1 << PIN_R)) == (1 << PIN_R)) {
		ch = INPUT_KEY_RIGHT;
	} else if ((m_rMaskedBits & (1 << PIN_C)) == (1 << PIN_C)) {
		ch = INPUT_KEY_ENTER;
	} else if ((m_rMaskedBits & (1 << PIN_U)) == (1 << PIN_U)) {
		ch = INPUT_KEY_UP;
	} else if ((m_rMaskedBits & (1 << PIN_D)) == (1 << PIN_D)) {
		ch = INPUT_KEY_DOWN;
	} else if ((m_rMaskedBits & (1 << PIN_A)) == (1 << PIN_A)) {
		ch = INPUT_KEY_ENTER;
	} else if ((m_rMaskedBits & (1 << PIN_B)) == (1 << PIN_B)) {
		ch = INPUT_KEY_ESC;
	}

	if (m_PrevChar == ch) {
		return INPUT_KEY_NOT_DEFINED;
	}

	m_PrevChar = ch;

	return ch;
}

void ButtonsAdafruit::InitGpioPin(const uint8_t nPin) {
    bcm2835_gpio_fsel(nPin,  BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(nPin, BCM2835_GPIO_PUD_UP);
}

#endif
