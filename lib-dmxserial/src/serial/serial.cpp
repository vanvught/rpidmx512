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

#include "serial.h"

#include "debug.h"

Serial *Serial::s_pThis = 0;

using namespace serial;

Serial::Serial(void) : m_tType(type::UART) {
	DEBUG_ENTRY

	assert(s_pThis == 0);
	s_pThis = this;

	m_UartConfiguration.nBaud = 115200;
	m_UartConfiguration.nBits = 8;
	m_UartConfiguration.tParity = uart::parity::NONE;
	m_UartConfiguration.nStopBits = 1;

	m_SpiConfiguration.nSpeed = 1000000; // 1MHz
	m_SpiConfiguration.nMode = 0;

	m_I2cConfiguration.nAddress = 0x30;
	m_I2cConfiguration.tMode = i2c::speed::FAST;

	DEBUG_EXIT
}

Serial::~Serial(void) {
	DEBUG_ENTRY

	s_pThis = 0;

	DEBUG_EXIT
}

void Serial::Send(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY
	debug_dump(const_cast<uint8_t *>(pData), nLength);

	if (m_tType == type::UART) {
		SendUart(pData, nLength);
	}

	if (m_tType == type::SPI) {
		SendSpi(pData, nLength);
	}

	if (m_tType == type::I2C) {
		SendI2c(pData, nLength);
	}

	DEBUG_EXIT
}

void Serial::Print(void) {
	printf("Serial [%s]\n", GetType(m_tType));

	if (m_tType == type::UART) {
		printf(" Baud     : %d\n", m_UartConfiguration.nBaud);
		printf(" Bits     : %d\n", m_UartConfiguration.nBits);
		printf(" Parity   : %s\n", GetUartParity(m_UartConfiguration.tParity));
		printf(" StopBits : %d\n", m_UartConfiguration.nStopBits);
		return;
	}

	if (m_tType == type::SPI) {
		printf(" Speed : %d Hz\n", m_SpiConfiguration.nSpeed);
		printf(" Mode  : %d\n", m_SpiConfiguration.nMode);
		return;
	}

	if (m_tType == type::I2C) {
		printf(" Address    : %.2x\n", m_I2cConfiguration.nAddress);
		printf(" Speed mode : %s\n", GetI2cSpeed(m_I2cConfiguration.tMode));
		return;
	}
}
