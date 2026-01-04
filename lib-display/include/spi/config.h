/**
 * @file config.h
 *
 */
/* Copyright (C) 2022-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SPI_CONFIG_H_
#define SPI_CONFIG_H_

#include <cstdint>

namespace config
{
#if defined(SPI_LCD_240X240)
inline constexpr uint32_t kWidth = 240;
inline constexpr uint32_t kHeight = 240;
#elif defined(SPI_LCD_240X320)
inline constexpr uint32_t kWidth = 240;
inline constexpr uint32_t kHeight = 320;
#elif defined(SPI_LCD_128X128)
inline constexpr uint32_t kWidth = 128;
inline constexpr uint32_t kHeight = 128;
#elif defined(SPI_LCD_160X80)
inline constexpr uint32_t kWidth = 80;
inline constexpr uint32_t kHeight = 160;
#else
#error lib-display spi config
#endif
} // namespace config

#if defined(H3)
#define SPI_LCD_RST_GPIO GPIO_EXT_7 // GPIO6
#define SPI_LCD_DC_GPIO GPIO_EXT_26 // GPIO10
#define SPI_LCD_BL_GPIO GPIO_EXT_22 // GPIO2
#if defined(SPI_LCD_HAVE_CS_GPIO)
#define SPI_LCD_CS_GPIO GPIO_EXT_24 // GPIO13 / SPI CS0
#endif
#elif defined(GD32) // See board file
#else
#include "bcm2835.h"
#define SPI_LCD_RST_GPIO RPI_V2_GPIO_P1_07 // GPIO4
#define SPI_LCD_DC_GPIO RPI_V2_GPIO_P1_31  // GPIO6
#define SPI_LCD_BL_GPIO RPI_V2_GPIO_P1_29  // GPIO5
#if defined(SPI_LCD_HAVE_CS_GPIO)
#define SPI_LCD_CS_GPIO RPI_V2_GPIO_P1_24 // GPIO8 / SPI CS0
#endif
#endif

#endif  // SPI_CONFIG_H_
