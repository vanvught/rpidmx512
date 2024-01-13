/**
 * @file buttonsgpio.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef NDEBUG
 #include <cstdio>
#endif
#include <cassert>

#include "buttonsgpio.h"
#include "oscclient.h"

#include "hal_gpio.h"

#include "debug.h"

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1U << x)) == (1U << x))

#define BUTTON0_GPIO		GPIO_EXT_13
#define BUTTON1_GPIO		GPIO_EXT_11
#define BUTTON2_GPIO		GPIO_EXT_22
#define BUTTON3_GPIO		GPIO_EXT_15

#define INT_MASK			((1U << BUTTON0_GPIO) |  (1U << BUTTON1_GPIO) | (1U << BUTTON2_GPIO) | (1U << BUTTON3_GPIO))

#define LED0_GPIO			GPIO_EXT_7
#define LED1_GPIO			GPIO_EXT_12
#define LED2_GPIO			GPIO_EXT_26
#define LED3_GPIO			GPIO_EXT_18

ButtonsGpio::ButtonsGpio(OscClient *pOscClient) : m_pOscClient(pOscClient) {
	assert(m_pOscClient != nullptr);
}

bool ButtonsGpio::Start() {
	FUNC_PREFIX(gpio_fsel(LED0_GPIO, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(LED1_GPIO, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(LED2_GPIO, GPIO_FSEL_OUTPUT));
	FUNC_PREFIX(gpio_fsel(LED3_GPIO, GPIO_FSEL_OUTPUT));

	FUNC_PREFIX(gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_EINT));
	FUNC_PREFIX(gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_EINT));
	FUNC_PREFIX(gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_EINT));
	FUNC_PREFIX(gpio_fsel(BUTTON3_GPIO, GPIO_FSEL_EINT));

	FUNC_PREFIX(gpio_set_pud(BUTTON0_GPIO, GPIO_PULL_UP));
	FUNC_PREFIX(gpio_set_pud(BUTTON1_GPIO, GPIO_PULL_UP));
	FUNC_PREFIX(gpio_set_pud(BUTTON2_GPIO, GPIO_PULL_UP));
	FUNC_PREFIX(gpio_set_pud(BUTTON3_GPIO, GPIO_PULL_UP));

	FUNC_PREFIX(gpio_int_cfg(BUTTON0_GPIO, GPIO_INT_CFG_NEG_EDGE));
	FUNC_PREFIX(gpio_int_cfg(BUTTON1_GPIO, GPIO_INT_CFG_NEG_EDGE));
	FUNC_PREFIX(gpio_int_cfg(BUTTON2_GPIO, GPIO_INT_CFG_NEG_EDGE));
	FUNC_PREFIX(gpio_int_cfg(BUTTON3_GPIO, GPIO_INT_CFG_NEG_EDGE));

	H3_PIO_PA_INT->CTL |= INT_MASK;
	H3_PIO_PA_INT->STA = INT_MASK;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7U << 4);

	m_nButtonsCount = 4;

	return true;
}

void ButtonsGpio::Stop() {
	FUNC_PREFIX(gpio_fsel(BUTTON0_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(BUTTON1_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(BUTTON2_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(BUTTON3_GPIO, GPIO_FSEL_DISABLE));

	FUNC_PREFIX(gpio_fsel(LED0_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(LED1_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(LED2_GPIO, GPIO_FSEL_DISABLE));
	FUNC_PREFIX(gpio_fsel(LED3_GPIO, GPIO_FSEL_DISABLE));
}

void ButtonsGpio::Run() {
	m_nButtons = H3_PIO_PA_INT->STA & INT_MASK;

	if (__builtin_expect((m_nButtons != 0), 0)) {
		H3_PIO_PA_INT->STA = INT_MASK;

		DEBUG_PRINTF("%d-%d-%d-%d", BUTTON(BUTTON0_GPIO), BUTTON(BUTTON1_GPIO), BUTTON(BUTTON2_GPIO), BUTTON(BUTTON3_GPIO));

		if (BUTTON_STATE(BUTTON0_GPIO)) {
			m_pOscClient->SendCmd(0);
			DEBUG_PUTS("BUTTON0_GPIO");
		}

		if (BUTTON_STATE(BUTTON1_GPIO)) {
			m_pOscClient->SendCmd(1);
			DEBUG_PUTS("BUTTON1_GPIO");
		}

		if (BUTTON_STATE(BUTTON2_GPIO)) {
			m_pOscClient->SendCmd(2);
			DEBUG_PUTS("BUTTON2_GPIO");
		}

		if (BUTTON_STATE(BUTTON3_GPIO)) {
			m_pOscClient->SendCmd(3);
			DEBUG_PUTS("BUTTON3_GPIO");
		}
	}
}

void ButtonsGpio::SetLed(const uint32_t nLed, const bool bOn) {
	DEBUG_PRINTF("led%d %s", nLed, bOn ? "On" : "Off");

	switch (nLed) {
	case 0:
		bOn ? FUNC_PREFIX(gpio_set(LED0_GPIO)) : FUNC_PREFIX(gpio_clr(LED0_GPIO));
		break;
	case 1:
		bOn ? FUNC_PREFIX(gpio_set(LED1_GPIO)) : FUNC_PREFIX(gpio_clr(LED1_GPIO));
		break;
	case 2:
		bOn ? FUNC_PREFIX(gpio_set(LED2_GPIO)) : FUNC_PREFIX(gpio_clr(LED2_GPIO));
		break;
	case 3:
		bOn ? FUNC_PREFIX(gpio_set(LED3_GPIO)) : FUNC_PREFIX(gpio_clr(LED3_GPIO));
		break;
	default:
		break;
	}
}
