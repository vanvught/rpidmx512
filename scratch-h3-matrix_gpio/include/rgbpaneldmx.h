/**
 * @file rgbpaneldmx.h
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

#ifndef RGBPANELDMX_H_
#define RGBPANELDMX_H_

#include <stdint.h>

#include "rgbpanel.h"
#include "rgbpanelconst.h"

#include "lightset.h"

class RgbPanelDmx: public RgbPanel, public LightSet {
public:
	RgbPanelDmx(uint32_t nColumns, uint32_t nRows, uint32_t nChain = rgbpanel::defaults::CHAIN, RgbPanelTypes type = rgbpanel::defaults::TYPE);

	void Start(__attribute__((unused)) uint8_t nPort = 0) {
		RgbPanel::Start();
	}
	void Stop(__attribute__((unused)) uint8_t nPort = 0) {
		RgbPanel::Stop();
	}

	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength);

	uint32_t GetUniverses() const {
		if (m_nLastPortId == 0) {
			return 0;
		}
		return 1 + m_nLastPortId;
	}

	void Print();

private:
	uint32_t m_nLastPortId{0};
	uint32_t m_nLastPortDataLength{0};
};

#endif /* RGBPANELDMX_H_ */
