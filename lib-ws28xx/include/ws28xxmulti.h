/**
 * @file ws28xxmulti.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

enum TWS28xxMultiType {
	WS28XXMULTI_WS2801_NOT_SUPPORTED = 0,
	WS28XXMULTI_WS2811,
	WS28XXMULTI_WS2812,
	WS28XXMULTI_WS2812B,
	WS28XXMULTI_WS2813,
	WS28XXMULTI_WS2815,
	WS28XXMULTI_SK6812,
	WS28XXMULTI_SK6812W,
	WS28XXMULTI_APA102_NOT_SUPPORTED,
	WS28XXMULTI_UCS1903,
	WS28XXMULTI_UCS2903
};

enum WS28xxMultiActivePorts {
	WS28XXMULTI_ACTIVE_PORTS_MAX = 4
};

class WS28xxMulti {
public:
	WS28xxMulti(TWS28xxMultiType tWS28xxMultiType, uint16_t nLedCount, uint8_t nActiveOutputs, uint8_t nT0H = 0, uint8_t nT1H = 0, bool bUseSI5351A = false);
	~WS28xxMulti(void);

	TWS28xxMultiType GetLEDType(void) {
		return m_tWS28xxMultiType;
	}

	uint16_t GetLEDCount(void) {
		return m_nLedCount;
	}

	uint8_t GetActiveOutputs(void) {
		return m_nActiveOutputs;
	}

	uint8_t GetLowCode(void) {
		return m_nLowCode;
	}

	uint8_t GetHighCode(void) {
		return m_nHighCode;
	}

	void SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetLED(uint8_t nPort, uint16_t nLedIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	void Update(void);
	void Blackout(void);

private:
	uint8_t CalculateBits(uint8_t nNanoSeconds);
	uint8_t ReverseBits(uint8_t nBits);
	bool SetupSI5351A(void);
	bool SetupMCP23017(uint8_t nT0H, uint8_t nT1H);
	void Generate800kHz(const uint32_t *pBuffer);

private:
	TWS28xxMultiType m_tWS28xxMultiType;
	uint16_t m_nLedCount;
	uint8_t m_nActiveOutputs;
	uint8_t m_nLowCode;
	uint8_t m_nHighCode;
	uint32_t m_nBufSize;
	uint32_t *m_pBuffer;
	uint32_t *m_pBlackoutBuffer;
};

#endif /* WS28XXMULTI_H_ */
