/**
 * @file gd32_platform_ltc.h
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_PLATFORM_LTC_H_
#define GD32_PLATFORM_LTC_H_

#include "gd32.h"

extern volatile uint32_t gv_ltc_nCurrentCAR;
extern uint32_t g_ltc_nCAR1;

#if defined DEBUG_LTC_TIMER3
# define DEBUG_TIMER3_RCU_GPIOx		RCU_GPIOA
# define DEBUG_TIMER3_GPIOx			GPIOA
# define DEBUG_TIMER3_GPIO_PINx		GPIO_PIN_11
#endif

#if defined DEBUG_LTC_TIMER10
# define DEBUG_TIMER10_RCU_GPIOx	RCU_GPIOA
# define DEBUG_TIMER10_GPIOx		GPIOA
# define DEBUG_TIMER10_GPIO_PINx	GPIO_PIN_13
#endif

#if defined DEBUG_LTC_TIMER11
# define DEBUG_TIMER11_RCU_GPIOx	RCU_GPIOA
# define DEBUG_TIMER11_GPIOx		GPIOA
# define DEBUG_TIMER11_GPIO_PINx	GPIO_PIN_14
#endif


namespace platform::ltc {
void timer3_config();
void timer10_config();
void timer11_config();
void timer11_set_type(const uint32_t nType);
void timer13_config();
} // namespace platform::ltc

#endif /* GD32_PLATFORM_LTC_H_ */
