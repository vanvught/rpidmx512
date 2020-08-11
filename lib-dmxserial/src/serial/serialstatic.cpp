/**
 * @file serialstatic.cpp
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
#include <string.h>

#include "serial.h"

using namespace serial;

constexpr char aType[type::UNDEFINED][5] = { "uart", "spi", "i2c" };
constexpr char aUartParity[uart::parity::UNDEFINED][5] = { "none", "odd", "even" };
constexpr char aI2cSpeed[i2c::speed::UNDEFINED][9] = { "standard", "fast" };

const char* Serial::GetType(type tType) {
	if (tType < type::UNDEFINED) {
		return aType[tType];
	}

	return "Undefined";
}

type Serial::GetType(const char *pType) {
	for (uint32_t i = 0; i < type::UNDEFINED; i++) {
		if (strcasecmp(aType[i], pType) == 0) {
			return static_cast<type>(i);
		}
	}

	return type::UART;
}

const char* Serial::GetUartParity(uart::parity tParity) {
	if (tParity < uart::parity::UNDEFINED) {
		return aUartParity[tParity];
	}

	return "Undefined";
}

uart::parity Serial::GetUartParity(const char *pParity) {
	for (uint32_t i = 0; i < uart::parity::UNDEFINED; i++) {
		if (strcasecmp(aUartParity[i], pParity) == 0) {
			return static_cast<uart::parity>(i);
		}
	}

	return uart::parity::NONE;
}

const char* Serial::GetI2cSpeed(i2c::speed tSpeed) {
	if (tSpeed < i2c::speed::UNDEFINED) {
		return aI2cSpeed[tSpeed];
	}

	return "Undefined";
}

i2c::speed Serial::GetI2cSpeed(const char *pSpeed) {
	for (uint32_t i = 0; i < i2c::speed::UNDEFINED; i++) {
		if (strcasecmp(aI2cSpeed[i], pSpeed) == 0) {
			return static_cast<i2c::speed>(i);
		}
	}

	return i2c::speed::FAST;
}
