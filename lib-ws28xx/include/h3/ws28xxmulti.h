/**
 * @file ws28xxmulti.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "gamma/gamma_tables.h"

#include "h3_spi.h"
#include "h3.h"

struct JamSTAPLDisplay;

class WS28xxMulti {
public:
	WS28xxMulti();
	~WS28xxMulti();

	void SetColourRTZ(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
		SetColour(nPortIndex, nPixelIndex, nColour1, nColour2, nColour3);
	}

#define BIT_SET(a,b) 	((a) |= static_cast<uint8_t>((1<<(b))))
#define BIT_CLEAR(a,b) 	((a) &= static_cast<uint8_t>(~(1<<(b))))

	void SetColourRTZ(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
		const auto pGammaTable = m_PixelConfiguration.GetGammaTable();

		nRed = pGammaTable[nRed];
		nGreen = pGammaTable[nGreen];
		nBlue = pGammaTable[nBlue];
		nWhite = pGammaTable[nWhite];
#endif

		const auto k = nPixelIndex * pixel::single::RGBW;
		uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			// GRBW
			if (mask & nGreen) {
				BIT_SET(m_pBuffer1[k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[k + j], nPortIndex);
			}

			if (mask & nRed) {
				BIT_SET(m_pBuffer1[8 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[8 + k + j], nPortIndex);
			}

			if (mask & nBlue) {
				BIT_SET(m_pBuffer1[16 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[16 + k + j], nPortIndex);
			}

			if (mask & nWhite) {
				BIT_SET(m_pBuffer1[24 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[24 + k + j], nPortIndex);
			}

			j++;
		}
	}	void SetColourWS2801(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
		SetColour(nPortIndex, nPixelIndex, nColour1, nColour2, nColour3);
	}

	void SetPixel4Bytes(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
		const auto k = nPixelIndex * pixel::single::RGBW;
		uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			// GRBW
			if (mask & nGreen) {
				BIT_SET(m_pBuffer1[k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[k + j], nPortIndex);
			}

			if (mask & nRed) {
				BIT_SET(m_pBuffer1[8 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[8 + k + j], nPortIndex);
			}

			if (mask & nBlue) {
				BIT_SET(m_pBuffer1[16 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[16 + k + j], nPortIndex);
			}

			if (mask & nWhite) {
				BIT_SET(m_pBuffer1[24 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[24 + k + j], nPortIndex);
			}

			j++;
		}
	}
	bool IsUpdating() {
		return h3_spi_dma_tx_is_active();  // returns TRUE while DMA operation is active
	}

	void Update();
	void Blackout();
	void FullOn();

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

	void SetColour(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
		uint32_t j = 0;
		const uint32_t k = nPixelIndex * pixel::single::RGB;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			if (mask & nColour1) {
				BIT_SET(m_pBuffer1[k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[k + j], nPortIndex);
			}
			if (mask & nColour2) {
				BIT_SET(m_pBuffer1[8 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[8 + k + j], nPortIndex);
			}
			if (mask & nColour3) {
				BIT_SET(m_pBuffer1[16 + k + j], nPortIndex);
			} else {
				BIT_CLEAR(m_pBuffer1[16 + k + j], nPortIndex);
			}

			j++;
		}
	}

private:
	uint32_t m_nBufSize { 0 };

	uint8_t *m_pBuffer1 { nullptr };
	uint8_t *m_pBuffer2 { nullptr };

	JamSTAPLDisplay *m_pJamSTAPLDisplay { nullptr };

	bool m_hasCPLD { false };

	static inline WS28xxMulti *s_pThis;
};

#endif /* SPI_WS28XXMULTI_H_ */
