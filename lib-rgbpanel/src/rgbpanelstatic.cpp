/**
 * @file rgbpanelstatic.cpp
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

#include <cassert>
#include <string.h>

#include "rgbpanel.h"
#include "rgbpanelconst.h"

using namespace rgbpanel;

uint32_t RgbPanel::ValidateColumns(uint32_t nColumns) {
	for (uint32_t i = 0; i < config::COLS; i++) {
		if (nColumns == RgbPanelConst::COLS[i]) {
			return nColumns;
		}
	}

	return defaults::COLS;
}

uint32_t RgbPanel::ValidateRows(uint32_t nRows) {
	for (uint32_t i = 0; i < config::ROWS; i++) {
		if (nRows == RgbPanelConst::ROWS[i]) {
			return nRows;
		}
	}

	return defaults::ROWS;
}

Types RgbPanel::GetType(const char *pType) {
	assert(pType != nullptr);

	for (uint32_t i = 0; i < static_cast<uint32_t>(Types::UNDEFINED); i++) {
		if (strcasecmp(pType, RgbPanelConst::TYPE[i]) == 0) {
			return static_cast<Types>(i);
		}
	}

	return defaults::TYPE;
}

const char* RgbPanel::GetType(Types tType) {
	if (tType < Types::UNDEFINED) {
		return RgbPanelConst::TYPE[static_cast<uint32_t>(tType)];
	}

	return RgbPanelConst::TYPE[static_cast<uint32_t>(defaults::TYPE)];
}
