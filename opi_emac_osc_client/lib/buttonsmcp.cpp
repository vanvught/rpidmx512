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
#include "h3_gpio.h"

#include "i2c.h"

#include "debug.h"

#define MCP23017_I2C_ADDRESS	0x20

#define MCP23X17_IOCON			0x0A

#define MCP23X17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPINTENA		0x04	///< INTERRUPT-ON-CHANGE CONTROL (GPINTENA) REGISTER, 0 = No Interrupt on Change (default), 1 = Interrupt on Change
#define MCP23X17_GPPUA			0x0C	///< PULL-UP RESISTOR CONFIGURATION (GPPUA) REGISTER, INPUT ONLY: 0 = No Internal 100k Pull-Up (default) 1 = Internal 100k Pull-Up
#define MCP23X17_INTCAPA		0x10	///< INTERRUPT CAPTURE (INTCAPA) REGISTER, READ ONLY: State of the Pin at the Time the Interrupt Occurred
#define MCP23X17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

#define MCP23X17_IODIRB			0x01	///< I/O DIRECTION (IODIRB) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPIOB			0x13	///< PORT (GPIOB) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

#define GPIO_INTA				GPIO_EXT_12 // PA7

ButtonsMcp::ButtonsMcp(OscClient *pOscClient):
	m_pOscClient(pOscClient),
	m_bIsConnected(false),
	m_nButtons(0x00),
	m_nButtonsPrevious(0x00),
	m_nPortB(0x00)
{
	assert(m_pOscClient != 0);
}

ButtonsMcp::~ButtonsMcp(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

bool ButtonsMcp::Start(void) {
	DEBUG_ENTRY

	i2c_set_baudrate(I2C_FULL_SPEED);

	m_bIsConnected = i2c_is_connected(MCP23017_I2C_ADDRESS);

	if (!m_bIsConnected) {
		DEBUG_EXIT
		return false;
	}

	i2c_set_address(MCP23017_I2C_ADDRESS);

	// Switches
	i2c_write_reg_uint8(MCP23X17_IODIRA, 0xFF); 	// All input
	i2c_write_reg_uint8(MCP23X17_GPPUA, 0xFF);		// Pull-up
	i2c_write_reg_uint8(MCP23X17_GPINTENA, 0xFF);	// Interrupt on Change
	i2c_read_reg_uint8(MCP23X17_INTCAPA);			// Clear interrupt
	// Led's
	i2c_write_reg_uint8(MCP23X17_IODIRB, 0x00); 	// All output
	i2c_write_reg_uint8(MCP23X17_GPIOB, 0);			// All led's Off

	h3_gpio_fsel(GPIO_INTA, GPIO_FSEL_INPUT); 		// PA7
	h3_gpio_pud(GPIO_INTA, GPIO_PULL_UP);

	m_nButtonsCount = 8;

	DEBUG_EXIT
	return true;
}

void ButtonsMcp::Stop(void) {
	h3_gpio_fsel(GPIO_INTA, GPIO_FSEL_DISABLE);
}

void ButtonsMcp::Run(void) {
	if (h3_gpio_lev(GPIO_INTA) == LOW) {

		i2c_set_address(MCP23017_I2C_ADDRESS);
		m_nButtonsPrevious = m_nButtons;
		m_nButtons = ~i2c_read_reg_uint8(MCP23X17_GPIOA);

		const uint8_t nButtonsChanged = (m_nButtons ^ m_nButtonsPrevious) & m_nButtons;

		DEBUG_PRINTF("%.2x", nButtonsChanged);

		for (uint32_t i = 0; i < 8; i++) {
			if ((nButtonsChanged & (1 << i)) == ((1 << i))) {
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

	i2c_set_address(MCP23017_I2C_ADDRESS);
	i2c_write_reg_uint8(MCP23X17_GPIOB, m_nPortB);

	DEBUG_PRINTF("%.2x", m_nPortB);
}
