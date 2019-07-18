/**
 * @file buttonsgpio.cpp
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "buttonsgpio.h"
#include "oscclient.h"

#include "h3_board.h"
#include "h3_gpio.h"

#include "arm/arm.h"
#include "arm/synchronize.h"
#include "arm/gic.h"

#include "debug.h"

volatile uint32_t s_nButtons;

#define BUTTON(x)			((m_nButtons >> x) & 0x01)
#define BUTTON_STATE(x)		((m_nButtons & (1 << x)) == (1 << x))

static void __attribute__((interrupt("FIQ"))) fiq_handler(void) {
	dmb();

	s_nButtons |= H3_PIO_PA_INT->STA & 0x0F;

	H3_PIO_PA_INT->STA = ~0x0;

	dmb();
}

ButtonsGpio::ButtonsGpio(OscClient* pOscClient):
	m_pOscClient(pOscClient),
	m_nButtons(0)
{
	assert(m_pOscClient != 0);
	s_nButtons = 0;
}

ButtonsGpio::~ButtonsGpio(void) {
}

bool ButtonsGpio::Start(void) {
	h3_gpio_fsel(GPIO_EXT_13, GPIO_FSEL_EINT);	// PA0
	h3_gpio_fsel(GPIO_EXT_11, GPIO_FSEL_EINT);	// PA1
	h3_gpio_fsel(GPIO_EXT_22, GPIO_FSEL_EINT);	// PA2
	h3_gpio_fsel(GPIO_EXT_15, GPIO_FSEL_EINT);	// PA3

#ifndef NDEBUG
	printf("H3_PIO_PORTA->PUL0=%p ", H3_PIO_PORTA->PUL0);
	debug_print_bits(H3_PIO_PORTA->PUL0);
#endif

	uint32_t value = H3_PIO_PORTA->PUL0;
	value &= ~((0x03 << 0) | (0x03 << 2) | (0x03 << 4) | (0x03 << 6));
	value |= (0x01 << 0) | (0x01 << 2) | (0x01 << 4) | (0x01 << 6); // Pull-Up
	H3_PIO_PORTA->PUL0 = value;

#ifndef NDEBUG
	printf("H3_PIO_PORTA->PUL0=%p ", H3_PIO_PORTA->PUL0);
	debug_print_bits(H3_PIO_PORTA->PUL0);
#endif

	arm_install_handler((unsigned) fiq_handler, ARM_VECTOR(ARM_VECTOR_FIQ));

	gic_fiq_config(H3_PA_EINT_IRQn, GIC_CORE0);

	H3_PIO_PA_INT->CFG0 = (0x1 << 0) | (0x1 << 4) |  (0x1 << 8) | (0x1 << 12); 									// Negative Edge
	H3_PIO_PA_INT->CTL = (1 << GPIO_EXT_13) |  (1 << GPIO_EXT_11) | (1 << GPIO_EXT_22) |  (1 << GPIO_EXT_15);	// PA0, PA1, PA2, PA3
	H3_PIO_PA_INT->STA = ~0x0;
	H3_PIO_PA_INT->DEB = (0x0 << 0) | (0x7 << 4);

#ifndef NDEBUG
	printf("H3_PIO_PA_INT->CFG0=%p ", H3_PIO_PA_INT->CFG0);
	debug_print_bits(H3_PIO_PA_INT->CFG0);
	printf("H3_PIO_PA_INT->CTL=%p ", H3_PIO_PA_INT->CTL);
	debug_print_bits(H3_PIO_PA_INT->CTL);
	printf("H3_PIO_PA_INT->DEB=%p ", H3_PIO_PA_INT->DEB);
	debug_print_bits(H3_PIO_PA_INT->DEB);
#endif

	__enable_fiq();

	return true;
}

bool ButtonsGpio::Stop(void) {
	h3_gpio_fsel(GPIO_EXT_13, GPIO_FSEL_DISABLE);	// PA0
	h3_gpio_fsel(GPIO_EXT_11, GPIO_FSEL_DISABLE);	// PA1
	h3_gpio_fsel(GPIO_EXT_22, GPIO_FSEL_DISABLE);	// PA2
	h3_gpio_fsel(GPIO_EXT_15, GPIO_FSEL_DISABLE);	// PA3

	__disable_fiq();

	s_nButtons = 0;

	return true;
}

void ButtonsGpio::Run(void) {

	dmb();
	if (s_nButtons != 0) {
		__disable_fiq();

		m_nButtons = s_nButtons;
		s_nButtons = 0;

		dmb();
		__enable_fiq();

		DEBUG_PRINTF("%d-%d-%d-%d", BUTTON(0), BUTTON(1), BUTTON(2), BUTTON(3));

		if (BUTTON_STATE(0)) {
			m_pOscClient->SendCmd(0);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(1)) {
			m_pOscClient->SendCmd(1);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(2)) {
			m_pOscClient->SendCmd(2);
			DEBUG_PUTS("");
		}

		if (BUTTON_STATE(3)) {
			m_pOscClient->SendCmd(3);
			DEBUG_PUTS("");
		}
	}
}
