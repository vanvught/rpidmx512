/**
 * @file dmx_internal.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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

#ifndef GD32_DMX_INTERNAL_H_
#define GD32_DMX_INTERNAL_H_

#include <cstdint>
#include <cassert>

#include "gd32.h"
#include "gd32_gpio.h"
#include "gd32/dmx_config.h"

#ifdef ALIGNED
#undef ALIGNED
#endif
#define ALIGNED __attribute__ ((aligned (4)))

/**
 * Needed for older GD32F firmware
 */
#if !defined(USART_TRANSMIT_DMA_ENABLE)
# define USART_TRANSMIT_DMA_ENABLE			USART_DENT_ENABLE
#endif

/**
 * https://www.gd32-dmx.org/memory.html
 */
#if defined (GD32F20X) || defined (GD32F4XX) || defined (GD32H7XX)
# define SECTION_DMA_BUFFER					__attribute__ ((section (".dmx")))
#else
# define SECTION_DMA_BUFFER
#endif

#if defined (GD32F4XX) || defined (GD32H7XX)
# define DMA_INTERRUPT_ENABLE				(DMA_CHXCTL_FTFIE)
# define DMA_INTERRUPT_DISABLE				(DMA_CHXCTL_FTFIE | DMA_CHXCTL_HTFIE | DMA_CHXFCTL_FEEIE)
# define DMA_INTERRUPT_FLAG_GET				(DMA_INT_FLAG_FTF)
# define DMA_INTERRUPT_FLAG_CLEAR			(DMA_INT_FLAG_FTF | DMA_INT_FLAG_TAE)
#else
# define DMA_INTERRUPT_ENABLE				(DMA_INT_FTF)
# define DMA_INTERRUPT_DISABLE				(DMA_INT_FTF | DMA_INT_HTF | DMA_INT_ERR)
# define DMA_INTERRUPT_FLAG_GET				(DMA_INT_FLAG_FTF)
# define DMA_INTERRUPT_FLAG_CLEAR			(DMA_INT_FLAG_FTF | DMA_INT_FLAG_G)
#endif

inline uint32_t dmx_port_to_uart(const uint32_t nPort) {
	switch (nPort) {
#if defined (DMX_USE_USART0)
	case dmx::config::USART0_PORT:
		return USART0;
		break;
#endif
#if defined (DMX_USE_USART1)
	case dmx::config::USART1_PORT:
		return USART1;
		break;
#endif
#if defined (DMX_USE_USART2)
	case dmx::config::USART2_PORT:
		return USART2;
		break;
#endif
#if defined (DMX_USE_UART3)
	case dmx::config::UART3_PORT:
		return UART3;
		break;
#endif
#if defined (DMX_USE_UART4)
	case dmx::config::UART4_PORT:
		return UART4;
		break;
#endif
#if defined (DMX_USE_USART5)
	case dmx::config::USART5_PORT:
		return USART5;
		break;
#endif
#if defined (DMX_USE_UART6)
	case dmx::config::UART6_PORT:
		return UART6;
		break;
#endif
#if defined (DMX_USE_UART7)
	case dmx::config::UART7_PORT:
		return UART7;
		break;
#endif
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	assert(0);
	__builtin_unreachable();
	return 0;
}

#if defined (GD32F4XX) || defined (GD32H7XX)
constexpr uint32_t get_usart_af(const uint32_t usart_periph) {
	switch (usart_periph) {
#if defined(DMX_USE_USART0)
    case USART0: return USART0_GPIO_AFx;
    #endif
#if defined(DMX_USE_USART1)
	case USART1:
		return USART1_GPIO_AFx;
#endif
#if defined(DMX_USE_USART2)
	case USART2:
		return USART2_GPIO_AFx;
#endif
#if defined(DMX_USE_UART3)
    case UART3: return UART3_GPIO_AFx;
    #endif
#if defined(DMX_USE_UART4)
	case UART4:
		return UART4_GPIO_AFx;
#endif
#if defined(DMX_USE_USART5)
	case USART5:
		return USART5_GPIO_AFx;
#endif
#if defined(DMX_USE_UART6)
	case UART6:
		return UART6_GPIO_AFx;
#endif
#if defined(DMX_USE_UART7)
	case UART7:
		return UART7_GPIO_AFx;
#endif
	default:
		__builtin_unreachable();
		return 0;
	}

	__builtin_unreachable();
	return 0;
}

template<uint32_t gpio_periph, uint32_t pin>
inline void gd32_gpio_mode_output() {
	gd32_gpio_mode_set<gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin>();
}

template<uint32_t gpio_periph, uint32_t pin, uint32_t usart_periph>
inline void gd32_gpio_mode_af() {
	gd32_gpio_mode_set<gpio_periph, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pin>();

	constexpr uint32_t af = get_usart_af(usart_periph);
	static_assert(af != 0, "Invalid USART peripheral");

	gd32_gpio_af_set<gpio_periph, af, pin>();
}
#else
template<uint32_t gpio_periph, uint32_t pin>
inline void gd32_gpio_mode_output() {
	gd32_gpio_init<gpio_periph, GPIO_MODE_OUT_PP, pin>();
}

template<uint32_t gpio_periph, uint32_t pin, uint32_t usart_periph>
inline void gd32_gpio_mode_af() {
	gd32_gpio_init<gpio_periph, GPIO_MODE_AF_PP, pin>();
}
#endif

#endif /* GD32_DMX_INTERNAL_H_ */
