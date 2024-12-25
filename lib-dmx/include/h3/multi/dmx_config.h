/**
 * @file dmx_config.h
 *
 */
/* Copyright (C) 2023-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_MULTI_DMX_CONFIG_H_
#define H3_MULTI_DMX_CONFIG_H_

#include "h3_board.h"

#if defined(ORANGE_PI_ONE)
# define DMX_MAX_PORTS  4
#else
# define DMX_MAX_PORTS	2
#endif



namespace dmx::config::max {
	static const uint32_t PORTS = DMX_MAX_PORTS;
} // namespace dmx::config::max




namespace dmx::buffer {
static constexpr auto SIZE = 516;
static constexpr auto INDEX_ENTRIES = (1U << 1);
static constexpr auto INDEX_MASK = (INDEX_ENTRIES - 1);
} // namespace dmx::buffer


#if defined(ORANGE_PI_ONE)
# define DMX_MAX_UARTS	4
# define GPIO_DMX_DATA_DIRECTION_OUT_A 	GPIO_EXT_32	///< UART1
# define GPIO_DMX_DATA_DIRECTION_OUT_B 	GPIO_EXT_22	///< UART2
# define GPIO_DMX_DATA_DIRECTION_OUT_C 	GPIO_EXT_12	///< UART3
# define GPIO_DMX_DATA_DIRECTION_OUT_D 	GPIO_EXT_31	///< UART0
#else
# define DMX_MAX_UARTS	2
# define GPIO_DMX_DATA_DIRECTION_OUT_B	GPIO_EXT_22	///< UART2
# define GPIO_DMX_DATA_DIRECTION_OUT_C	GPIO_EXT_12	///< UART1
#endif

#endif /* H3_MULTI_DMX_CONFIG_H_ */
