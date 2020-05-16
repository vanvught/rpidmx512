/**
 * @file h3_gpio_int_cfg.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "h3_gpio.h"
#include "h3.h"

void h3_gpio_int_cfg(uint32_t gpio, gpio_int_cfg_t int_cfg) {
	const uint32_t number = H3_GPIO_TO_NUMBER(gpio);
	const uint32_t shift = (number & 0x7) * 4;
	uint32_t value;

	switch (H3_GPIO_TO_PORT(gpio)) {
	case H3_GPIO_PORTA:
		value = H3_PIO_PA_INT->CFG0;
		value &= ~((uint32_t) GPIO_INT_CFG_MASK << shift);
		value |= ((uint32_t) int_cfg << shift);
		H3_PIO_PA_INT->CFG0 = value;
		break;
	case H3_GPIO_PORTG:
		value = H3_PIO_PG_INT->CFG0;
		value &= ~((uint32_t) GPIO_INT_CFG_MASK << shift);
		value |= ((uint32_t) int_cfg << shift);
		H3_PIO_PG_INT->CFG0 = value;
		break;
	default:
		break;
	}

#ifndef NDEBUG
	printf("%s gpio=%d, int_cfg=%d : port=%d[%c], number=%d, shift=%d\n", __func__, gpio, int_cfg, H3_GPIO_TO_PORT(gpio), 'A' + H3_GPIO_TO_PORT(gpio), number, shift);
#endif
}
