/**
 * @file tcnetparamssave.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "tcnetparams.h"
#include "tcnetparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

const uint8_t s_nFPS[4] = { 24, 25, 29, 30};

void TCNetParams::Builder(const struct TTCNetParams *pTTCNetParams, uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);
	assert(m_tTTCNetParams.nTimeCodeType < (sizeof(s_nFPS) / sizeof(s_nFPS[0])));

	if (pTTCNetParams != 0) {
		memcpy(&m_tTTCNetParams, pTTCNetParams, sizeof(struct TTCNetParams));
	} else {
		m_pTCNetParamsStore->Copy(&m_tTTCNetParams);
	}

	char name[2];
	name[0] = TCNet::GetLayerName((TTCNetLayers) m_tTTCNetParams.nLayer);
	name[1] = '\0';

	PropertiesBuilder builder(TCNetParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(TCNetParamsConst::NODE_NAME, (const char *)m_tTTCNetParams.aNodeName, isMaskSet(TCNET_PARAMS_MASK_NODE_NAME));
	builder.Add(TCNetParamsConst::LAYER, (const char *)name, isMaskSet(TCNET_PARAMS_MASK_LAYER));
	builder.Add(TCNetParamsConst::TIMECODE_TYPE, s_nFPS[m_tTTCNetParams.nTimeCodeType], isMaskSet(TCNET_PARAMS_MASK_TIMECODE_TYPE));
	builder.Add(TCNetParamsConst::USE_TIMECODE, m_tTTCNetParams.nUseTimeCode, isMaskSet(TCNET_PARAMS_MASK_USE_TIMECODE));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);

	DEBUG_EXIT
}

void TCNetParams::Save(uint8_t *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pTCNetParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(0, pBuffer, nLength, nSize);
}
