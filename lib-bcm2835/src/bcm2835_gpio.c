/**
 * @file bcm2835_gpio.c
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

#include <stdint.h>

#include "bcm2835.h"
#include "bcm2835_gpio.h"


/**
 * @ingroup GPIO
 *
 * Sets the Pull-up/down register for the given pin. This is
 * used with \ref bcm2835_gpio_pudclk to set the  Pull-up/down resistor for the given pin.
 * However, it is usually more convenient to use \ref bcm2835_gpio_set_pud.
 *
 * @param pud The desired Pull-up/down mode. One of BCM2835_GPIO_PUD_* from \ref bcm2835PUDControl
 */
static void bcm2835_gpio_pud(const uint8_t pud) {
	BCM2835_GPIO ->GPPUD = pud;
}

/**
 * @ingroup GPIO
 *
 * @param pin
 * @param on
 */
static void bcm2835_gpio_pudclk(const uint8_t pin, const uint8_t on) {
	BCM2835_GPIO ->GPPUDCLK0 = (uint32_t)(((on != 0) ? 1 : 0) << pin);
}

/**
 * @ingroup GPIO
 *
 * Sets the Pull-up/down mode for the specified pin.
 *
 * @param pin GPIO number.
 * @param pud The desired Pull-up/down mode. One of BCM2835_GPIO_PUD_* from \ref bcm2835PUDControl
 */
void bcm2835_gpio_set_pud(const uint8_t pin, const uint8_t pud) {
	bcm2835_gpio_pud(pud);
	udelay(10);
	bcm2835_gpio_pudclk(pin, 1);
	udelay(10);
	bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_pudclk(pin, 0);
}


/**
 * @ingroup GPIO
 *
 * Enable Low Detect Enable for the specified pin.
 * When a LOW level is detected on the pin, sets the appropriate pin in Event Detect Status.
 *
 * @param pin GPIO number
 */
void bcm2835_gpio_len(const uint8_t pin) {
	const uint32_t value = 1 << pin;
	BCM2835_PERI_SET_BITS(BCM2835_GPIO->GPLEN0, value, value);
}
