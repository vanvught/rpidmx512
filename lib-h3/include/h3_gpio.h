	/**
 * @file h3_gpio.h
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

#ifndef H3_GPIO_H_
#define H3_GPIO_H_

#include "h3.h"
#include "h3_board.h"

enum H3_GPIO_LEVEL {
	LOW = 0,
	HIGH = 1
};

enum H3_GPIO_MASK {
	GPIO_SELECT_MASK = 0x7,
	GPIO_PULL_MASK = 0x3
};

typedef enum H3_GPIO_FSEL {
	GPIO_FSEL_INPUT = 0,
	GPIO_FSEL_OUTPUT = 1,
	GPIO_FSEL_DISABLE = 7
} gpio_fsel_t;

enum H3_PA_SELECT_SHIFT {
	PA0_SELECT_CFG0_SHIFT = 0,
	PA1_SELECT_CFG0_SHIFT = 4,
	PA2_SELECT_CFG0_SHIFT = 8,
	PA3_SELECT_CFG0_SHIFT = 12,
	PA4_SELECT_CFG0_SHIFT = 16,
	PA5_SELECT_CFG0_SHIFT = 20,
	PA6_SELECT_CFG0_SHIFT = 24,
	PA8_SELECT_CFG1_SHIFT = 0,
	PA9_SELECT_CFG1_SHIFT = 4,
	PA10_SELECT_CFG1_SHIFT = 8,
	PA11_SELECT_CFG1_SHIFT = 12,
	PA12_SELECT_CFG1_SHIFT = 16,
	PA13_SELECT_CFG1_SHIFT = 20,
	PA14_SELECT_CFG1_SHIFT = 24,
	PA15_SELECT_CFG1_SHIFT = 28,
	PA16_SELECT_CFG2_SHIFT = 0,
	PA17_SELECT_CFG2_SHIFT = 4,
	PA18_SELECT_CFG2_SHIFT = 8,
	PA19_SELECT_CFG2_SHIFT = 12
};

enum H3_PL_SELECT_SHIFT {
	PL0_SELECT_CFG0_SHIFT = 0,
	PL1_SELECT_CFG0_SHIFT = 4,
	PL2_SELECT_CFG0_SHIFT = 8,
	PL3_SELECT_CFG0_SHIFT = 12,
	PL8_SELECT_CFG1_SHIFT = 0,
	PL9_SELECT_CFG1_SHIFT = 4,
	PL10_SELECT_CFG1_SHIFT = 8,
	PL11_SELECT_CFG1_SHIFT = 12
};

enum H3_PG_SELECT_SHIFT {
	PG6_SELECT_CFG0_SHIFT = 24,
	PG7_SELECT_CFG0_SHIFT = 28,
	PG8_SELECT_CFG1_SHIFT = 0,
	PG9_SELECT_CFG1_SHIFT = 4
};

enum H3_PA_PULL0_SHIFT {
	H3_PA5_PULL0_SHIFT = 10
};

typedef enum H3_GPIO_PULL {
	GPIO_PULL_DISABLE = 0,
	GPIO_PULL_UP = 1,
	GPIO_PULL_DOWN = 2,
	GPIO_PULL_RESERVED = 3
} gpio_pull_t;

enum H3_PA_SELECT {
	H3_PA0_SELECT_UART2_TX = 2,
	H3_PA1_SELECT_UART2_RX = 2,

	H3_PA4_SELECT_UART0_TX = 2,
	H3_PA5_SELECT_UART0_RX = 2,

	H3_PA11_SELECT_TWI0_SCK = 2,
	H3_PA12_SELECT_TWI0_SDA = 2,

	H3_PA13_SELECT_SPI1_CS = 2,
	H3_PA14_SELECT_SPI1_CLK = 2,
	H3_PA15_SELECT_SPI1_MOSI = 2,
	H3_PA16_SELECT_SPI1_MISO = 2,

	H3_PA13_SELECT_UART3_TX = 3,
	H3_PA14_SELECT_UART3_RX = 3,

	H3_PA18_SELECT_TWI1_SCK = 3,
	H3_PA19_SELECT_TWI1_SDA = 3
};

enum H3_PC_SELECT {
	H3_PC0_SELECT_SPI0_MOSI = 3,
	H3_PC1_SELECT_SPI0_MISO = 3,
	H3_PC2_SELECT_SPI0_CLK = 3,
	H3_PC3_SELECT_SPI0_CS = 3
};

enum H3_PG_SELECT {
	H3_PG6_SELECT_UART1_TX = 2,
	H3_PG7_SELECT_UART1_RX = 2
};

extern void h3_gpio_fsel(_gpio_pin pin, gpio_fsel_t fsel);

inline static void h3_gpio_clr(_gpio_pin pin) {
	switch H3_GPIO_TO_PORT(pin) {
		case H3_GPIO_PORTA:
			H3_PIO_PORTA->DAT &= ~(1 << pin);
			break;
		case H3_GPIO_PORTC:
			H3_PIO_PORTC->DAT &= ~(1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTD:
			H3_PIO_PORTD->DAT &= ~(1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTE:
			H3_PIO_PORTE->DAT &= ~(1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTF:
			H3_PIO_PORTF->DAT &= ~(1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTG:
			H3_PIO_PORTG->DAT &= ~(1 << H3_GPIO_TO_NUMBER(pin));
			break;
		default:
			break;
	}
}

inline static void h3_gpio_set(_gpio_pin pin) {
	switch H3_GPIO_TO_PORT(pin) {
		case H3_GPIO_PORTA:
			H3_PIO_PORTA->DAT |= (1 << pin);
			break;
		case H3_GPIO_PORTC:
			H3_PIO_PORTC->DAT |= (1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTD:
			H3_PIO_PORTD->DAT |= (1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTE:
			H3_PIO_PORTE->DAT |= (1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTF:
			H3_PIO_PORTF->DAT |= (1 << H3_GPIO_TO_NUMBER(pin));
			break;
		case H3_GPIO_PORTG:
			H3_PIO_PORTG->DAT |= (1 << H3_GPIO_TO_NUMBER(pin));
			break;
		default:
			break;
	}
}

//FIXME Remove
//TODO Backwards compatibility with the Raspberry Pi API's
#define bcm2835_gpio_clr	h3_gpio_clr
#define bcm2835_gpio_set	h3_gpio_set
#define bcm2835_gpio_fsel	h3_gpio_fsel
//
#define BCM2835_GPIO_FSEL_OUTP	GPIO_FSEL_OUTPUT
//
#define bcm2835_delay(x)		udelay(x * 1000)

#endif /* H3_GPIO_H_ */
