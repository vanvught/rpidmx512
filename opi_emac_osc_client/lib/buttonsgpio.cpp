/**
 * @file buttonsgpio.cpp
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "buttonsgpio.h"
#include "oscclient.h"

#include "board/h3_opi_zero.h"
#include "h3_gpio.h"

#include "debug.h"

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1U << x)) == (1U << x))

#define BUTTON0_GPIO		GPIO_EXT_13		// PA0
#define BUTTON1_GPIO		GPIO_EXT_11		// PA1
#define BUTTON2_GPIO		GPIO_EXT_22		// PA2
#define BUTTON3_GPIO		GPIO_EXT_15		// PA3

#define INT_MASK			((1U << BUTTON0_GPIO) |  (1U << BUTTON1_GPIO) | (1U << BUTTON2_GPIO) | (1U << BUTTON3_GPIO))

#define LED0_GPIO			GPIO_EXT_7		// PA6
#define LED1_GPIO			GPIO_EXT_12		// PA7
#define LED2_GPIO			GPIO_EXT_26		// PA10
#define LED3_GPIO			GPIO_EXT_18		// PA18

ButtonsGpio::ButtonsGpio(OscClient *pOscClient) : m_pOscClient(pOscClient), m_nButtons(0) {
	assert(m_pOscClient != nullptr);
}

bool ButtonsGpio::Start() {
	h3_gpio_fsel(LED0_GPIO, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(LED1_GPIO, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(LED2_GPIO, GPIO_FSEL_OUTPUT);
	h3_gpio_fsel(LED3_GPIO, GPIO_FSEL_OUTPUT);

	h3_gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_EINT);
	h3_gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_EINT);
	h3_gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_EINT);
	h3_gpio_fsel(BUTTON3_GPIO, GPIO_FSEL_EINT);

	h3_gpio_pud(BUTTON0_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON1_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON2_GPIO, GPIO_PULL_UP);
	h3_gpio_pud(BUTTON3_GPIO, GPIO_PULL_UP);

	h3_gpio_int_cfg(BUTTON0_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON1_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON2_GPIO, GPIO_INT_CFG_NEG_EDGE);
	h3_gpio_int_cfg(BUTTON3_GPIO, GPIO_INT_CFG_NEG_EDGE);

	H3_PIO_PA_INT->CTL |= INT_MASK;
	H3_PIO_PA_INT->STA = INT_MASK;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);

#ifndef NDEBUG
	printf("H3_PIO_PORTA->PUL0=%p ", H3_PIO_PORTA->PUL0);
	debug_print_bits(H3_PIO_PORTA->PUL0);
	printf("H3_PIO_PA_INT->CFG0=%p ", H3_PIO_PA_INT->CFG0);
	debug_print_bits(H3_PIO_PA_INT->CFG0);
	printf("H3_PIO_PA_INT->CTL=%p ", H3_PIO_PA_INT->CTL);
	debug_print_bits(H3_PIO_PA_INT->CTL);
	printf("H3_PIO_PA_INT->DEB=%p ", H3_PIO_PA_INT->DEB);
	debug_print_bits(H3_PIO_PA_INT->DEB);
#endif

	m_nButtonsCount = 4;

	return true;
}

void ButtonsGpio::Stop() {
	h3_gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(BUTTON3_GPIO, GPIO_FSEL_DISABLE);

	h3_gpio_fsel(LED0_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(LED1_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(LED2_GPIO, GPIO_FSEL_DISABLE);
	h3_gpio_fsel(LED3_GPIO, GPIO_FSEL_DISABLE);
}

void ButtonsGpio::Run() {
	m_nButtons = H3_PIO_PA_INT->STA & INT_MASK;

	if (__builtin_expect((m_nButtons != 0), 0)) {
		H3_PIO_PA_INT->STA = INT_MASK;

		DEBUG_PRINTF("%d-%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO), BUTTON(BUTTON3_GPIO));

		if (BUTTON_STATE(BUTTON0_GPIO)) {
			m_pOscClient->SendCmd(0);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(BUTTON1_GPIO)) {
			m_pOscClient->SendCmd(1);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(BUTTON2_GPIO)) {
			m_pOscClient->SendCmd(2);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(BUTTON3_GPIO)) {
			m_pOscClient->SendCmd(3);
			DEBUG_PUTS("");
		}
	}
}

void ButtonsGpio::SetLed(uint8_t nLed, bool bOn) {
	DEBUG_PRINTF("led%d %s", nLed, bOn ? "On" : "Off");

	switch (nLed) {
	case 0:
		bOn ? h3_gpio_set(LED0_GPIO) : h3_gpio_clr(LED0_GPIO);
		break;
	case 1:
		bOn ? h3_gpio_set(LED1_GPIO) : h3_gpio_clr(LED1_GPIO);
		break;
	case 2:
		bOn ? h3_gpio_set(LED2_GPIO) : h3_gpio_clr(LED2_GPIO);
		break;
	case 3:
		bOn ? h3_gpio_set(LED3_GPIO) : h3_gpio_clr(LED3_GPIO);
		break;
	default:
		break;
	}
}

