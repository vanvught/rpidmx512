/**
 * @file h3_gpio.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

void h3_gpio_fsel(uint8_t gpio, gpio_fsel_t fsel) {
	const uint32_t number = H3_GPIO_TO_NUMBER(gpio);
	const uint32_t reg = number / 8;
	const uint32_t shift = (number & 0x7) * 4;
	const uint32_t base_pointer = (uint32_t) (H3_PIO_BASE + (H3_GPIO_TO_PORT(gpio) * 0x24));
	H3_PIO_TypeDef *pio = (H3_PIO_TypeDef *) base_pointer;
	uint32_t value;

#ifndef NDEBUG
	printf("%s gpio=%d, fsel=%d : port=%d[%c], number=%d, reg=%d, shift=%d, pio=%p\n", __FUNCTION__, gpio, fsel, H3_GPIO_TO_PORT(gpio), 'A' + H3_GPIO_TO_PORT(gpio), number, reg, shift, (void *) pio);
#endif

	switch (reg) {
		case 0:
			value = pio->CFG0;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			pio->CFG0 = value;
			break;
		case 1:
			value = pio->CFG1;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			pio->CFG1 = value;
			break;
		case 2:
			value = pio->CFG2;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			pio->CFG2 = value;
			break;
		default:
			break;
	}
}

 void h3_gpio_a_fsel(_gpio_pin pin_pa, gpio_fsel_t fsel) {
	const uint32_t reg = pin_pa / 8;
	const uint32_t shift = (pin_pa & 0x7) * 4;
	uint32_t value;

#ifndef NDEBUG
	printf("%s pin_pa=%d, fsel=%d, reg=%d, shift=%d\n", __FUNCTION__, pin_pa, fsel, reg, shift);
#endif

	switch (reg) {
		case 0:
			value = H3_PIO_PORTA->CFG0;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			H3_PIO_PORTA->CFG0 = value;
			break;
		case 1:
			value = H3_PIO_PORTA->CFG1;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			H3_PIO_PORTA->CFG1 = value;
			break;
		case 2:
			value = H3_PIO_PORTA->CFG2;
			value &= ~(GPIO_SELECT_MASK << shift);
			value |= (fsel << shift);
			H3_PIO_PORTA->CFG2 = value;
			break;
		default:
			break;
	}
 }

void h3_gpio_a_set_pud(_gpio_pin pin_pa, gpio_pull_t pud) {
	const uint32_t reg = pin_pa / 16;
	const uint32_t shift = (pin_pa & 0xf) * 2;
	uint32_t value;

#ifndef NDEBUG
	printf("%s pin_pa=%d, pud=%d\n", __FUNCTION__, pin_pa, pud);
	printf("\treg=%d, shift=%d\n", reg, shift);
#endif

	switch (reg) {
	case 0:
		value = H3_PIO_PORTA->PUL0;
		value &= ~(GPIO_PULL_MASK << shift);
		value |= (pud << shift);
		H3_PIO_PORTA->PUL0 = value;
		break;
	case 1:
		value = H3_PIO_PORTA->PUL1;
		value &= ~(GPIO_PULL_MASK << shift);
		value |= (pud << shift);
		H3_PIO_PORTA->PUL1 = value;
		break;
	default:
		break;
	}
}

