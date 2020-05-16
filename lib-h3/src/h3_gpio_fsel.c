/**
 * @file h3_gpio_fsel.c
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

void h3_gpio_fsel(uint32_t gpio, gpio_fsel_t fsel) {
	const uint32_t number = H3_GPIO_TO_NUMBER(gpio);
	const uint32_t reg = number / 8;
	const uint32_t shift = (number & 0x7) * 4;
	const uint32_t base_pointer = (uint32_t) (H3_PIO_BASE + (H3_GPIO_TO_PORT(gpio) * 0x24));
	H3_PIO_TypeDef *pio = (H3_PIO_TypeDef*) base_pointer;
	uint32_t value;

#ifndef NDEBUG
	printf("%s gpio=%d, fsel=%d : port=%d[%c], number=%d, reg=%d, shift=%d, pio=%p\n", __func__, gpio, fsel, H3_GPIO_TO_PORT(gpio), 'A' + H3_GPIO_TO_PORT(gpio), number, reg, shift, (void *) pio);
#endif

	switch (reg) {
	case 0:
		value = pio->CFG0;
		value &= ~((uint32_t) GPIO_SELECT_MASK << shift);
		value |= ((uint32_t) fsel << shift);
		pio->CFG0 = value;
		break;
	case 1:
		value = pio->CFG1;
		value &= ~((uint32_t) GPIO_SELECT_MASK << shift);
		value |= ((uint32_t) fsel << shift);
		pio->CFG1 = value;
		break;
	case 2:
		value = pio->CFG2;
		value &= ~((uint32_t) GPIO_SELECT_MASK << shift);
		value |= ((uint32_t) fsel << shift);
		pio->CFG2 = value;
		break;
	default:
		break;
	}
}
