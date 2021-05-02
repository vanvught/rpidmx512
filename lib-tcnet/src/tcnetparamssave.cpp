/**
 * @file tcnetparamssave.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "tcnetparams.h"
#include "tcnetparamsconst.h"
#include "tcnetconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

void TCNetParams::Builder(const struct TTCNetParams *pTTCNetParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(m_tTTCNetParams.nTimeCodeType < (sizeof(TCNetConst::FPS) / sizeof(TCNetConst::FPS[0])));

	if (pTTCNetParams != nullptr) {
		memcpy(&m_tTTCNetParams, pTTCNetParams, sizeof(struct TTCNetParams));
	} else {
		m_pTCNetParamsStore->Copy(&m_tTTCNetParams);
	}

	char aName[2];
	aName[0] = TCNet::GetLayerName(static_cast<TCNetLayer>(m_tTTCNetParams.nLayer));
	aName[1] = '\0';

	PropertiesBuilder builder(TCNetParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(TCNetParamsConst::NODE_NAME, m_tTTCNetParams.aNodeName, isMaskSet(TCNetParamsMask::NODE_NAME));
	builder.Add(TCNetParamsConst::LAYER, aName, isMaskSet(TCNetParamsMask::LAYER));
	builder.Add(TCNetParamsConst::TIMECODE_TYPE, TCNetConst::FPS[m_tTTCNetParams.nTimeCodeType], isMaskSet(TCNetParamsMask::TIMECODE_TYPE));
	builder.Add(TCNetParamsConst::USE_TIMECODE, m_tTTCNetParams.nUseTimeCode, isMaskSet(TCNetParamsMask::USE_TIMECODE));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);

	DEBUG_EXIT
}

void TCNetParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pTCNetParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
