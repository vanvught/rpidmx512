/**
 * @file h3_board.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include "h3.h"

#if defined(ORANGE_PI)
# include "board/h3_opi_zero.h"
#elif defined(ORANGE_PI_ONE)
# include "board/h3_opi_one.h"
#else
# error Board configuration error
#endif

#ifndef H3_BOARD_NAME
# error Board configuration error
#endif

#ifndef H3_BOARD_STATUS_LED
# error Status LED not defined
#endif

#if !defined(EXT_UART_BASE)
# error EXT_UART_BASE not defined
#endif

#if !defined(EXT_I2C_BASE)
# error EXT_I2C_BASE not defined
#endif

#if !defined(EXT_SPI_BASE)
# error EXT_SPI_BASE not defined
#endif

#ifdef __cplusplus
# define _CAST(x)	reinterpret_cast<x>
#else
# define _CAST(x)	(x)
#endif

#define EXT_UART_NUMBER			((EXT_UART_BASE - H3_UART_BASE) / 0x400)
#define EXT_UART				(_CAST(H3_UART_TypeDef *)(EXT_UART_BASE))

#define EXT_UART_TX				GPIO_EXT_8
#define EXT_UART_RX				GPIO_EXT_10

#define EXT_I2C_NUMBER			((EXT_I2C_BASE - H3_TWI_BASE) / 0x400)
#define EXT_I2C					(_CAST(H3_TWI_TypeDef *)(EXT_I2C_BASE))
#define EXT_I2C_SDA				GPIO_EXT_3
#define EXT_I2C_SCL				GPIO_EXT_5

#define EXT_SPI_NUMBER			((EXT_SPI_BASE - H3_SPI_BASE) / 0x1000)
#define EXT_SPI					(_CAST(H3_SPI_TypeDef *)(EXT_SPI_BASE))
#define EXT_SPI_CS				GPIO_EXT_24
#define EXT_SPI_CLK				GPIO_EXT_23
#define EXT_SPI_MOSI			GPIO_EXT_19
#define EXT_SPI_MISO			GPIO_EXT_21

#define KEY1_GPIO				GPIO_EXT_7
#define KEY2_GPIO				GPIO_EXT_15
#define KEY3_GPIO				GPIO_EXT_22

#define PIXELDMXSTARTSTOP_GPIO	GPIO_EXT_12
#define DISPLAYTIMEOUT_GPIO		GPIO_EXT_15	// KEY2

void h3_board_dump();

#endif /* H3_BOARD_H_ */
