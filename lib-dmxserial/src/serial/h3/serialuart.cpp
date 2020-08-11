/**
 * @file serialuart.cpp
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

#include "h3.h"
#include "h3_gpio.h"
#include "h3_ccu.h"
#include "uart.h"

#include "arm/synchronize.h"

#include "debug.h"

using namespace serial;

void Serial::SetUartBaud(uint32_t nBaud) {
	DEBUG_PRINTF("nBaud=%d", nBaud);

	if (nBaud == 0) {
		return;
	}

	if (((24000000 / 16) / nBaud) > static_cast<uint16_t>(~0)) {
		DEBUG_PUTS("Baud is too low");
		return;
	}

	if (((24000000 / 16) / nBaud) == 0) {
		DEBUG_PUTS("Baud is too high");
		return;
	}

	m_UartConfiguration.nBaud = nBaud;
}

void Serial::SetUartBits(uint8_t nBits) {
	DEBUG_PRINTF("nBits=%d", nBits);

	if ((nBits >= 5) && (nBits <= 9)) {
		m_UartConfiguration.nBits = nBits;
	}
}

void Serial::SetUartParity(uart::parity tParity) {
	DEBUG_PRINTF("tParity=%d", tParity);

	if (tParity >= uart::parity::UNDEFINED ) {
		return;
	}

	m_UartConfiguration.tParity = tParity;
}

void Serial::SetUartStopBits(uint8_t nStopBits) {
	DEBUG_PRINTF("nStopBits=%d", nStopBits);

	if ((nStopBits == 1) || (nStopBits == 2)) {
		m_UartConfiguration.nStopBits = nStopBits;
	}
}

bool Serial::InitUart(void) {
	DEBUG_ENTRY

	uint32_t value = H3_PIO_PORTG->CFG0;
	// PG6, TX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG6_SELECT_CFG0_SHIFT));
	value |= H3_PG6_SELECT_UART1_TX << PG6_SELECT_CFG0_SHIFT;
	// PG7, RX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PG7_SELECT_CFG0_SHIFT));
	value |= H3_PG7_SELECT_UART1_RX << PG7_SELECT_CFG0_SHIFT;
	H3_PIO_PORTG->CFG0 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART1;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART1;

	assert(m_UartConfiguration.nBaud != 0);
	const uint32_t nDivisor = (24000000 / 16) / m_UartConfiguration.nBaud;

	uint32_t nRegisterLCR = 0;

	switch (m_UartConfiguration.nBits) {
	case 5:
		nRegisterLCR |= UART_LCR_DLS_5BITS;
		break;
	case 6:
		nRegisterLCR |= UART_LCR_DLS_6BITS;
		break;
	case 7:
		nRegisterLCR |= UART_LCR_DLS_7BITS;
		break;
	case 8:
		nRegisterLCR |= UART_LCR_DLS_8BITS;
		break;
	default:
		nRegisterLCR |= UART_LCR_DLS_8BITS;
	}

	if (m_UartConfiguration.tParity != uart::parity::NONE) {
		nRegisterLCR |= UART_LCR_PEN;

		if (m_UartConfiguration.tParity == uart::parity::ODD) {
			nRegisterLCR |= UART_LCR_EPS_ODD;
		} else if (m_UartConfiguration.tParity == uart::parity::EVEN) {
			nRegisterLCR |= UART_LCR_EPS_EVEN;
		}
	}

	switch (m_UartConfiguration.nStopBits) {
	case 1:
		nRegisterLCR |= UART_LCR_STOP_1BIT;
		break;
	case 2:
		nRegisterLCR |= UART_LCR_STOP_2BITS;
		break;
	default:
		nRegisterLCR |= UART_LCR_STOP_1BIT;
	}

	debug_print_bits(nRegisterLCR);

	dmb();
	H3_UART1->O08.FCR = 0;
	H3_UART1->LCR = UART_LCR_DLAB;
	H3_UART1->O00.DLL = nDivisor & 0xFF;
	H3_UART1->O04.DLH = (nDivisor >> 8);
	H3_UART1->LCR = nRegisterLCR;
	H3_UART1->O08.FCR = UART_FCR_EFIFO | UART_FCR_TRESET;
	H3_UART1->O04.IER = 0;
	isb();

	DEBUG_EXIT
	return true;
}



void Serial::SendUart(const uint8_t *pData, uint32_t nLength) {
	DEBUG_ENTRY

	const uint8_t *p = pData;

	while (nLength > 0) {
		uint32_t nAvailable = 64 - H3_UART1->TFL;

		while ((nLength > 0) && (nAvailable > 0)) {
			H3_UART1->O00.THR = static_cast<uint32_t>(*p);
			nLength--;
			nAvailable--;
			p++;
		}
	}

	DEBUG_EXIT
}
