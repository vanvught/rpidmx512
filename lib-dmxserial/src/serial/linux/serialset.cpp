/**
 * @file serial.cpp
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

#include "debug.h"

/*
 * UART
 */
void Serial::SetUartBaud(uint32_t nBaud) {
	DEBUG_PRINTF("nBaud=%d", nBaud);

	m_UartConfiguration.nBaud = nBaud;
}

void Serial::SetUartBits(__attribute__((unused)) uint8_t nBits) {
	DEBUG_PRINTF("nBits=%d", nBits);

}

void Serial::SetUartParity(__attribute__((unused)) serial::uart::parity tParity) {
	DEBUG_PRINTF("tParity=%d", static_cast<int>(tParity));

}

void Serial::SetUartStopBits(__attribute__((unused)) uint8_t nStopBits) {
	DEBUG_PRINTF("nStopBits=%d", nStopBits);

}

/*
 * SPI
 */
void Serial::SetSpiSpeedHz(__attribute__((unused)) uint32_t nSpeedHz) {
	DEBUG_PRINTF("nSpeedHz=%d", nSpeedHz);
}

void Serial::SetSpiMode(__attribute__((unused)) serial::spi::mode tMode) {
	DEBUG_PRINTF("tMode=%d", static_cast<int>(tMode));
}

/*
 * I2C
 */
void Serial::SetI2cAddress(__attribute__((unused)) uint8_t nAddress) {
	DEBUG_PRINTF("nAddress=%.x", nAddress);
}

void Serial::SetI2cSpeedMode(__attribute__((unused)) serial::i2c::speed tSpeedMode) {
	DEBUG_PRINTF("tSpeedMode=%.x", static_cast<int>(tSpeedMode));
}



