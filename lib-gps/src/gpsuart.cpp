/**
 * @file gpsuart.cpp
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

#include "gps.h"

#include "hal_uart.h"

#include "debug.h"

namespace gps {
enum class State {
	START_DELIMITER, DATA, CHECKSUM_1, CHECKSUM_2, CR, LF
};

static constexpr uint32_t RING_BUFFER_INDEX_ENTRIES = (1 << 4);
static constexpr uint32_t RING_BUFFER_INDEX_MASK = (RING_BUFFER_INDEX_ENTRIES - 1);
}  // namespace gps

using namespace gps;

static uint8_t s_RingBuffer[RING_BUFFER_INDEX_ENTRIES][nmea::MAX_SENTENCE_LENGTH];
static uint32_t s_nRingBufferIndexHead;
static uint32_t s_nRingBufferIndexTail;
static uint32_t s_nDataIndex;
static uint8_t s_nChecksum;
static State s_State;

void GPS::UartInit() {
	DEBUG_ENTRY

	s_nRingBufferIndexHead = 0;
	s_nRingBufferIndexTail = 0;
	s_State = State::START_DELIMITER;

	FUNC_PREFIX (uart_begin(EXT_UART_BASE, 9600, hal::uart::BITS_8, hal::uart::PARITY_NONE, hal::uart::STOP_1BIT));

	DEBUG_EXIT
}

void GPS::UartSetBaud(uint32_t nBaud) {
	DEBUG_ENTRY
	assert(nBaud != 0);

	FUNC_PREFIX (uart_set_baudrate(EXT_UART_BASE, nBaud));
	m_nBaud = nBaud;

	DEBUG_EXIT
}

void GPS::UartSend(const char *pSentence) {
	DEBUG_ENTRY

	FUNC_PREFIX (uart_transmit_string(EXT_UART_BASE, pSentence));

	DEBUG_EXIT
}

const char* GPS::UartGetSentence() {
	uint32_t rfl = FUNC_PREFIX (uart_get_rx_fifo_level(EXT_UART_BASE));

	while (rfl--) {
		const auto nByte = FUNC_PREFIX (uart_get_rx_data(EXT_UART_BASE));

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
		case State::CHECKSUM_1: {
			const auto nNibble = nByte > '9' ? static_cast<uint8_t>(nByte - 'A' + 10) : static_cast<uint8_t>(nByte - '0');
			if (nNibble == ((s_nChecksum >> 4) & 0xF)) {
				s_State = State::CHECKSUM_2;
			} else {
				s_State = State::START_DELIMITER;
			}
		}
			break;
		case State::CHECKSUM_2: {
			const auto nNibble = nByte > '9' ? static_cast<uint8_t>(nByte - 'A' + 10) : static_cast<uint8_t>(nByte - '0');
			if (nNibble == (s_nChecksum & 0xF)) {
				s_State = State::CR;
			} else {
				s_State = State::START_DELIMITER;
			}
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
			assert(0);
			__builtin_unreachable();
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
