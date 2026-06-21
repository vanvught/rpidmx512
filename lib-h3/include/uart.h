/**
 * @file uart.h
 * @brief H2+/H3
 */
/* Copyright (C) 2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef UART_H_
#define UART_H_

#include "h3_uart.h"

namespace uart {
inline constexpr auto BITS_5 = H3_UART_BITS_5;
inline constexpr auto BITS_6 = H3_UART_BITS_6;
inline constexpr auto BITS_7 = H3_UART_BITS_7;
inline constexpr auto BITS_8 = H3_UART_BITS_8;

inline constexpr auto PARITY_NONE = H3_UART_PARITY_NONE;
inline constexpr auto PARITY_ODD = H3_UART_PARITY_ODD;
inline constexpr auto PARITY_EVEN = H3_UART_PARITY_EVEN;

inline constexpr auto STOP_1BIT = H3_UART_STOP_1BIT;
inline constexpr auto STOP_2BITS = H3_UART_STOP_2BITS;

inline void Begin(uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits) {
    H3UartBegin(uart_base, baudrate, bits, parity, stop_bits);
}

inline void SetBaudrate(uint32_t uart_base, uint32_t baudrate) {
    H3UartSetBaudrate(uart_base, baudrate);
}

inline void Transmit(uint32_t uart_base, const uint8_t* data, uint32_t length) {
    H3UartTransmit(uart_base, data, length);
}

inline void TransmitString(uint32_t uart_base, const char* data) {
    H3UartTransmitString(uart_base, data);
}

inline uint32_t GetRxFifoLevel(uint32_t uart_base) {
    return H3UartGetRxFifoLevel(uart_base);
}

inline uint8_t GetRxData(uint32_t uart_base) {
    return H3UartGetRxData(uart_base);
}
} // namespace uart

#endif // UART_H_
