/**
 * @file spi_flash.cpp
 *
 */
/* Copyright (C) 2022-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>

#include "./../../spi/spi_flash_internal.h"

#include "gd32_spi.h"
#include "gd32_gpio.h"
#include "gd32.h"

#include "debug.h"

int spi_init() {
	gd32_spi_begin();
	gd32_spi_chipSelect(GD32_SPI_CS_NONE);
	gd32_spi_set_speed_hz(SPI_XFER_SPEED_HZ);
	gd32_spi_setDataMode(GD32_SPI_MODE0);

	gpio_fsel(SPI_FLASH_CS_GPIOx, SPI_FLASH_CS_GPIO_PINx, GPIO_FSEL_OUTPUT);
	gpio_bit_set(SPI_FLASH_CS_GPIOx, SPI_FLASH_CS_GPIO_PINx);

#if defined (SPI_FLASH_WP_GPIO_PINx)
	gpio_fsel(SPI_GPIOx, SPI_FLASH_WP_GPIO_PINx, GPIO_FSEL_OUTPUT);
	gpio_bit_set(SPI_GPIOx, SPI_FLASH_WP_GPIO_PINx);
#endif

#if defined (SPI_FLASH_HOLD_GPIO_PINx)
	gpio_fsel(SPI_GPIOx, SPI_FLASH_HOLD_GPIO_PINx, GPIO_FSEL_OUTPUT);
	gpio_bit_set(SPI_GPIOx, SPI_FLASH_HOLD_GPIO_PINx);
#endif

	return 0;
}

inline static void spi_transfern(char *pBuffer, uint32_t nLength) {
	gd32_spi_transfernb(pBuffer, pBuffer, nLength);
}

int spi_xfer(uint32_t nLength, const uint8_t *pOut, uint8_t *pIn, uint32_t nFlags) {

	if (nFlags & SPI_XFER_BEGIN) {
		gpio_bit_reset(SPI_FLASH_CS_GPIOx, SPI_FLASH_CS_GPIO_PINx);
	}

	if (nLength != 0) {
		if (pIn == 0) {
			gd32_spi_writenb(reinterpret_cast<const char *>(pOut), nLength);
		} else if (pOut == 0) {
			spi_transfern(reinterpret_cast<char *>(pIn), nLength);
		} else {
			gd32_spi_transfernb(reinterpret_cast<const char *>(pOut), reinterpret_cast<char *>(pIn), nLength);
		}
	}

	if (nFlags & SPI_XFER_END) {
		gpio_bit_set(SPI_FLASH_CS_GPIOx, SPI_FLASH_CS_GPIO_PINx);
	}

	return 0;
}
