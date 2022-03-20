/**
 * @file ws28xxmulti.h
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef SPI_WS28XXMULTI_H_
#define SPI_WS28XXMULTI_H_

#include <cstdint>

#include "pixelconfiguration.h"

#if defined (H3)
# include "h3_spi.h"
#endif

struct JamSTAPLDisplay;

class WS28xxMulti {
public:
	WS28xxMulti(PixelConfiguration& pixelConfiguration);
	~WS28xxMulti();

	void Print();

	void SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	bool IsUpdating() {
		return h3_spi_dma_tx_is_active();  // returns TRUE while DMA operation is active
	}

	void Update();
	void Blackout();

	pixel::Type GetType() const {
		return m_Type;
	}

	uint32_t GetCount() const {
		return m_nCount;
	}

	pixel::Map GetMap() const {
		return m_Map;
	}

	void SetJamSTAPLDisplay(JamSTAPLDisplay *pJamSTAPLDisplay) {
		m_pJamSTAPLDisplay = pJamSTAPLDisplay;
	}

	static WS28xxMulti *Get() {
		return s_pThis;
	}

private:
	uint8_t ReverseBits(uint8_t nBits);
	void SetupHC595(uint8_t nT0H, uint8_t nT1H);
	void SetupSPI(uint32_t nSpeedHz);
	bool SetupCPLD();
	void SetupBuffers();
	void SetColour(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3);

private:
	bool m_hasCPLD { false };
	pixel::Type m_Type { pixel::defaults::TYPE };
	uint32_t m_nCount { pixel::defaults::COUNT };
	pixel::Map m_Map { pixel::Map::UNDEFINED };
	bool m_bIsRTZProtocol { true };
	uint8_t m_nGlobalBrightness { 0xFF };
	uint32_t m_nBufSize { 0 };
	uint8_t *m_pBuffer { nullptr };
	uint8_t *m_pBlackoutBuffer { nullptr };
	JamSTAPLDisplay *m_pJamSTAPLDisplay { nullptr };

	static WS28xxMulti *s_pThis;
};

#endif /* SPI_WS28XXMULTI_H_ */
