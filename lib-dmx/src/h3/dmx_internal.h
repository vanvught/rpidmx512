/**
 * @file dmx_internal.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef H3_DMX_INTERNAL_H_
#define H3_DMX_INTERNAL_H_

#include <cassert>

#include "h3.h"

#if defined(ORANGE_PI)
/*
 * PORT
 * 0	UART1
 * 1	UART2
 */
inline H3_UART_TypeDef * _port_to_uart(const uint32_t nPortIndex) {
	switch (nPortIndex) {
	case 0:
		return H3_UART1;
		break;
	case 1:
		return H3_UART2;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return nullptr;
}
#else
/*
 * PORT
 * 0	UART1
 * 1	UART2
 * 2	UART3
 * 3	UART0
 */
inline H3_UART_TypeDef * _port_to_uart(const uint32_t nPortIndex) {
	switch (nPortIndex) {
	case 0:
		return H3_UART1;
		break;
	case 1:
		return H3_UART2;
		break;
	case 2:
		return H3_UART3;
		break;
	case 3:
		return H3_UART0;
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return nullptr;
}
#endif

#endif /* H3_DMX_INTERNAL_H_ */
