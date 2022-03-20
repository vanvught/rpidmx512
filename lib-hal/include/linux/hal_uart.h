/**
 * @file hal_uart.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef LINUX_HAL_H_
#define LINUX_HAL_H_

namespace hal {
namespace uart {
static constexpr auto BITS_5 = 5;
static constexpr auto BITS_6 = 6;
static constexpr auto BITS_7 = 7;
static constexpr auto BITS_8 = 8;

static constexpr auto PARITY_NONE = 0;
static constexpr auto PARITY_ODD = 1;
static constexpr auto PARITY_EVEN = 2;

static constexpr auto STOP_1BIT = 1;
static constexpr auto STOP_2BITS = 2;
}  // namespace uart
}  // namespace hal

#define FUNC_PREFIX(x)	x

#define EXT_UART_NUMBER	0
#define EXT_UART_BASE	0

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

inline static void uart_begin(__attribute__((unused)) const uint32_t uart_base, __attribute__((unused)) uint32_t baudrate, __attribute__((unused)) uint32_t bits, __attribute__((unused)) uint32_t parity, __attribute__((unused)) uint32_t stop_bits) {}
inline static void uart_set_baudrate(__attribute__((unused)) const uint32_t uart_base, __attribute__((unused)) uint32_t baudrate) {}
inline static void uart_transmit(__attribute__((unused)) const uint32_t uart_base, __attribute__((unused)) const uint8_t *data, __attribute__((unused)) uint32_t length) {}
inline static void uart_transmit_string(__attribute__((unused)) const uint32_t uart_base, __attribute__((unused)) const char *data) {}

static inline uint32_t uart_get_rx_fifo_level(__attribute__((unused)) const uint32_t uart_base) {
	return 0;
}

static inline uint8_t uart_get_rx_data(__attribute__((unused)) const uint32_t uart_base) {
	return (uint8_t) ' ';
}

#ifdef __cplusplus
}
#endif

#endif /* LINUX_HAL_H_ */
