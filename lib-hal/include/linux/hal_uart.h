/**
 * @file hal_uart.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (RASPPI)
#else
# define FUNC_PREFIX(x)	x
#endif

#define EXT_UART_NUMBER		0
#define EXT_UART_BASE		0
#define EXT_MIDI_UART_BASE	0

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void uart_begin(const uint32_t uart_base, uint32_t baudrate, uint32_t bits, uint32_t parity, uint32_t stop_bits);
void uart_set_baudrate(const uint32_t uart_base, uint32_t baudrate);
void uart_transmit(const uint32_t uart_base, const uint8_t *data, uint32_t length);
void uart_transmit_string(const uint32_t uart_base, const char *data);

uint32_t uart_get_rx_fifo_level(const uint32_t uart_base);
uint8_t uart_get_rx_data(const uint32_t uart_base);
uint32_t uart_get_rx(const uint32_t uart_base, char *pData, uint32_t nLength);

#ifdef __cplusplus
}
#endif

#endif /* LINUX_HAL_H_ */
