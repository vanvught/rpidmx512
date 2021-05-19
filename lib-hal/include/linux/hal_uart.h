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

#ifndef LINUX_HAL_UART_H_
#define LINUX_HAL_UART_H_

#define UART_BITS_5			5
#define UART_BITS_6			6
#define UART_BITS_7			7
#define UART_BITS_8			8

#define UART_PARITY_NONE	0
#define UART_PARITY_ODD		1
#define UART_PARITY_EVEN	2

#define UART_STOP_1BIT		1
#define UART_STOP_2BITS		2

#define FUNC_PREFIX(x) x

#ifdef __cplusplus
extern "C" {
#endif

inline static void uart_begin(__attribute__((unused)) uint32_t uart, __attribute__((unused)) uint32_t baudrate, __attribute__((unused)) uint32_t bits, __attribute__((unused)) uint32_t parity, __attribute__((unused)) uint32_t stop_bits) {}
inline static void uart_transmit(__attribute__((unused)) uint32_t uart, __attribute__((unused)) const uint8_t *data, __attribute__((unused)) uint32_t length) {}

#ifdef __cplusplus
}
#endif

#endif /* LINUX_HAL_UART_H_ */