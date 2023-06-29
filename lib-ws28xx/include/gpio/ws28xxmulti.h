/**
 * @file ws28xxmulti.h
 *
 */
/* Copyright (C) 2021-2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GPIO_WS28XXMULTI_H_
#define GPIO_WS28XXMULTI_H_

#include <cstdint>

#include "pixelconfiguration.h"

class WS28xxMulti {
public:
	WS28xxMulti(PixelConfiguration& pixelConfiguration);
	~WS28xxMulti();

	void Print();

	void SetPixel(const uint32_t nPortIndex, const uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);
	void SetPixel(const uint32_t nPortIndex, const uint32_t nPixelIndex, uint8_t nRed, uint8_t nGreen, uint8_t nBlue, uint8_t nWhite);

	bool IsUpdating();

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

	static WS28xxMulti *Get() {
		return s_pThis;
	}

private:
	void Setup(uint8_t nLowCode, uint8_t nHighCode);
	void Setup(uint32_t nFrequency);
	void SetColourRTZ(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3);
	void SetColourWS2801(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3);
	void SetPixel4Bytes(const uint32_t nPortIndex, const uint32_t nPixelIndex, const uint8_t nCtrl, const uint8_t nColour1, const uint8_t nColour2, const uint8_t nColour3);

private:
	PixelConfiguration m_PixelConfiguration;
	uint32_t m_nBufSize { 0 };

	static WS28xxMulti *s_pThis;
};

#endif /* GPIO_WS28XXMULTI_H_ */
