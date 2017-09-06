/**
 * @file ws28xxstripe.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WS28XXSTRIPE_H_
#define WS28XXSTRIPE_H_

#include <stdint.h>

#define WS2801_SPI_SPEED_MAX_HZ		25000000	///< 25 MHz
#define WS2801_SPI_SPEED_DEFAULT_HZ	4000000		///< 4 MHz

enum TWS28XXType {
	WS2801 = 0,
	WS2811,
	WS2812,
	WS2812B,
	WS2813,
	SK6812,
	SK6812W
} ;

class WS28XXStripe {
public:
	WS28XXStripe(const uint16_t nLEDCount, const TWS28XXType Type, const uint32_t nClockSpeed);
	~WS28XXStripe(void);

	unsigned GetLEDCount(void) const;
	TWS28XXType GetLEDType(void) const;

	void SetLED(unsigned nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);	// nIndex is 0-based
	void SetLED(unsigned nLEDIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);	// nIndex is 0-based

	void Update(void);
	void Blackout(void);

private:
	void SetColorWS28xx(unsigned nOffset, uint8_t nValue);

private:
	TWS28XXType	m_Type;
	unsigned	m_nLEDCount;
	unsigned	m_nBufSize;
	uint8_t		*m_pBuffer;
	uint8_t		*m_pBlackoutBuffer;
	uint8_t		m_nHighCode;
};



#endif /* WS28XXSTRIPE_H_ */
