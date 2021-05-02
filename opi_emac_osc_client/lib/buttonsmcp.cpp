/**
 * @file buttonsmcp.cpp
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
#include <assert.h>

#include "buttonsmcp.h"
#include "oscclient.h"

#include "board/h3_opi_zero.h"
#include "h3_gpio.h"		//TODO Remove H3 dependency

#include "hal_i2c.h"
#include "mcp23x17.h"

#include "debug.h"

namespace mcp23017 {
static constexpr auto address = 0x20;
}

namespace gpio {
static constexpr auto interrupt = GPIO_EXT_12; // PA7
}

ButtonsMcp::ButtonsMcp(OscClient *pOscClient):
	m_I2C(mcp23017::address),
	m_pOscClient(pOscClient),
	m_bIsConnected(false),
	m_nButtonsPrevious(0),
	m_nPortB(0)
{
	assert(m_pOscClient != nullptr);
}

bool ButtonsMcp::Start() {
	DEBUG_ENTRY

	m_bIsConnected = m_I2C.IsConnected();

	if (!m_bIsConnected) {
		DEBUG_EXIT
		return false;
	}

	// Switches
	m_I2C.WriteRegister(mcp23x17::reg::IODIRA, static_cast<uint8_t>(0xFF)); // All input
	m_I2C.WriteRegister(mcp23x17::reg::GPPUA, static_cast<uint8_t>(0xFF));	// Pull-up
	m_I2C.WriteRegister(mcp23x17::reg::IPOLA, static_cast<uint8_t>(0xFF));	// Invert read
	m_I2C.WriteRegister(mcp23x17::reg::INTCONA, static_cast<uint8_t>(0x00));
	m_I2C.WriteRegister(mcp23x17::reg::GPINTENA, static_cast<uint8_t>(0xFF));// Interrupt on Change
	m_I2C.ReadRegister(mcp23x17::reg::INTCAPA);								// Clear interrupt
	// Led's
	m_I2C.WriteRegister(mcp23x17::reg::IODIRB, static_cast<uint8_t>(0x00)); // All output
	m_I2C.WriteRegister(mcp23x17::reg::GPIOB, static_cast<uint8_t>(0x00));	// All led's Off

	h3_gpio_fsel(gpio::interrupt, GPIO_FSEL_INPUT); // PA7
	h3_gpio_pud(gpio::interrupt, GPIO_PULL_UP);

	m_nButtonsCount = 8;

	DEBUG_EXIT
	return true;
}

void ButtonsMcp::Stop() {
	h3_gpio_fsel(gpio::interrupt, GPIO_FSEL_DISABLE);
}

void ButtonsMcp::Run() {
	if (__builtin_expect(h3_gpio_lev(gpio::interrupt) == LOW, 0)) {

		const uint8_t nButtons = m_I2C.ReadRegister(mcp23x17::reg::GPIOA);
		const uint8_t nButtonsChanged = (nButtons ^ m_nButtonsPrevious) & nButtons;

		m_nButtonsPrevious = nButtons;

		/* P = m_nButtonsPrevious
		 * N = m_nButtons
		 * X = m_nButtons ^ m_nButtonsPrevious
		 * C = nButtonsChanged
		 *
		 * P N	X N	C
		 * 0 0	0 0	0
		 * 0 1	1 1	1
		 * 1 0	1 0	0
		 * 1 1	0 1	0
		 */

		DEBUG_PRINTF("%.2x %.2x", nButtons, nButtonsChanged);

		for (uint32_t i = 0; i < 8; i++) {
			if ((nButtonsChanged & (1U << i)) == ((1U << i))) {
				m_pOscClient->SendCmd(i);
			}
		}
	}
}

void ButtonsMcp::SetLed(uint8_t nLed, bool bOn) {
	DEBUG_PRINTF("led%d %s", nLed, bOn ? "On" : "Off");

	m_nPortB &= ~(1U << nLed);

	if (bOn) {
		m_nPortB |= (1U << nLed);
	}

	m_I2C.WriteRegister(mcp23x17::reg::GPIOB, m_nPortB);

	DEBUG_PRINTF("%.2x", m_nPortB);
}
