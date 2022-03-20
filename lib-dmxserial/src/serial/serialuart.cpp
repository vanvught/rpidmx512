/**
 * @file serialuart.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "serial.h"

#include "hal_uart.h"

#include "debug.h"

using namespace serial;

void Serial::SetUartBaud(uint32_t nBaud) {
	DEBUG_PRINTF("nBaud=%d", nBaud);

	m_UartConfiguration.nBaud = nBaud;
}

void Serial::SetUartBits(uint32_t nBits) {
	DEBUG_PRINTF("nBits=%d", nBits);

	if ((nBits >= 5) && (nBits <= 9)) {
		m_UartConfiguration.nBits = static_cast<uint8_t>(nBits);
	}
}

void Serial::SetUartParity(uart::parity tParity) {
	DEBUG_PRINTF("tParity=%d", tParity);

	switch (tParity) {
	case uart::parity::ODD:
		m_UartConfiguration.nParity = static_cast<uint8_t>(hal::uart::PARITY_ODD);
		break;
	case uart::parity::EVEN:
		m_UartConfiguration.nParity = static_cast<uint8_t>(hal::uart::PARITY_EVEN);
		break;
	default:
		m_UartConfiguration.nParity = static_cast<uint8_t>(hal::uart::PARITY_NONE);
		break;
	}
}

void Serial::SetUartStopBits(uint32_t nStopBits) {
	DEBUG_PRINTF("nStopBits=%d", nStopBits);

	if ((nStopBits == 1) || (nStopBits == 2)) {
		m_UartConfiguration.nStopBits = static_cast<uint8_t>(nStopBits);
	}
}

bool Serial::InitUart() {
	DEBUG_ENTRY

	FUNC_PREFIX (uart_begin(EXT_UART_BASE, m_UartConfiguration.nBaud, m_UartConfiguration.nBits, m_UartConfiguration.nParity, m_UartConfiguration.nStopBits));

	DEBUG_EXIT
	return true;
}

void Serial::SendUart(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	FUNC_PREFIX (uart_transmit(EXT_UART_BASE, pData, nLength));

	DEBUG_EXIT
}
