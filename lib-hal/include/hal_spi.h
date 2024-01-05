/**
 * @file hal_spi.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef HAL_SPI_H_
#define HAL_SPI_H_

#include "hal_api.h"

#if defined(__linux__) || defined (__APPLE__)
# include "linux/hal_api.h"
# include "linux/hal_spi.h"
#elif defined(H3)
# include "h3/hal_api.h"
# include "h3/hal_spi.h"
#elif defined(GD32)
# include "gd32/hal_api.h"
# include "gd32/hal_spi.h"
#else
# include "rpi/hal_api.h"
# include "rpi/hal_spi.h"
#endif

#ifdef __cplusplus
# include <cstdint>

class HAL_SPI {
	void Setup() {
		FUNC_PREFIX(spi_chipSelect(m_nChipSelect));
		FUNC_PREFIX(spi_setDataMode	(m_nMode));
		FUNC_PREFIX(spi_set_speed_hz(m_nSpeedHz));
	}
public:
	HAL_SPI(uint8_t nChipSelect, uint32_t nSpeedHz, uint8_t nMode = 0) :
		m_nSpeedHz(nSpeedHz), m_nChipSelect(nChipSelect), m_nMode(nMode & 0x3) {
		FUNC_PREFIX(spi_begin());
	}

	void Write(const char *pData, uint32_t nLength, const bool bDoSetup = true) {
		if (bDoSetup) {
			Setup();
		}
		FUNC_PREFIX(spi_writenb(pData, nLength));
	}

	void Write(uint16_t nData, const bool bDoSetup = true) {
		if (bDoSetup) {
			Setup();
		}
		FUNC_PREFIX(spi_write(nData));
	}

	void WriteRead(char *pData, uint32_t nLength, const bool bDoSetup = true) {
		if (bDoSetup) {
			Setup();
		}
		FUNC_PREFIX(spi_transfern(pData, nLength));
	}

private:
	uint32_t m_nSpeedHz;
	uint8_t m_nChipSelect;
	uint8_t m_nMode;
};
#endif

#endif /* HAL_SPI_H_ */
