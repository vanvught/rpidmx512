/**
 * @file rgbpanelparamsdump.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <stdio.h>

#include "rgbpanelparams.h"
#include "rgbpanelparamsconst.h"
#include "rgbpanel.h"

using namespace rgbpanel;

void RgbPanelParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RgbPanelParamsConst::FILE_NAME);

	if (isMaskSet(RgbPanelParamsMask::COLS)) {
		printf(" %s=%d\n", RgbPanelParamsConst::COLS, m_tRgbPanelParams.nCols);
	}

	if (isMaskSet(RgbPanelParamsMask::ROWS)) {
		printf(" %s=%d\n", RgbPanelParamsConst::ROWS, m_tRgbPanelParams.nRows);
	}

	if (isMaskSet(RgbPanelParamsMask::CHAIN)) {
		printf(" %s=%d\n", RgbPanelParamsConst::CHAIN, m_tRgbPanelParams.nChain);
	}

	if (isMaskSet(RgbPanelParamsMask::TYPE)) {
		printf(" %s=%d [%s]\n", RgbPanelParamsConst::TYPE, m_tRgbPanelParams.nType, RgbPanel::GetType(static_cast<Types>(m_tRgbPanelParams.nType)));
	}
#endif
}
