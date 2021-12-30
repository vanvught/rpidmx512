/**
 * @file bitbanging595.h
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

#ifndef GD32_BITBANGING595_H_
#define GD32_BITBANGING595_H_

#include <cstdint>
#include <cassert>

#include "gd32.h"

#include "debug.h"

#if !defined(BITBANGING595_COUNT)
# define BITBANGING595_COUNT	1
#endif

namespace bitbanging595 {
#if (BITBANGING595_COUNT == 1)
static constexpr uint32_t MASK_HIGH = (1U << 7);
#elif (BITBANGING595_COUNT == 2)
static constexpr uint32_t MASK_HIGH = (1U << 15);
#elif (BITBANGING595_COUNT == 3)
static constexpr uint32_t MASK_HIGH = (1U << 23);
#else
static constexpr uint32_t MASK_HIGH = (1U << 31);
#endif
}  // namespace bitbanging595

// TODO Change LEDx with functional names
namespace bitbang595 {
enum class Led : uint32_t {
	LED1 = (1U << 0),
	LED2 = (1U << 1),
	LED3 = (1U << 2),
	LED4 = (1U << 3),
	LED5 = (1U << 4),
	LED6 = (1U << 5),
	LED7 = (1U << 6),
	LED8 = (1U << 7)
};
}  // namespace bitbang595

class BitBanging595 {
public:
	BitBanging595(void) {
		DEBUG_PRINTF("Mask=%x", bitbanging595::MASK_HIGH);

		assert(s_pThis == nullptr);
		s_pThis = this;

	    rcu_periph_clock_enable(LED595_DATA_RCU_GPIOx);
	    gpio_init(LED595_DATA_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED595_DATA_GPIO_PINx);

	    rcu_periph_clock_enable(LED595_CLK_RCU_GPIOx);
	    gpio_init(LED595_CLK_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED595_CLK_GPIO_PINx);
	    GPIO_BOP(LED595_CLK_GPIOx) = LED595_CLK_GPIO_PINx;

	    rcu_periph_clock_enable(LED595_LOAD_RCU_GPIOx);
	    gpio_init(LED595_LOAD_GPIOx, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED595_LOAD_GPIO_PINx);
	    GPIO_BOP(LED595_LOAD_GPIOx) = LED595_LOAD_GPIO_PINx;

	    ShiftOut();
	}

	// TODO bit-banging ?
	void SetOn(uint32_t nData) {
		s_nData |= nData;
	}

	// TODO bit-banging ?
	void SetOff(uint32_t nData) {
		s_nData &= ~nData;
	}

	void Run() {
		if (__builtin_expect ((s_nData != s_nDataPrevious), 0)) {
			s_nDataPrevious = s_nData;
			ShiftOut();
		}
	}

	static BitBanging595* Get() {
		return s_pThis;
	}

private:
	void ShiftOut() {
		GPIO_BC(LED595_LOAD_GPIOx) = LED595_LOAD_GPIO_PINx;

		for (uint32_t nMask = bitbanging595::MASK_HIGH; nMask != 0; nMask = static_cast<uint32_t>(nMask >> 1U)) {
			if (s_nData & nMask) {
				gpio_bit_set(LED595_DATA_GPIOx, LED595_DATA_GPIO_PINx);
			} else {
				gpio_bit_reset(LED595_DATA_GPIOx, LED595_DATA_GPIO_PINx);
			}

			__ISB();
			gpio_bit_reset(LED595_CLK_GPIOx, LED595_CLK_GPIO_PINx);
			__ISB();
			gpio_bit_set(LED595_CLK_GPIOx, LED595_CLK_GPIO_PINx);
		}

		GPIO_BOP(LED595_LOAD_GPIOx) = LED595_LOAD_GPIO_PINx;
	}

private:
	static uint32_t s_nData;
	static uint32_t s_nDataPrevious;
	static BitBanging595 *s_pThis;
};

#endif /* GD32_BITBANGING595_H_ */
