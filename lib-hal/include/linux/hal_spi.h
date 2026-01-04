/**
 * @file hal_spi.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_HAL_SPI_H_
#define LINUX_HAL_SPI_H_

#if defined (RASPPI)
# define SPI_BIT_ORDER_MSBFIRST	BCM2835_SPI_BIT_ORDER_MSBFIRST
# define SPI_MODE0				BCM2835_SPI_MODE0
# define SPI_MODE1				BCM2835_SPI_MODE1
# define SPI_MODE2				BCM2835_SPI_MODE2
# define SPI_MODE3				BCM2835_SPI_MODE3
# define SPI_CS0				BCM2835_SPI_CS0
# define SPI_CS_NONE			BCM2835_SPI_CS_NONE
#else
# define SPI_BIT_ORDER_MSBFIRST	0
# define SPI_MODE0				0
# define SPI_MODE1				0
# define SPI_MODE2				0
# define SPI_MODE3				0
# define SPI_CS0				0
# define SPI_CS_NONE			0
# define FUNC_PREFIX(x) x
# ifdef __cplusplus
#  include <cstdint>
  extern "C" {
# else
#  include <stdint.h>
# endif
  inline static void SpiBegin() {}
  inline static void SpiChipSelect([[maybe_unused]] uint8_t _q) {}
  inline static void SpiSetDataMode([[maybe_unused]] uint8_t _q) {}
  inline static void SpiSetSpeedHz([[maybe_unused]] uint32_t _q) {}
  inline static void SpiWrite([[maybe_unused]] uint16_t _q) {}
  inline static void SpiTransfern([[maybe_unused]] const char *_p, [[maybe_unused]] uint32_t _q) {}
  inline static void SpiWritenb([[maybe_unused]] const char *_p, [[maybe_unused]] uint32_t _q) {}
# ifdef __cplusplus
 }
# endif
#endif

#endif /* LINUX_HAL_SPI_H_ */
