/**
 * @file hal_i2c.c
 *
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "hal_i2c.h"

bool i2c_is_connected(const uint8_t nAddress, const uint32_t nBaudrate) {
	FUNC_PREFIX(i2c_set_address	(nAddress));
	FUNC_PREFIX(i2c_set_baudrate(nBaudrate));

	uint8_t nResult;
	char buffer;

	if ((nAddress >= 0x30 && nAddress <= 0x37) || (nAddress >= 0x50 && nAddress <= 0x5F)) {
		nResult = FUNC_PREFIX(i2c_read(&buffer, 1));
	} else {
		/* This is known to corrupt the Atmel AT24RF08 EEPROM */
		nResult = FUNC_PREFIX(i2c_write(nullptr, 0));
	}

	return (nResult == 0) ? true : false;
}

void i2c_write_register(const uint8_t nRegister, const uint8_t nValue) {
	char buffer[2];

	buffer[0] = static_cast<char>(nRegister);
	buffer[1] = static_cast<char>(nValue);

	FUNC_PREFIX(i2c_write(buffer, 2));
}

void i2c_read_register(const uint8_t nRegister, uint8_t& nValue) {
	char buffer[1];

	buffer[0] = static_cast<char>(nRegister);

	FUNC_PREFIX(i2c_write(buffer, 1));
	FUNC_PREFIX(i2c_read(buffer, 1));

	nValue = buffer[0];
}
