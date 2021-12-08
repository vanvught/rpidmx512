/**
 * @file platform_gpio.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (BARE_METAL)
# if defined (H3)
#  include "h3_gpio.h"
#  include "h3_board.h"
	void platform_gpio_init() {
		h3_gpio_fsel(GPIO_EXT_18, GPIO_FSEL_EINT);

		H3_PIO_PA_INT->CFG2 = (GPIO_INT_CFG_POS_EDGE << 8);
		H3_PIO_PA_INT->CTL |= (1 << GPIO_EXT_18);
		H3_PIO_PA_INT->STA = (1 << GPIO_EXT_18);
		H3_PIO_PA_INT->DEB = 1;
	}

	bool platform_is_pps() {
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
# else
#  error Platform is not supported
# endif
#else
	void platform_gpio_init(void) {}

	bool platform_is_pps(void) {
		return false;
	}
#endif

#endif /* PLATFORM_GPIO_H_ */
