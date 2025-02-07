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

#ifndef H3_WS28XXMULTI_H_
#define H3_WS28XXMULTI_H_

#if defined (DEBUG_PIXEL)
# if defined (NDEBUG)
#  undef NDEBUG
#  define _NDEBUG
# endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O3")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
# pragma GCC optimize ("-fprefetch-loop-arrays")
#endif

#include <cstdint>

#include "pixelconfiguration.h"

#include "h3_spi.h"
#include "h3.h"

#include "logic_analyzer.h"

#include "debug.h"

struct JamSTAPLDisplay;

class WS28xxMulti {
public:
	WS28xxMulti();
	~WS28xxMulti();

	inline void SetColourRTZ(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
		SetColour(nPortIndex, nPixelIndex, nColour1, nColour2, nColour3);
	}

#define BIT_SET(a,b) 	((a) |= static_cast<uint8_t>((1<<(b))))
#define BIT_CLEAR(a,b) 	((a) &= static_cast<uint8_t>(~(1<<(b))))

	inline void SetColourRTZ(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
		uint32_t localBuffer[32] __attribute__((aligned(32)));
		const auto k = nPixelIndex * pixel::single::RGBW;

	    for (uint32_t i = 0; i < 32; i++) {
	        localBuffer[i] = m_pPixelDataBuffer[k + i];
	    }

		uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			// GRBW
			if (mask & nGreen) {
				BIT_SET(localBuffer[j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[j], nPortIndex);
			}

			if (mask & nRed) {
				BIT_SET(localBuffer[8 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[8 + j], nPortIndex);
			}

			if (mask & nBlue) {
				BIT_SET(localBuffer[16 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[16 + j], nPortIndex);
			}

			if (mask & nWhite) {
				BIT_SET(localBuffer[24 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[24 + j], nPortIndex);
			}

			j++;
		}

	    // Write back to m_pBuffer1
	    for (uint32_t i = 0; i < 32; i++) {
	    	m_pPixelDataBuffer[k + i] = localBuffer[i];
	    }
	}

	inline void SetColourWS2801(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3) {
		SetColour(nPortIndex, nPixelIndex, nColour1, nColour2, nColour3);
	}

	inline void SetPixel4Bytes(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
	    uint32_t localBuffer[32] __attribute__((aligned(32)));
		const auto k = nPixelIndex * pixel::single::RGBW;

	    for (uint32_t i = 0; i < 32; i++) {
	        localBuffer[i] = m_pPixelDataBuffer[k + i];
	    }

		uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			// GRBW
			if (mask & nGreen) {
				BIT_SET(localBuffer[j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[j], nPortIndex);
			}

			if (mask & nRed) {
				BIT_SET(localBuffer[8 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[8 + j], nPortIndex);
			}

			if (mask & nBlue) {
				BIT_SET(localBuffer[16 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[16 + j], nPortIndex);
			}

			if (mask & nWhite) {
				BIT_SET(localBuffer[24 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[24 + j], nPortIndex);
			}

			j++;
		}

	    // Write back to m_pBuffer1
	    for (uint32_t i = 0; i < 32; i++) {
	    	m_pPixelDataBuffer[k + i] = localBuffer[i];
	    }
	}

	bool IsUpdating() {
		return h3_spi_dma_tx_is_active();  // returns TRUE while DMA operation is active
	}

	void Update();
	void Blackout();
	void FullOn();

	uint32_t GetUserData();

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
	    uint32_t localBuffer[24] __attribute__((aligned(32)));
	    const uint32_t k = nPixelIndex * pixel::single::RGB;

	    for (uint32_t i = 0; i < 24; i++) {
	        localBuffer[i] = m_pPixelDataBuffer[k + i];
	    }

	    uint32_t j = 0;

		for (uint8_t mask = 0x80; mask != 0; mask = static_cast<uint8_t>(mask >> 1)) {
			if (mask & nColour1) {
				BIT_SET(localBuffer[j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[j], nPortIndex);
			}
			if (mask & nColour2) {
				BIT_SET(localBuffer[8 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[8 + j], nPortIndex);
			}
			if (mask & nColour3) {
				BIT_SET(localBuffer[16 + j], nPortIndex);
			} else {
				BIT_CLEAR(localBuffer[16 + j], nPortIndex);
			}

			j++;
		}

	    // Write back to m_pBuffer1
	    for (uint32_t i = 0; i < 24; i++) {
	    	m_pPixelDataBuffer[k + i] = localBuffer[i];
	    }
	}

private:
	uint32_t m_nBufSize { 0 };

	uint8_t *const m_pPixelDataBuffer { reinterpret_cast<uint8_t *>(H3_SRAM_A1_BASE + 512) };
	uint8_t *m_pDmaBuffer { nullptr };

	JamSTAPLDisplay *m_pJamSTAPLDisplay { nullptr };

	bool m_hasCPLD { false };

	static inline WS28xxMulti *s_pThis;
};

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC pop_options
#endif
#if defined (_NDEBUG)
# undef _NDEBUG
# define NDEBUG
#endif
#endif /* H3_WS28XXMULTI_H_ */
