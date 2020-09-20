/**
 * @file hub75bdisplay.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
/**
 * PoC
 */

#ifndef HUB75BDISPLAY_H_
#define HUB75BDISPLAY_H_

#include <stdint.h>

#include "lightset.h"

class Hub75bDisplay: public LightSet {
public:
	Hub75bDisplay(uint32_t nColumns, uint32_t nRows);

	void Start();
	void Run();

	void SetPixel(uint32_t nColumn, uint32_t nRow, uint8_t nRed, uint8_t nGreen, uint8_t nBlue);

	uint32_t GetFps();

	void Dump();

	// LightSet
	void Print();

	void Start(__attribute__((unused)) uint8_t nPort) {

	}
	void Stop(__attribute__((unused)) uint8_t nPort) {

	}
	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength);

	uint32_t GetUniverses() const {
		if (m_nLastPortId == 0) {
			return 0;
		}
		return 1 + m_nLastPortId;
	}

private:
	uint32_t m_nColumns;
	uint32_t m_nRows;
	uint32_t *m_pFramebuffer{0};
	uint8_t m_TablePWM[255];
	// LightSet
	uint32_t m_nLastPortId{0};
	uint32_t m_nLastPortDataLength{0};
};

#endif /* HUB75BDISPLAY_H_ */
