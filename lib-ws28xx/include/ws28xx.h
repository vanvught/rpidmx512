/**
 * @file ws28xx.h
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XX_H_
#define WS28XX_H_

#include <cstdint>

#include "pixelconfiguration.h"

#if defined (USE_SPI_DMA)
# include "hal_spi.h"
#endif

class WS28xx {
public:
	WS28xx(PixelConfiguration& pixelConfiguration);
	~WS28xx();

	void SetPixel(uint32_t nIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel(uint32_t nIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

#if defined ( USE_SPI_DMA )
	bool IsUpdating () {
		return FUNC_PREFIX (spi_dma_tx_is_active());
	}
#else
	bool IsUpdating() const {
		return false;
	}
#endif

	void Update();
	void Blackout();
	void FullOn();

	pixel::Type GetType() const {
		return m_PixelConfiguration.GetType();
	}

	uint32_t GetCount() const {
		return m_PixelConfiguration.GetCount();
	}

	pixel::Map GetMap() const {
		return m_PixelConfiguration.GetMap();
	}

	static WS28xx *Get() {
		return s_pThis;
	}

private:
	void SetupBuffers();
	void SetColorWS28xx(uint32_t nOffset, uint8_t nValue);

private:
	PixelConfiguration m_PixelConfiguration;
	uint32_t m_nBufSize;
	uint8_t *m_pBuffer { nullptr };
	uint8_t *m_pBlackoutBuffer { nullptr };

	static WS28xx *s_pThis;
};

#endif /* WS28XX_H_ */
