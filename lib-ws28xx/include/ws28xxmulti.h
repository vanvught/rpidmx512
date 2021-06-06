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

#ifndef WS28XXMULTI_H_
#define WS28XXMULTI_H_

#include <cstdint>

#include "pixelconfiguration.h"

#if defined (H3)
# include "h3_spi.h"
#endif

namespace ws28xxmulti {
enum class Board {
	X4, X8, UNKNOWN
};
namespace defaults {
static constexpr auto BOARD = Board::X4;
}  // namespace defaults
}  // namespace ws28xxmulti

struct JamSTAPLDisplay;

class WS28xxMulti {
public:
	WS28xxMulti(PixelConfiguration& pixelConfiguration);
	~WS28xxMulti();

	void Print();

	void SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
		if (m_Board == ws28xxmulti::Board::X8) {
			SetPixel8x(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		} else {
			SetPixel4x(nPortIndex, nPixelIndex, nRed, nGreen, nBlue);
		}
	}
	void SetPixel(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
		if (m_Board == ws28xxmulti::Board::X8) {
			SetPixel8x(nPortIndex, nPixelIndex, nRed, nGreen, nBlue, nWhite);
		} else {
			SetPixel4x(nPortIndex, nPixelIndex, nRed, nGreen, nBlue, nWhite);
		}
	}

#if defined (H3)
	bool IsUpdating() {
		if (m_Board == ws28xxmulti::Board::X8) {
			return h3_spi_dma_tx_is_active();  // returns TRUE while DMA operation is active
		} else {
			return false;
		}
	}
#else
	bool IsUpdating(void) const {
		return false;
	}
#endif

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

// 8x
	void SetJamSTAPLDisplay(JamSTAPLDisplay *pJamSTAPLDisplay) {
		m_pJamSTAPLDisplay = pJamSTAPLDisplay;
	}

	static ws28xxmulti::Board GetBoard();

	static WS28xxMulti *Get() {
		return s_pThis;
	}

private:
	uint8_t ReverseBits(uint8_t nBits);
// 4x
	static bool IsMCP23017();
	bool SetupMCP23017(uint8_t nT0H, uint8_t nT1H);
	bool SetupSI5351A();
	void SetupGPIO();
	void SetupBuffers4x();
	void Generate800kHz(const uint32_t *pBuffer);
	void SetColour4x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3);
	void SetPixel4x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel4x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);
// 8x
	void SetupHC595(uint8_t nT0H, uint8_t nT1H);
	void SetupSPI(uint32_t nSpeedHz);
	bool SetupCPLD();
	void SetupBuffers8x();
	void SetColour8x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nColour1, uint8_t nColour2, uint8_t nColour3);
	void SetPixel8x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel8x(uint32_t nPortIndex, uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

private:
	ws28xxmulti::Board m_Board { ws28xxmulti::defaults::BOARD };
	bool m_hasCPLD { false };
	pixel::Type m_Type { pixel::defaults::TYPE };
	uint32_t m_nCount { pixel::defaults::COUNT };
	pixel::Map m_Map { pixel::Map::UNDEFINED };
	bool m_bIsRTZProtocol { true };
	uint8_t m_nGlobalBrightness { 0xFF };
	uint32_t m_nBufSize { 0 };
	uint32_t *m_pBuffer4x { nullptr };
	uint32_t *m_pBlackoutBuffer4x { nullptr };
	uint8_t *m_pBuffer8x { nullptr };
	uint8_t *m_pBlackoutBuffer8x { nullptr };
	JamSTAPLDisplay *m_pJamSTAPLDisplay { nullptr };

	static WS28xxMulti *s_pThis;
};

#endif /* WS28XXMULTI_H_ */
