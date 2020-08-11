/**
 * @file seriali2c.cpp
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "../src/serial/serial.h"

#include "h3_i2c.h"	// TODO Replace with hal_i2c.h ?

#include "debug.h"

using namespace serial;

void Serial::SetI2cAddress(uint8_t nAddress) {
	DEBUG_PRINTF("nAddress=%.x", nAddress);

	m_I2cConfiguration.nAddress = nAddress;
}

void Serial::SetI2cSpeedMode(i2c::speed tSpeedMode) {
	DEBUG_PRINTF("tSpeedMode=%.x", tSpeedMode);

	if (tSpeedMode >= i2c::speed::UNDEFINED) {
		return;
	}

	m_I2cConfiguration.tMode = tSpeedMode;
}

bool Serial::InitI2c(void) {
	DEBUG_ENTRY

	h3_i2c_begin();
	h3_i2c_set_baudrate(m_I2cConfiguration.tMode == i2c::speed::NORMAL ? H3_I2C_NORMAL_SPEED : H3_I2C_FULL_SPEED);

	DEBUG_EXIT
	return true;
}

void Serial::SendI2c(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	h3_i2c_set_slave_address(m_I2cConfiguration.nAddress);
	h3_i2c_write(reinterpret_cast<const char*>(pData), nLength);

	DEBUG_EXIT
}
