/**
 * @file rgbpanelparams.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(__GNUC__) && !defined(__clang__)	
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
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

namespace rgbpanelparams::store {
static void Update(const struct rgbpanelparams::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::RGBPANEL, pParams, sizeof(struct rgbpanelparams::Params));
}

static void Copy(struct rgbpanelparams::Params *pRgbPanelParamss) {
	ConfigStore::Get()->Copy(configstore::Store::RGBPANEL, pRgbPanelParamss, sizeof(struct rgbpanelparams::Params));
}
}  // namespace rgbpanelparams::store

RgbPanelParams::RgbPanelParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;
	m_Params.nCols = defaults::COLS;
	m_Params.nRows = defaults::ROWS;
	m_Params.nChain = defaults::CHAIN;
	m_Params.nType = static_cast<uint8_t>(defaults::TYPE);

	DEBUG_EXIT
}

void RgbPanelParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(RgbPanelParams::StaticCallbackFunction, this);

	if (configfile.Read(RgbPanelParamsConst::FILE_NAME)) {
		rgbpanelparams::store::Update(&m_Params);
	} else
#endif
		rgbpanelparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RgbPanelParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(RgbPanelParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	rgbpanelparams::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RgbPanelParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, RgbPanelParamsConst::COLS, nValue8) == Sscan::OK) {
		if ((m_Params.nCols = static_cast<uint8_t>(RgbPanel::ValidateColumns(nValue8))) != defaults::COLS) {
			m_Params.nSetList |= rgbpanelparams::Mask::COLS;
		} else {
			m_Params.nSetList &= ~rgbpanelparams::Mask::COLS;
		}
		return;
	}


	if (Sscan::Uint8(pLine, RgbPanelParamsConst::ROWS, nValue8) == Sscan::OK) {
		if ((m_Params.nRows = static_cast<uint8_t>(RgbPanel::ValidateRows(nValue8))) != defaults::ROWS) {
			m_Params.nSetList |= rgbpanelparams::Mask::ROWS;
		} else {
			m_Params.nSetList &= ~rgbpanelparams::Mask::ROWS;
		}
		return;
	}

	if (Sscan::Uint8(pLine, RgbPanelParamsConst::CHAIN, nValue8) == Sscan::OK) {
		if ((nValue8 != 0) && (nValue8 != static_cast<uint8_t>(defaults::CHAIN))) {
			m_Params.nType = nValue8;
			m_Params.nSetList |= rgbpanelparams::Mask::CHAIN;
		} else {
			m_Params.nType = static_cast<uint8_t>(defaults::CHAIN);
			m_Params.nSetList &= ~rgbpanelparams::Mask::CHAIN;
		}
		return;
	}

	char cBuffer[type::MAX_NAME_LENGTH];
	uint32_t nLength = sizeof(cBuffer) - 1;

	if (Sscan::Char(pLine, RgbPanelParamsConst::TYPE, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		if ((m_Params.nType = static_cast<uint8_t>(RgbPanel::GetType(cBuffer))) != static_cast<uint8_t>(defaults::TYPE)) {
			m_Params.nSetList |= rgbpanelparams::Mask::TYPE;
		} else {
			m_Params.nSetList &= ~rgbpanelparams::Mask::TYPE;
		}
		return;
	}
}

void RgbPanelParams::Builder(const struct rgbpanelparams::Params *pRgbPanelParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (pRgbPanelParams != nullptr) {
		memcpy(&m_Params, pRgbPanelParams, sizeof(struct rgbpanelparams::Params));
	} else {
		rgbpanelparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(RgbPanelParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(RgbPanelParamsConst::COLS, m_Params.nCols, IsMaskSet(rgbpanelparams::Mask::COLS));
	builder.Add(RgbPanelParamsConst::ROWS, m_Params.nRows, IsMaskSet(rgbpanelparams::Mask::ROWS));
	builder.Add(RgbPanelParamsConst::CHAIN, m_Params.nChain, IsMaskSet(rgbpanelparams::Mask::CHAIN));
	builder.Add(RgbPanelParamsConst::TYPE, RgbPanel::GetType(static_cast<Types>(m_Params.nType)), IsMaskSet(rgbpanelparams::Mask::TYPE));

	nSize = builder.GetSize();
}

void RgbPanelParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RgbPanelParams *>(p))->CallbackFunction(s);
}

void RgbPanelParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RgbPanelParamsConst::FILE_NAME);

	if (IsMaskSet(rgbpanelparams::Mask::COLS)) {
		printf(" %s=%d\n", RgbPanelParamsConst::COLS, m_Params.nCols);
	}

	if (IsMaskSet(rgbpanelparams::Mask::ROWS)) {
		printf(" %s=%d\n", RgbPanelParamsConst::ROWS, m_Params.nRows);
	}

	if (IsMaskSet(rgbpanelparams::Mask::CHAIN)) {
		printf(" %s=%d\n", RgbPanelParamsConst::CHAIN, m_Params.nChain);
	}

	if (IsMaskSet(rgbpanelparams::Mask::TYPE)) {
		printf(" %s=%d [%s]\n", RgbPanelParamsConst::TYPE, m_Params.nType, RgbPanel::GetType(static_cast<Types>(m_Params.nType)));
	}
}
