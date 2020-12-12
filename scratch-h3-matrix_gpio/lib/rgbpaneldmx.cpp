/**
 * @file rgbpaneldmx.cpp
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

#include <stdint.h>
#include <algorithm>

#include "rgbpaneldmx.h"

#include "debug.h"

using namespace rgbpanel;

RgbPanelDmx::RgbPanelDmx(uint32_t nColumns, uint32_t nRows, uint32_t nChain, Types tType): RgbPanel(nColumns, nRows, nChain, tType) {
	m_nLastPortId = (m_nColumns * m_nRows) / 170;
	m_nLastPortDataLength = 3 * ((m_nColumns * m_nRows) - (m_nLastPortId * 170));
}

void RgbPanelDmx::Print() {
	RgbPanel::Print();

	printf(" Universes : 1 to %u-%u\n", 1 + m_nLastPortId, m_nLastPortDataLength);
}

void RgbPanelDmx::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength) {
	if (nPort < m_nLastPortId) {
		nLength = std::min(nLength, static_cast<uint16_t>(510));
	} else {
		nLength = std::min(nLength, static_cast<uint16_t>(m_nLastPortDataLength));
	}

	uint32_t nIndex = 0;

	for (uint32_t i = 0; i < nLength; i = i + 3) {

		const uint32_t nPixelIndex = (nPort * 170U) + nIndex++;

		const uint32_t nRow = nPixelIndex / m_nColumns;
		const uint32_t nColumn = nPixelIndex - (nRow * m_nColumns);

		SetPixel(nColumn, nRow, pData[i], pData[i + 1], pData[i + 2]);
	}

	if (nPort == m_nLastPortId) {
		Show();
	}
}
