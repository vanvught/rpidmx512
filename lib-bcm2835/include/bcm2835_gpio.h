/**
 * @file bcm2835_gpio.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

/// Function select modes for \ref bcm2835_gpio_fsel
typedef enum {
	BCM2835_GPIO_FSEL_INPT = 0x00,	///< 0b000,	Input
	BCM2835_GPIO_FSEL_OUTP = 0x01,	///< 0b001,	Output
	BCM2835_GPIO_FSEL_ALT0 = 0x04,	///< 0b100,	Alternate function 0
	BCM2835_GPIO_FSEL_ALT1 = 0x05,	///< 0b101,	Alternate function 1
	BCM2835_GPIO_FSEL_ALT2 = 0x06,	///< 0b110,	Alternate function 2
	BCM2835_GPIO_FSEL_ALT3 = 0x07,	///< 0b111,	Alternate function 3
	BCM2835_GPIO_FSEL_ALT4 = 0x03,	///< 0b011,	Alternate function 4
	BCM2835_GPIO_FSEL_ALT5 = 0x02,	///< 0b010,	Alternate function 5
	BCM2835_GPIO_FSEL_MASK = 0x07	///< 0b111, Function select bits mask
} bcm2835FunctionSelect;

/// Pullup/Pulldown defines for \ref bcm2835_gpio_set_pud
typedef enum {
	BCM2835_GPIO_PUD_OFF 	= 0x00,	///< 0b00, Off ? disable pull-up/down
	BCM2835_GPIO_PUD_DOWN 	= 0x01,	///< 0b01, Enable Pull Down control
	BCM2835_GPIO_PUD_UP 	= 0x02	///< 0b10, Enable Pull Up control
} bcm2835PUDControl;


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup GPIO
 *
 * Sets the specified pin output to HIGH
 *
 * @param pin GPIO number.
 */
/*@unused@*/inline static void bcm2835_gpio_set(const uint8_t pin) {
	BCM2835_GPIO->GPSET0 = (uint32_t) (1 << pin);
}

/**
 * @ingroup GPIO
 *
 * Sets the specified pin output to LOW
 *
 * @param pin GPIO number.
 */
/*@unused@*/inline static void bcm2835_gpio_clr(const uint8_t pin) {
	BCM2835_GPIO ->GPCLR0 = (uint32_t) (1 << pin);
}

/**
 * @ingroup GPIO
 *
 * Sets the output state of the specified pin.
 *
 * @param pin GPIO number
 * @param on \ref HIGH sets the output to HIGH and \ref LOW to LOW.
 */
/*@unused@*/inline static void bcm2835_gpio_write(const uint8_t pin, const uint8_t on) {
	if (on != 0) {
		bcm2835_gpio_set(pin);
	} else {
		bcm2835_gpio_clr(pin);
	}
}

/**
 * @ingroup GPIO
 *
 * @param pin
 * @return
 */
/*@unused@*/inline static uint8_t bcm2835_gpio_lev(const uint8_t pin) {
	const uint32_t value = BCM2835_GPIO->GPLEV0; // TODO BUG pin > 32
	return (value & (1 << pin)) ? (uint8_t) HIGH : (uint8_t) LOW;
}


/**
 * @ingroup GPIO
 *
 * See if an event detection bit is set
 *
 * @param pin
 * @return
 */
/*@unused@*/inline static uint8_t bcm2835_gpio_eds(const uint8_t pin) {
	const uint32_t value = BCM2835_GPIO->GPEDS0; // TODO BUG pin > 32
	return (value & (1 << pin)) ? (uint8_t) HIGH : (uint8_t) LOW;
}

/**
 * @ingroup GPIO
 *
 * Write a 1 to clear the bit in EDS
 *
 * @param pin
 */
/*@unused@*/inline static void bcm2835_gpio_set_eds(const uint8_t pin) {
	BCM2835_GPIO->GPEDS0 = (1 << pin); // TODO BUG pin > 32
}


#define BCM2835_PERI_SET_BITS(a, v, m)		a = ((a) & ~(m)) | ((v) & (m));

/**
 * @ingroup GPIO
 *
 * Defined in file bcm2835_gpio_fsel.S
 *
 * Sets the Function Select register for the given pin, which configures
 * the pin as Input, Output or one of the 6 alternate functions.
 *
 * @param pin GPIO number.
 * @param mode Mode to set the pin to, one of BCM2835_GPIO_FSEL_* from \ref bcm2835FunctionSelect
 */
extern void bcm2835_gpio_fsel(const uint8_t, const uint8_t);

extern void bcm2835_gpio_set_pud(const uint8_t, const uint8_t);

extern void bcm2835_gpio_len(const uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* BCM2835_GPIO_H_ */
