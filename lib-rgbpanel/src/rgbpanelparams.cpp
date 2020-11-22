/**
 * @file rgbpanelparams.cpp
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
#include <string.h>
#include <cassert>

#include "rgbpanelparams.h"
#include "rgbpanelparamsconst.h"
#include "rgbpanel.h"
#include "rgbpanelconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace rgbpanel;

RgbPanelParams::RgbPanelParams(RgbPanelParamsStore *pRgbPanelParamsStore): m_pRgbPanelParamsStore(pRgbPanelParamsStore) {
	m_tRgbPanelParams.nSetList = 0;
	m_tRgbPanelParams.nCols = defaults::COLS;
	m_tRgbPanelParams.nRows = defaults::ROWS;
	m_tRgbPanelParams.nChain = defaults::CHAIN;
	m_tRgbPanelParams.nType = static_cast<uint8_t>(defaults::TYPE);
}

bool RgbPanelParams::Load() {
	m_tRgbPanelParams.nSetList = 0;

	ReadConfigFile configfile(RgbPanelParams::staticCallbackFunction, this);

	if (configfile.Read(RgbPanelParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pRgbPanelParamsStore != nullptr) {
			m_pRgbPanelParamsStore->Update(&m_tRgbPanelParams);
		}
	} else if (m_pRgbPanelParamsStore != nullptr) {
		m_pRgbPanelParamsStore->Copy(&m_tRgbPanelParams);
	} else {
		return false;
	}

	return true;
}

void RgbPanelParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pRgbPanelParamsStore != nullptr);

	if (m_pRgbPanelParamsStore == nullptr) {
		return;
	}

	m_tRgbPanelParams.nSetList = 0;

	ReadConfigFile config(RgbPanelParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRgbPanelParamsStore->Update(&m_tRgbPanelParams);
}

void RgbPanelParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, RgbPanelParamsConst::COLS, nValue8) == Sscan::OK) {
		if ((m_tRgbPanelParams.nCols = RgbPanel::ValidateColumns(nValue8)) != defaults::COLS) {
			m_tRgbPanelParams.nSetList |= RgbPanelParamsMask::COLS;
		} else {
			m_tRgbPanelParams.nSetList &= ~RgbPanelParamsMask::COLS;
		}
		return;
	}


	if (Sscan::Uint8(pLine, RgbPanelParamsConst::ROWS, nValue8) == Sscan::OK) {
		if ((m_tRgbPanelParams.nRows = RgbPanel::ValidateRows(nValue8)) != defaults::ROWS) {
			m_tRgbPanelParams.nSetList |= RgbPanelParamsMask::ROWS;
		} else {
			m_tRgbPanelParams.nSetList &= ~RgbPanelParamsMask::ROWS;
		}
		return;
	}

	if (Sscan::Uint8(pLine, RgbPanelParamsConst::CHAIN, nValue8) == Sscan::OK) {
		if ((nValue8 != 0) && (nValue8 != static_cast<uint8_t>(defaults::CHAIN))) {
			m_tRgbPanelParams.nType = nValue8;
			m_tRgbPanelParams.nSetList |= RgbPanelParamsMask::CHAIN;
		} else {
			m_tRgbPanelParams.nType = static_cast<uint8_t>(defaults::CHAIN);
			m_tRgbPanelParams.nSetList &= ~RgbPanelParamsMask::CHAIN;
		}
		return;
	}

	char cBuffer[type::MAX_NAME_LENGTH];
	uint32_t nLength = sizeof(cBuffer) - 1;

	if (Sscan::Char(pLine, RgbPanelParamsConst::TYPE, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		if ((m_tRgbPanelParams.nType = static_cast<uint8_t>(RgbPanel::GetType(cBuffer))) != static_cast<uint8_t>(defaults::TYPE)) {
			m_tRgbPanelParams.nSetList |= RgbPanelParamsMask::TYPE;
		} else {
			m_tRgbPanelParams.nSetList &= ~RgbPanelParamsMask::TYPE;
		}
		return;
	}
}

void RgbPanelParams::Builder(const struct TRgbPanelParams *pRgbPanelParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (pRgbPanelParams != nullptr) {
		memcpy(&m_tRgbPanelParams, pRgbPanelParams, sizeof(struct TRgbPanelParams));
	} else {
		m_pRgbPanelParamsStore->Copy(&m_tRgbPanelParams);
	}

	PropertiesBuilder builder(RgbPanelParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(RgbPanelParamsConst::COLS, m_tRgbPanelParams.nCols, isMaskSet(RgbPanelParamsMask::COLS));
	builder.Add(RgbPanelParamsConst::ROWS, m_tRgbPanelParams.nRows, isMaskSet(RgbPanelParamsMask::ROWS));
	builder.Add(RgbPanelParamsConst::CHAIN, m_tRgbPanelParams.nChain, isMaskSet(RgbPanelParamsMask::CHAIN));
	builder.Add(RgbPanelParamsConst::TYPE, RgbPanel::GetType(static_cast<Types>(m_tRgbPanelParams.nType)), isMaskSet(RgbPanelParamsMask::TYPE));

	nSize = builder.GetSize();
}

void RgbPanelParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	if (m_pRgbPanelParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void RgbPanelParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RgbPanelParams *>(p))->callbackFunction(s);
}
