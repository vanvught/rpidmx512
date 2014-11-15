/**
 * @file bcm2835_gpio.c
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

#include "bcm2835.h"
#include "bcm2835_gpio.h"

/**
 *
 * @param pin
 * @return
 */
uint8_t bcm2835_gpio_lev(const uint8_t pin) {
	uint32_t value = BCM2835_GPIO ->GPLEV0;
	return (value & (1 << pin)) ? HIGH : LOW;
}

/**
 *
 * @param pud
 */
void bcm2835_gpio_pud(const uint8_t pud) {
	BCM2835_GPIO ->GPPUD = pud;
}

/**
 *
 * @param pin
 * @param on
 */
void bcm2835_gpio_pudclk(const uint8_t pin, const uint8_t on) {
	BCM2835_GPIO ->GPPUDCLK0 = (on ? 1 : 0) << pin;
}

/**
 *
 * @param pin
 * @param pud
 */
void bcm2835_gpio_set_pud(const uint8_t pin, const uint8_t pud) {
	bcm2835_gpio_pud(pud);
	udelay(10);
	bcm2835_gpio_pudclk(pin, 1);
	udelay(10);
	bcm2835_gpio_pud(BCM2835_GPIO_PUD_OFF);
	bcm2835_gpio_pudclk(pin, 0);
}
