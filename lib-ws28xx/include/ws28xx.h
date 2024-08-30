/**
 * @file ws28xx.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "hal_spi.h"

class WS28xx {
public:
	WS28xx();
	~WS28xx();

	void SetPixel(uint32_t nIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel(uint32_t nIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	bool IsUpdating () {
#if defined (GD32)
		return i2s::gd32_spi_dma_tx_is_active();
#elif defined (H3)
		return h3_spi_dma_tx_is_active();
#else
		return false;
#endif
	}

	void Update();
	void Blackout();
	void FullOn();

	static WS28xx *Get() {
		return s_pThis;
	}

private:
	void SetupBuffers();
	void SetColorWS28xx(uint32_t nOffset, uint8_t nValue);

private:
	uint32_t m_nBufSize;
	uint8_t *m_pBuffer { nullptr };
	uint8_t *m_pBlackoutBuffer { nullptr };

	static WS28xx *s_pThis;
};

#endif /* WS28XX_H_ */
