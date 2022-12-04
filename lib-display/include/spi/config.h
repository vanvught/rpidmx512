/**
 * @file config.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef CONFIG_H
#define CONFIG_H

namespace config {
#if defined (SPI_LCD_240X240)
static constexpr auto WIDTH = 240;
static constexpr auto HEIGHT = 240;
#elif defined (SPI_LCD_240X320)
static constexpr auto WIDTH = 240;
static constexpr auto HEIGHT = 320;
#else
# error lib-display spi config
#endif
}  // namespace config

#if defined (H3)
# define SPI_LCD_RST_PIN		GPIO_EXT_7			// GPIO6
# define SPI_LCD_DC_PIN 		GPIO_EXT_26			// GPIO10
# define SPI_LCD_BL_PIN			GPIO_EXT_22			// GPIO2
# if defined(SPI_LCD_HAVE_CS_PIN)
#  define SPI_LCD_CS_PIN		GPIO_EXT_24			// GPIO13 / SPI CS0
# endif
#elif defined (GD32)		//TODO Move to board file
# include "gd32.h"
# if defined(BOARD_GD32F107RC) || defined(BOARD_GD32F207RG) || defined(BOARD_GD32F407RE)
#  define SPI_LCD_RST_PIN		GPIO_EXT_7
#  define SPI_LCD_DC_PIN 		GPIO_EXT_26
#  define SPI_LCD_BL_PIN		GPIO_EXT_22
#  if defined(SPI_LCD_HAVE_CS_PIN)
#   define SPI_LCD_CS_PIN		GPIO_EXT_24
#  endif
# elif defined(BOARD_GD32F207VC)
#  define SPI_LCD_DC_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTB, 1)
#  define SPI_LCD_BL_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTA, 3)
#  define SPI_LCD_CS_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTE, 5)
# elif defined(BOARD_GD32F450VE)
#  define SPI_LCD_DC_PIN        GD32_PORT_TO_GPIO(GD32_GPIO_PORTB, 1)
#  define SPI_LCD_BL_PIN        GD32_PORT_TO_GPIO(GD32_GPIO_PORTA, 3)
#  define SPI_LCD_CS_PIN        GD32_PORT_TO_GPIO(GD32_GPIO_PORTE, 5)
# elif defined(BOARD_GD32F207C_EVAL)
#  define SPI_LCD_DC_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTB, 1)
#  define SPI_LCD_BL_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTA, 3)
#  define SPI_LCD_CS_PIN		GD32_PORT_TO_GPIO(GD32_GPIO_PORTE, 5)
# else
#  error
# endif
#else
# include "bcm2835.h"
# define SPI_LCD_RST_PIN		RPI_V2_GPIO_P1_07	// GPIO4
# define SPI_LCD_DC_PIN 		RPI_V2_GPIO_P1_31	// GPIO6
# define SPI_LCD_BL_PIN			RPI_V2_GPIO_P1_29	// GPIO5
# if defined(SPI_LCD_HAVE_CS_PIN)
#  define SPI_LCD_CS_PIN		RPI_V2_GPIO_P1_24	// GPIO8 / SPI CS0
# endif
#endif

#endif /* CONFIG_H */
