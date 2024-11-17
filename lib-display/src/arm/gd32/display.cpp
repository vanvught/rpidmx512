/**
 * @file display.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_DISPLAY)
# undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "display.h"

#include "gd32.h"
#include "gd32_gpio.h"

#include "debug.h"

#if defined (DISPLAYTIMEOUT_CONFIG_IRQ) && !defined (CONFIG_USE_EXTI10_15_IRQHandler)
extern "C" {
void DISPLAYTIMEOUT_IRQ_HANDLE() {
	if (RESET != exti_interrupt_flag_get(DISPLAYTIMEOUT_EXTI_LINE)) {
		exti_interrupt_flag_clear(DISPLAYTIMEOUT_EXTI_LINE);
		Display::Get()->SetSleep(false);
		DEBUG_PUTS("Key pressed.");
	}
}
}
#endif

namespace display::timeout {
void irq_init() {
#if defined (DISPLAYTIMEOUT_CONFIG_IRQ) && !defined (CONFIG_USE_EXTI10_15_IRQHandler)
	DEBUG_ENTRY

	rcu_periph_clock_enable(DISPLAYTIMEOUT_GPIO_CLK);
	DISPLAYTIMEOUT_GPIO_CONFIG;

    rcu_periph_clock_enable(DISPLAYTIMEOUT_EXTI_CLK);

 	NVIC_SetPriority(DISPLAYTIMEOUT_EXTI_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); // Lowest priority
	NVIC_EnableIRQ(DISPLAYTIMEOUT_EXTI_IRQn);

    DISPLAYTIMEOUT_EXTI_SOURCE_CONFIG(DISPLAYTIMEOUT_EXTI_PORT_SOURCE, DISPLAYTIMEOUT_EXTI_PIN_SOURCE);

    exti_init(DISPLAYTIMEOUT_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(DISPLAYTIMEOUT_EXTI_LINE);

	DEBUG_EXIT
#endif
}
}  // namespace display::timeout
