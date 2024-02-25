/**
 * @file platform_gpio.h
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

#ifndef PLATFORM_GPIO_H_
#define PLATFORM_GPIO_H_

#if defined(__linux__) || defined (__APPLE__)
	inline void platform_gpio_init() {}

	inline bool platform_is_pps() {
		return false;
	}
#else
# if defined (H3)
#  include "h3_gpio.h"
#  include "h3_board.h"
	inline void platform_gpio_init() {
		h3_gpio_fsel(GPIO_EXT_18, GPIO_FSEL_EINT);

		H3_PIO_PA_INT->CFG2 = (GPIO_INT_CFG_POS_EDGE << 8);
		H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_18);
		H3_PIO_PA_INT->STA = (1 << GPIO_EXT_18);
		H3_PIO_PA_INT->DEB = 1;
	}

	inline bool platform_is_pps() {
		const auto isPPS = ((H3_PIO_PA_INT->STA & (1 << GPIO_EXT_18)) == (1 << GPIO_EXT_18));
		if (!isPPS) {
			return false;
		}
		H3_PIO_PA_INT->STA = (1 << GPIO_EXT_18);
		return true;
	}
# elif defined (GD32)
#  include "gd32_gpio.h"
#  include "gd32_board.h"
	/**
	 * https://www.gd32-dmx.org/dev-board.html
	 * GPIO_EXT_18 = PA13
	 */
	static_assert(GD32_GPIO_TO_NUMBER(GPIO_EXT_18) == 13, "GPIO PIN is not 13");
	static_assert(GD32_GPIO_TO_PORT(GPIO_EXT_18) == GD32_GPIO_PORTA, "GPIO PORT is not A");

	inline void platform_gpio_init() {
		rcu_periph_clock_enable(RCU_GPIOA);
		gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);
		/* connect key EXTI line to key GPIO pin */
		syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN13);
		/* configure key EXTI line */
		exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_RISING);
		exti_interrupt_flag_clear(EXTI_13);
	}

	inline bool platform_is_pps() {
		const uint32_t flag_left = EXTI_PD & (uint32_t) EXTI_13;
		const uint32_t flag_right = EXTI_INTEN & (uint32_t) EXTI_13;

		if ((RESET != flag_left) && (RESET != flag_right)) {
			// exti_interrupt_flag_clear(EXTI_13);
			EXTI_PD = (uint32_t) EXTI_13;
			return true;
		}

		return false;
	}
# else
#  error Platform is not supported
# endif
#endif

#endif /* PLATFORM_GPIO_H_ */
