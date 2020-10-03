/**
 * @file gpsuart.cpp
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

#include "gps.h"

#include "h3_board.h"

#include "h3.h"
#include "h3_gpio.h"
#include "h3_ccu.h"
#include "uart.h"

#include "arm/synchronize.h"

#include "debug.h"

enum class State {
	START_DELIMITER,
	DATA,
	CHECKSUM_1,
	CHECKSUM_2,
	CR,
	LF
};

using namespace gps;

static constexpr uint32_t RING_BUFFER_INDEX_ENTRIES = (1 << 4);
static constexpr uint32_t RING_BUFFER_INDEX_MASK = (RING_BUFFER_INDEX_ENTRIES - 1);

static uint8_t s_RingBuffer[RING_BUFFER_INDEX_ENTRIES][nmea::MAX_SENTENCE_LENGTH];
static uint32_t s_nRingBufferIndexHead;
static uint32_t s_nRingBufferIndexTail;
static uint32_t s_nDataIndex;
static uint8_t s_nChecksum;
static State s_State;

void GPS::UartInit() {
#if (EXT_UART_NUMBER == 1)
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
#elif (EXT_UART_NUMBER == 3)
	uint32_t value = H3_PIO_PORTA->CFG1;
	// PA13, TX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA13_SELECT_CFG1_SHIFT));
	value |= H3_PA13_SELECT_UART3_TX << PA13_SELECT_CFG1_SHIFT;
	// PA14, RX
	value &= static_cast<uint32_t>(~(GPIO_SELECT_MASK << PA14_SELECT_CFG1_SHIFT));
	value |= H3_PA14_SELECT_UART3_RX << PA14_SELECT_CFG1_SHIFT;
	H3_PIO_PORTA->CFG1 = value;

	H3_CCU->BUS_CLK_GATING3 |= CCU_BUS_CLK_GATING3_UART3;
	H3_CCU->BUS_SOFT_RESET4 |= CCU_BUS_SOFT_RESET4_UART3;
#else
# error Unsupported UART device configured
#endif

	UartSetBaud();
	isb();

	s_nRingBufferIndexHead = 0;
	s_nRingBufferIndexTail = 0;
	s_State = State::START_DELIMITER;
}

void GPS::UartSetBaud(uint32_t nBaud) {
	DEBUG_ENTRY

	assert(nBaud != 0);

	const uint32_t nDivisor = (24000000 / 16) / nBaud;

	assert(nDivisor <= static_cast<uint16_t>(~0)); // too low
	assert(nDivisor != 0); // too high

	dmb();
	EXT_UART->O08.FCR = 0;
	EXT_UART->LCR = UART_LCR_DLAB;
	EXT_UART->O00.DLL = nDivisor & 0xFF;
	EXT_UART->O04.DLH = (nDivisor >> 8);
	EXT_UART->O08.FCR = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRESET;
	EXT_UART->LCR = UART_LCR_8_N_1;
	isb();

	m_nBaud = nBaud;

	DEBUG_EXIT
}

void GPS::UartSend(const char *pSentence) {
	DEBUG_ENTRY

	const char *p = pSentence;

	while (*p != '\0') {
		while (!(EXT_UART->LSR & UART_LSR_THRE))
			;

		EXT_UART->O00.THR = static_cast<uint32_t>(*p);
		p++;
	}

	DEBUG_EXIT
}

const char* GPS::UartGetSentence() {
	uint8_t nNibble;
	uint32_t rfl = EXT_UART->RFL;

	while (rfl--) {
		const uint8_t nByte = EXT_UART->O00.RBR;

		switch (s_State) {
		case State::START_DELIMITER:
			if (nByte == nmea::START_DELIMITER) {
				s_State = State::DATA;
				s_nDataIndex = 0;
				s_nChecksum = 0;
			}
			break;
		case State::DATA:
			if (nByte != '*') {
				s_nChecksum ^= nByte;
			} else {
				s_State = State::CHECKSUM_1;
			}
			break;
		case State::CHECKSUM_1:
			nNibble = nByte > '9' ? static_cast<uint8_t>(nByte - 'A' + 10) : static_cast<uint8_t>(nByte - '0');
			if (nNibble == ((s_nChecksum >> 4) & 0xF)) {
				s_State = State::CHECKSUM_2;
			} else {
				s_State = State::START_DELIMITER;
			}
			break;
		case State::CHECKSUM_2:
			nNibble = nByte > '9' ? static_cast<uint8_t>(nByte - 'A' + 10) : static_cast<uint8_t>(nByte - '0');
			if (nNibble == (s_nChecksum & 0xF)) {
				s_State = State::CR;
			} else {
				s_State = State::START_DELIMITER;
			}
			break;
		case State::CR:
			if (nByte == '\r') {
				s_State = State::LF;
			} else {
				s_State = State::START_DELIMITER;
			}
			break;
		case State::LF:
			if (nByte == '\n') {
				s_RingBuffer[s_nRingBufferIndexHead][s_nDataIndex] = '\n';
				s_nRingBufferIndexHead = (s_nRingBufferIndexHead + 1) & RING_BUFFER_INDEX_MASK;
			}
			s_State = State::START_DELIMITER;
			break;
		default:
			s_State = State::START_DELIMITER;
			break;
		}

		if (s_State != State::START_DELIMITER) {
			s_RingBuffer[s_nRingBufferIndexHead][s_nDataIndex++] = nByte;
			if (s_nDataIndex == nmea::MAX_SENTENCE_LENGTH) {
				s_State = State::START_DELIMITER;
			}
		}
	}

	if (s_nRingBufferIndexHead == s_nRingBufferIndexTail) {
		return nullptr;
	} else {
		const char *p = reinterpret_cast<const char *>(&s_RingBuffer[s_nRingBufferIndexTail][0]);
		s_nRingBufferIndexTail = (s_nRingBufferIndexTail + 1) & RING_BUFFER_INDEX_MASK;
		return p;
	}
}
