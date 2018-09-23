/**
 * @file h3_opi_zero.h
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

#ifndef H3_OPI_ZERO_H_
#define H3_OPI_ZERO_H_

#if !defined(ORANGE_PI)
 #error This file should not be included
#endif

#include "h3.h"

#define H3_BOARD_NAME		"Orange Pi Zero"
#define H3_BOARD_STATUS_LED	H3_PORT_TO_GPIO(H3_GPIO_PORTA, 17)	///< PA17

#define EXT_I2C_BASE	H3_TWI0_BASE
#define EXT_SPI_BASE	H3_SPI1_BASE
#define EXT_UART_BASE 	H3_UART1_BASE

typedef enum H3_BOARD_OPI_ZERO {
	GPIO_EXT_3 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 12),	///< I2C0 SCA, PA12
	GPIO_EXT_5 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 11),	///< I2C0 SDL, PA11
	GPIO_EXT_7 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 6),		///< PA6
	GPIO_EXT_11 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 1),	///< UART2 RX, PA1
	GPIO_EXT_13 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 0),	///< UART2 TX, PA0
	GPIO_EXT_15 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 3),	///< PA3

	GPIO_EXT_19 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 15),	///< SPI1 MOSI, PA15
	GPIO_EXT_21 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 16),	///< SPI1 MISO, PA16
	GPIO_EXT_23 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 14),	///< SPI1 SCLK, PA14
	//
	GPIO_EXT_8 = H3_PORT_TO_GPIO(H3_GPIO_PORTG, 6),		///< UART1 TX, PG6
	GPIO_EXT_10 = H3_PORT_TO_GPIO(H3_GPIO_PORTG, 7),	///< UART1 RX, PG7
	GPIO_EXT_12 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 7),	///< PA7
	GPIO_EXT_16 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 19),	///< PA19
	GPIO_EXT_18 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 18),	///< PA18
	GPIO_EXT_22 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 2),	///< PA2
	GPIO_EXT_24 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 13),	///< SPI1 SPICS0, PA13
	GPIO_EXT_26 = H3_PORT_TO_GPIO(H3_GPIO_PORTA, 10)	///< PA10
} _gpio_pin;

#endif /* H3_OPI_ZERO_H_ */
