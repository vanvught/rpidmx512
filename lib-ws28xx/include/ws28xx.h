/**
 * @file ws28xx.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>

#if defined (__circle__)
 #include <circle/interrupt.h>
 #include <circle/spimasterdma.h>
#endif

enum TWS28XXType {
	WS2801 = 0,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	WS2815,
	SK6812,
	SK6812W,
	APA102,
	UCS1903,
	UCS2903
};

#define WS2801_SPI_SPEED_MAX_HZ		25000000	///< 25 MHz
#define WS2801_SPI_SPEED_DEFAULT_HZ	4000000		///< 4 MHz

class WS28xx {
public:
#if defined (__circle__)
	WS28xx (CInterruptSystem *pInterruptSystem, TWS28XXType Type, uint16_t nLEDCount, uint32_t nClockSpeed = WS2801_SPI_SPEED_DEFAULT_HZ);
#else
	WS28xx(TWS28XXType Type, uint16_t nLEDCount, uint32_t nClockSpeed = WS2801_SPI_SPEED_DEFAULT_HZ);
#endif
	~WS28xx(void);

#if defined (__circle__)
	bool Initialize (void);
#else
	bool Initialize (void) {
		return true;
	}
#endif

	uint16_t GetLEDCount(void) {
		return m_nLEDCount;
	}

	TWS28XXType GetLEDType(void) {
		return m_tLEDType;
	}

	uint32_t GetClockSpeedHz(void) {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(uint8_t nGlobalBrightness);

	uint8_t GetGlobalBrightness(void) {
		return m_nGlobalBrightness;
	}

	void SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetLED(uint32_t nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	void Update(void);
	void Blackout(void);

#if defined (__circle__)
	bool IsUpdating (void) const; // returns TRUE while DMA operation is active
#else
	 	bool IsUpdating (void) const {
		return false;
	}
#endif

private:
	void SetColorWS28xx(uint32_t nOffset, uint8_t nValue);

#if defined (__circle__)
private:
	void SPICompletionRoutine (boolean bStatus);
	static void SPICompletionStub (boolean bStatus, void *pParam);
#endif

private:
	TWS28XXType m_tLEDType;
	uint16_t m_nLEDCount;
	uint32_t m_nClockSpeedHz;
	uint8_t m_nGlobalBrightness;
	uint32_t m_nBufSize;
	uint8_t *m_pBuffer;
	uint8_t *m_pBlackoutBuffer;
	volatile bool m_bUpdating;
	uint8_t m_nHighCode;
#if defined (__circle__)
	uint8_t *m_pReadBuffer;
	CSPIMasterDMA m_SPIMaster;
#endif
};

#endif /* WS28XX_H_ */
