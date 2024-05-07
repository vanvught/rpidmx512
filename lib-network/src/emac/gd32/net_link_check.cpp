/**
 * net_link_check.cpp
 *
 */
/* Copyright (C) 2022-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "emac/phy.h"
#include "emac/net_link_check.h"

#include "gd32.h"
#include "enet_config.h"

#include "debug.h"

namespace net {
#if defined (ENET_LINK_CHECK_USE_INT) || defined (ENET_LINK_CHECK_USE_PIN_POLL)
void link_gpio_init() {
	rcu_periph_clock_enable(LINK_CHECK_GPIO_CLK);
	LINK_CHECK_GPIO_CONFIG;
}
#endif

#if defined (ENET_LINK_CHECK_USE_INT)
void link_exti_init() {
    rcu_periph_clock_enable(LINK_CHECK_EXTI_CLK);

 	NVIC_SetPriority(LINK_CHECK_EXTI_IRQn, 7);
	NVIC_EnableIRQ(LINK_CHECK_EXTI_IRQn);

    LINK_CHECK_EXTI_SOURCE_CONFIG(LINK_CHECK_EXTI_PORT_SOURCE, LINK_CHECK_EXTI_PIN_SOURCE);

    exti_init(LINK_CHECK_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(LINK_CHECK_EXTI_LINE);
}
#endif

#if defined (ENET_LINK_CHECK_USE_PIN_POLL)
void link_pin_poll() {
	if (RESET == gpio_input_bit_get(LINK_CHECK_GPIO_PORT, LINK_CHECK_GPIO_PIN)) {
		link_pin_recovery();
		link_handle_change(link_status_read());
	}
}
#endif

}  // namespace net

#if defined (ENET_LINK_CHECK_USE_INT)
extern "C" {
void LINK_CHECK_IRQ_HANDLE(void) {
	if (RESET != exti_interrupt_flag_get(LINK_CHECK_EXTI_LINE)) {
		exti_interrupt_flag_clear(LINK_CHECK_EXTI_LINE);
		net::link_pin_recovery();
		net::link_handle_change(net::link_status_read());
	}
}
}
#endif
