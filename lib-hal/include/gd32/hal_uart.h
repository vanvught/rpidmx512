/**
 * @file hal_uart.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_HAL_UART_H_
#define GD32_HAL_UART_H_

#include "gd32_uart.h"

namespace hal {
namespace uart {
static constexpr auto BITS_8 = GD32_UART_BITS_8;
static constexpr auto BITS_9 = GD32_UART_BITS_9;

static constexpr auto PARITY_NONE = GD32_UART_PARITY_NONE;
static constexpr auto PARITY_ODD = GD32_UART_PARITY_ODD;
static constexpr auto PARITY_EVEN = GD32_UART_PARITY_EVEN;

static constexpr auto STOP_1BIT = GD32_UART_STOP_1BIT;
static constexpr auto STOP_2BITS = GD32_UART_STOP_2BITS;
}  // namespace uart
}  // namespace hal

#endif /* GD32_HAL_UART_H_ */
