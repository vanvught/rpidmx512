/**
 * @file ws28xxmulti.h
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "ws28xx.h"

#include "rgbmapping.h"

#if defined (H3)
# include "h3/ws28xxdma.h"
#endif

enum WS28xxMultiBoard {
	WS28XXMULTI_BOARD_4X,
	WS28XXMULTI_BOARD_8X,
	WS28XXMULTI_BOARD_UNKNOWN
};

enum WS28xxMultiActivePorts {
	WS28XXMULTI_ACTIVE_PORTS_MAX = 4
};

class WS28xxMulti {
public:
	WS28xxMulti();
	~WS28xxMulti();

	void Initialize(TWS28XXType tWS28xxType, uint16_t nLedCount, TRGBMapping tRGBMapping = RGB_MAPPING_UNDEFINED, uint8_t nT0H = 0, uint8_t nT1H = 0, bool bUseSI5351A = false);

	TWS28XXType GetLEDType() const {
		return m_tWS28xxType;
	}

	TRGBMapping GetRgbMapping() const {
		return m_tRGBMapping;
	}

	uint8_t GetLowCode() const {
		return m_nLowCode;
	}

	uint8_t GetHighCode() const {
		return m_nHighCode;
	}

	uint16_t GetLEDCount() const {
		return m_nLedCount;
	}

	WS28xxMultiBoard GetBoard() const {
		return m_tBoard;
	}

	void SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue) {
		if (m_tBoard == WS28XXMULTI_BOARD_8X) {
			SetLED8x(nPort, nLedIndex, nRed, nGreen, nBlue);
		} else {
			SetLED4x(nPort, nLedIndex, nRed, nGreen, nBlue);
		}
	}
	void SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite) {
		if (m_tBoard == WS28XXMULTI_BOARD_8X) {
			SetLED8x(nPort, nLedIndex, nRed, nGreen, nBlue, nWhite);
		} else {
			SetLED4x(nPort, nLedIndex, nRed, nGreen, nBlue, nWhite);
		}
	}

#if defined (H3)
	bool IsUpdating() {
		if (m_tBoard == WS28XXMULTI_BOARD_8X) {
			return h3_spi_dma_tx_is_active();  // returns TRUE while DMA operation is active
		} else {
			return false;
		}
	}
#else
	bool IsUpdating(void) {
		return false;
	}
#endif

	void Update();
	void Blackout();

private:
	uint8_t ReverseBits(uint8_t nBits);
// 4x
	bool IsMCP23017();
	bool SetupMCP23017(uint8_t nT0H, uint8_t nT1H);
	bool SetupSI5351A();
	void SetupGPIO();
	void SetupBuffers4x();
	void Generate800kHz(const uint32_t *pBuffer);
	void SetLED4x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetLED4x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);
// 8x
	void SetupHC595(uint8_t nT0H, uint8_t nT1H);
	void SetupSPI();
	void SetupBuffers8x();
	void SetLED8x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetLED8x(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

private:
	WS28xxMultiBoard m_tBoard{WS28XXMULTI_BOARD_4X};
	TWS28XXType m_tWS28xxType{WS2812B};
	uint16_t m_nLedCount{170};
	TRGBMapping m_tRGBMapping{RGB_MAPPING_UNDEFINED};
	uint8_t m_nLowCode{0};
	uint8_t m_nHighCode{0};
	uint32_t m_nBufSize{0};
	uint32_t *m_pBuffer4x{nullptr};
	uint32_t *m_pBlackoutBuffer4x{nullptr};
	uint8_t *m_pBuffer8x{nullptr};
	uint8_t *m_pBlackoutBuffer8x{nullptr};
};

#endif /* WS28XXMULTI_H_ */
