/**
 * @file seriali2c.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "./serial.h"

#include "hal_i2c.h"

#include "debug.h"

using namespace serial;

void Serial::SetI2cAddress(uint8_t nAddress) {
	DEBUG_PRINTF("nAddress=%.x", nAddress);

	m_I2cConfiguration.nAddress = nAddress;
}

void Serial::SetI2cSpeedMode(i2c::speed tSpeedMode) {
	DEBUG_PRINTF("tSpeedMode=%.x", tSpeedMode);

	if (tSpeedMode == i2c::speed::NORMAL) {
		m_I2cConfiguration.nSpeed = HAL_I2C::NORMAL_SPEED;
	} else {
		m_I2cConfiguration.nSpeed = HAL_I2C::FULL_SPEED;
	}
}

bool Serial::InitI2c() {
	DEBUG_ENTRY

	FUNC_PREFIX (i2c_begin());
	FUNC_PREFIX (i2c_set_baudrate(m_I2cConfiguration.nSpeed));

	DEBUG_EXIT
	return true;
}

void Serial::SendI2c(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	FUNC_PREFIX (i2c_set_address(m_I2cConfiguration.nAddress));
	FUNC_PREFIX (i2c_write(reinterpret_cast<const char*>(pData), nLength));

	DEBUG_EXIT
}
