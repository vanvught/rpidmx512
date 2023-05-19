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

#include "emac/net_link_check.h"

#include "gd32.h"
#include "enet_config.h"

#include "debug.h"

namespace net {
#if (PHY_TYPE == RTL8201F)
	void phy_write_paged(uint16_t phy_page, uint16_t phy_reg, uint16_t phy_value, uint16_t mask = 0x0);
#endif

static void link_pin_enable() {
	uint16_t phy_value;
#if (PHY_TYPE == LAN8700)

#elif (PHY_TYPE == DP83848)
	phy_value = PHY_INT_AND_OUTPUT_ENABLE;
	enet_phy_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_MICR, &phy_value);

	enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_MICR, &phy_value);

	if (PHY_INT_AND_OUTPUT_ENABLE != phy_value) {
		DEBUG_PUTS("PHY_INT_AND_OUTPUT_ENABLE != phy_value");
	}

	phy_value = PHY_LINK_INT_ENABLE;
	enet_phy_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_MISR, &phy_value);
#elif (PHY_TYPE == RTL8201F)
	phy_write_paged(0x07, PHY_REG_IER, PHY_REG_IER_INT_ENABLE, PHY_REG_IER_INT_ENABLE);
	// Clear interrupt
	enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_ISR, &phy_value);
#endif
}

static void link_gpio_init() {
	rcu_periph_clock_enable(LINK_CHECK_GPIO_CLK);
	LINK_CHECK_GPIO_CONFIG;
}

static void link_pin_recovery() {
    uint16_t phy_value;
#if (PHY_TYPE == LAN8700)

#elif (PHY_TYPE == DP83848)
    enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_MISR, &phy_value);
#elif (PHY_TYPE == RTL8201F)
    enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_ISR, &phy_value);
#endif
    enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);
}

static void link_exti_init() {
    rcu_periph_clock_enable(LINK_CHECK_EXTI_CLK);

 	NVIC_SetPriority(LINK_CHECK_EXTI_IRQn, 7);
	NVIC_EnableIRQ(LINK_CHECK_EXTI_IRQn);

    LINK_CHECK_EXTI_SOURCE_CONFIG(LINK_CHECK_EXTI_PORT_SOURCE, LINK_CHECK_EXTI_PIN_SOURCE);

    exti_init(LINK_CHECK_EXTI_LINE, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(LINK_CHECK_EXTI_LINE);
}

void link_interrupt_init() {
    link_pin_enable();
    link_pin_recovery();
    link_gpio_init();
    link_exti_init();
}

void link_pin_poll_init() {
    link_pin_enable();
    link_pin_recovery();
    link_gpio_init();
}

void link_pin_poll() {
	if (RESET == gpio_input_bit_get(LINK_CHECK_GPIO_PORT, LINK_CHECK_GPIO_PIN)) {
		link_pin_recovery();
		link_handle_change(link_register_read());
	}
}

net::Link link_register_read() {
	uint16_t phy_value;

	enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);

	if (PHY_LINKED_STATUS == (phy_value & PHY_LINKED_STATUS)) {
		return net::Link::STATE_UP;
	}

	return net::Link::STATE_DOWN;
}
}  // namespace net

#if defined (ENET_LINK_CHECK_USE_INT)
extern "C" {
void LINK_CHECK_IRQ_HANDLE(void) {
	if (RESET != exti_interrupt_flag_get(LINK_CHECK_EXTI_LINE)) {
		exti_interrupt_flag_clear(LINK_CHECK_EXTI_LINE);
		net::link_pin_recovery();
		net::link_handle_change(net::link_register_read());
	}
}
}
#endif
