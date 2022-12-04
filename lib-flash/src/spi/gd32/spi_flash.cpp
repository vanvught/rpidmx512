/**
 * @file spi_flash.cpp
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

#include <cstdint>

#include "./../../spi/spi_flash_internal.h"

#include "gd32_spi.h"
#include "gd32.h"

int spi_init() {
	gd32_spi_begin();
	gd32_spi_chipSelect(GD32_SPI_CS_NONE);
	gd32_spi_set_speed_hz(SPI_XFER_SPEED_HZ);
	gd32_spi_setDataMode(GD32_SPI_MODE0);

	rcu_periph_clock_enable(SPI_FLASH_CS_RCU_GPIOx);

#if !defined (GD32F4XX)
	gpio_init(SPI_FLASH_CS_GPIOx, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_FLASH_CS_GPIO_PINx);
#else
	gpio_af_set(SPI_FLASH_CS_GPIOx, GPIO_AF_0, SPI_FLASH_CS_GPIO_PINx);
	gpio_mode_set(SPI_FLASH_CS_GPIOx, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI_FLASH_CS_GPIO_PINx);
	gpio_output_options_set(SPI_FLASH_CS_GPIOx, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_FLASH_CS_GPIO_PINx);
#endif

	GPIO_BOP(SPI_FLASH_CS_GPIOx) = SPI_FLASH_CS_GPIO_PINx;

	return 0;
}

inline static void spi_transfern(char *pBuffer, uint32_t nLength) {
	gd32_spi_transfernb(pBuffer, pBuffer, nLength);
}

int spi_xfer(uint32_t nLength, const uint8_t *pOut, uint8_t *pIn, uint32_t nFlags) {
	if (nFlags & SPI_XFER_BEGIN) {
		GPIO_BC(SPI_FLASH_CS_GPIOx) = SPI_FLASH_CS_GPIO_PINx;
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
		GPIO_BOP(SPI_FLASH_CS_GPIOx) = SPI_FLASH_CS_GPIO_PINx;
	}

	return 0;
}
