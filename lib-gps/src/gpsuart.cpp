/**
 * @file gpsuart.cpp
 *
 */
/* Copyright (C) 2020-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cassert>

#include "gps.h"
#include "hal_uart.h"
#include "firmware/debug/debug_debug.h"

namespace gps {
enum class State { kStartDelimiter, kData, kChecksum1, kChecksum2, kCr, kLf };

static constexpr uint32_t kRingBufferIndexEntries = (1 << 4);
static constexpr uint32_t kRingBufferIndexMask = (kRingBufferIndexEntries - 1);
} // namespace gps

static uint8_t s_ring_buffer[gps::kRingBufferIndexEntries][gps::nmea::kMaxSentenceLength];
static uint32_t s_ring_buffer_index_head;
static uint32_t s_ring_buffer_index_tail;
static uint32_t s_data_index;
static uint8_t s_checksum;
static gps::State s_state;

void GPS::UartInit() {
    DEBUG_ENTRY();

    s_ring_buffer_index_head = 0;
    s_ring_buffer_index_tail = 0;
    s_state = gps::State::kStartDelimiter;

    FUNC_PREFIX(UartBegin(EXT_UART_BASE, 9600, hal::uart::BITS_8, hal::uart::PARITY_NONE, hal::uart::STOP_1BIT));

    DEBUG_EXIT();
}

void GPS::UartSetBaud(uint32_t baud) {
    DEBUG_ENTRY();
    assert(baud != 0);

    FUNC_PREFIX(UartSetBaudrate(EXT_UART_BASE, baud));
    baud_ = baud;

    DEBUG_EXIT();
}

void GPS::UartSend(const char* sentence) {
    DEBUG_ENTRY();

    FUNC_PREFIX(UartTransmitString(EXT_UART_BASE, sentence));

    DEBUG_EXIT();
}

const char* GPS::UartGetSentence() {
    uint32_t rfl = FUNC_PREFIX(UartGetRxFifoLevel(EXT_UART_BASE));

    while (rfl--) {
        const auto kByte = FUNC_PREFIX(UartGetRxData(EXT_UART_BASE));

        switch (s_state) {
            case gps::State::kStartDelimiter:
                if (kByte == gps::nmea::kStartDelimiter) {
                    s_state = gps::State::kData;
                    s_data_index = 0;
                    s_checksum = 0;
                }
                break;
            case gps::State::kData:
                if (kByte != '*') {
                    s_checksum ^= kByte;
                } else {
                    s_state = gps::State::kChecksum1;
                }
                break;
            case gps::State::kChecksum1: {
                const auto kNibble = kByte > '9' ? static_cast<uint8_t>(kByte - 'A' + 10) : static_cast<uint8_t>(kByte - '0');
                if (kNibble == ((s_checksum >> 4) & 0xF)) {
                    s_state = gps::State::kChecksum2;
                } else {
                    s_state = gps::State::kStartDelimiter;
                }
            } break;
            case gps::State::kChecksum2: {
                const auto kNibble = kByte > '9' ? static_cast<uint8_t>(kByte - 'A' + 10) : static_cast<uint8_t>(kByte - '0');
                if (kNibble == (s_checksum & 0xF)) {
                    s_state = gps::State::kCr;
                } else {
                    s_state = gps::State::kStartDelimiter;
                }
            } break;
            case gps::State::kCr:
                if (kByte == '\r') {
                    s_state = gps::State::kLf;
                } else {
                    s_state = gps::State::kStartDelimiter;
                }
                break;
            case gps::State::kLf:
                if (kByte == '\n') {
                    s_ring_buffer[s_ring_buffer_index_head][s_data_index] = '\n';
                    s_ring_buffer_index_head = (s_ring_buffer_index_head + 1) & gps::kRingBufferIndexMask;
                }
                s_state = gps::State::kStartDelimiter;
                break;
            default:
                assert(0);
                __builtin_unreachable();
                break;
        }

        if (s_state != gps::State::kStartDelimiter) {
            s_ring_buffer[s_ring_buffer_index_head][s_data_index++] = kByte;
            if (s_data_index == gps::nmea::kMaxSentenceLength) {
                s_state = gps::State::kStartDelimiter;
            }
        }
    }

    if (s_ring_buffer_index_head == s_ring_buffer_index_tail) {
        return nullptr;
    } else {
        const char* p = reinterpret_cast<const char*>(&s_ring_buffer[s_ring_buffer_index_tail][0]);
        s_ring_buffer_index_tail = (s_ring_buffer_index_tail + 1) & gps::kRingBufferIndexMask;
        return p;
    }
}
