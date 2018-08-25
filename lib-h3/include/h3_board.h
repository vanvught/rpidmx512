/**
 * @file h3_board.h
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

#ifndef H3_BOARD_H_
#define H3_BOARD_H_

#define H3_PORT_TO_GPIO(p,n)	((p * 32) + n)
#define H3_GPIO_TO_PORT(g)		(g / 32)
#define H3_GPIO_TO_NUMBER(g)	(g - (32 * H3_GPIO_TO_PORT(g)))

#if defined(ORANGE_PI)
 #include "board/h3_opi_zero.h"
#elif defined(ORANGE_PI_ONE)
 #include "board/h3_opi_one.h"
#elif defined(NANO_PI)
 #include "board/h3_nanopi_neo.h"
#else
 #error Board configuration error
#endif

#ifndef H3_BOARD_NAME
 #error Board configuration error
#endif

#ifndef H3_BOARD_STATUS_LED
 #error Status LED not defined
#endif

#if !defined(EXT_UART_BASE)
 #error EXT_UART_BASE not defined
#endif

#if !defined(EXT_I2C_BASE)
 #error EXT_I2C_BASE not defined
#endif

#if !defined(EXT_SPI_BASE)
 #error EXT_SPI_BASE not defined
#endif

#include "h3.h"

#define EXT_UART_NUMBER		((EXT_UART_BASE - H3_UART_BASE) / 0x400)
#define EXT_UART			((H3_UART_TypeDef *) EXT_UART_BASE)
#define EXT_UART_TX			GPIO_EXT_8
#define EXT_UART_RX			GPIO_EXT_10

#define EXT_I2C_NUMBER		((EXT_I2C_BASE - H3_TWI_BASE) / 0x400)
#define EXT_I2C				((H3_TWI_TypeDef *) EXT_I2C_BASE)
#define EXT_I2C_SDA			GPIO_EXT_3
#define EXT_I2C_SCL			GPIO_EXT_5

#define EXT_SPI_NUMBER		((EXT_SPI_BASE - H3_SPI_BASE) / 0x1000)
#define EXT_SPI				((H3_SPI_TypeDef *) EXT_SPI_BASE)
#define EXT_SPI_CS			GPIO_EXT_24
#define EXT_SPI_CLK			GPIO_EXT_23
#define EXT_SPI_MOSI		GPIO_EXT_19
#define EXT_SPI_MISO		GPIO_EXT_21

#ifdef __cplusplus
extern "C" {
#endif

extern void h3_board_dump(void) ;

#ifdef __cplusplus
}
#endif

#endif /* H3_BOARD_H_ */
