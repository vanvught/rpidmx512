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
static constexpr auto WIDTH = 240U;
static constexpr auto HEIGHT = 240U;
#elif defined (SPI_LCD_240X320)
static constexpr auto WIDTH = 240U;
static constexpr auto HEIGHT = 320U;
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
#elif defined (GD32)		//See board file
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
