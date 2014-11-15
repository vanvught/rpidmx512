/**
 * @file bcm2835_gpio.h
 *
 */
/* Copyright (C) 2014 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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

#ifndef BCM2835_GPIO_H_
#define BCM2835_GPIO_H_

#include <stdint.h>
#include "bcm2835.h"

typedef enum {
	BCM2835_GPIO_FSEL_INPT = 0b000,	///< Input
	BCM2835_GPIO_FSEL_OUTP = 0b001,	///< Output
	BCM2835_GPIO_FSEL_ALT0 = 0b100,	///< Alternate function 0
	BCM2835_GPIO_FSEL_ALT1 = 0b101,	///< Alternate function 1
	BCM2835_GPIO_FSEL_ALT2 = 0b110,	///< Alternate function 2
	BCM2835_GPIO_FSEL_ALT3 = 0b111,	///< Alternate function 3
	BCM2835_GPIO_FSEL_ALT4 = 0b011,	///< Alternate function 4
	BCM2835_GPIO_FSEL_ALT5 = 0b010,	///< Alternate function 5
	BCM2835_GPIO_FSEL_MASK = 0b111	///< Function select bits mask
} bcm2835FunctionSelect;

typedef enum {
	BCM2835_GPIO_PUD_OFF 	= 0b00,	///< Off ? disable pull-up/down
	BCM2835_GPIO_PUD_DOWN 	= 0b01,	///< Enable Pull Down control
	BCM2835_GPIO_PUD_UP 	= 0b10	///< Enable Pull Up control
} bcm2835PUDControl;

/**
 *
 * @param pin
 */
inline static void bcm2835_gpio_set(const uint8_t pin) {
	BCM2835_GPIO ->GPSET0 = 1 << pin;
}

/**
 *
 * @param pin
 */
inline static void bcm2835_gpio_clr(const uint8_t pin) {
	BCM2835_GPIO ->GPCLR0 = 1 << pin;
}

/**
 *
 * @param pin
 * @param on
 */
inline static void bcm2835_gpio_write(const uint8_t pin, const uint8_t on) {
	if (on)
		bcm2835_gpio_set(pin);
	else
		bcm2835_gpio_clr(pin);
}

#define BCM2835_PERI_SET_BITS(a, v, m)		a = ((a) & ~(m)) | ((v) & (m));

extern void  bcm2835_gpio_set_pud(const uint8_t, const uint8_t);
extern void bcm2835_gpio_fsel(const uint8_t, const uint8_t);
extern uint8_t bcm2835_gpio_lev(const uint8_t pin);

#endif /* BCM2835_GPIO_H_ */
