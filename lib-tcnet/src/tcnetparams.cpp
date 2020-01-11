/**
 * @file tcnetparams.cpp
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
#include <ctype.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "tcnetparams.h"
#include "tcnetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

TCNetParams::TCNetParams(TCNetParamsStore* pTCNetParamsStore): m_pTCNetParamsStore(pTCNetParamsStore) {
	m_tTTCNetParams.nSetList = 0;

	memset(m_tTTCNetParams.aNodeName, '\0', TCNET_NODE_NAME_LENGTH);
	m_tTTCNetParams.nLayer = TCNET_LAYER_M;
	m_tTTCNetParams.nTimeCodeType = TCNET_TIMECODE_TYPE_SMPTE_30FPS;
	m_tTTCNetParams.nUseTimeCode = 0;
}

TCNetParams::~TCNetParams(void) {
	m_tTTCNetParams.nSetList = 0;
}

bool TCNetParams::Load(void) {
	m_tTTCNetParams.nSetList = 0;

	ReadConfigFile configfile(TCNetParams::staticCallbackFunction, this);

	if (configfile.Read(TCNetParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pTCNetParamsStore != 0) {
			m_pTCNetParamsStore->Update(&m_tTTCNetParams);
		}
	} else if (m_pTCNetParamsStore != 0) {
		m_pTCNetParamsStore->Copy(&m_tTTCNetParams);
	} else {
		return false;
	}

	return true;
}

void TCNetParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pTCNetParamsStore != 0);

	if (m_pTCNetParamsStore == 0) {
		return;
	}

	m_tTTCNetParams.nSetList = 0;

	ReadConfigFile config(TCNetParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pTCNetParamsStore->Update(&m_tTTCNetParams);
}

void TCNetParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t len;
	char ch;
	uint8_t uint8;

	len = TCNET_NODE_NAME_LENGTH;
	if (Sscan::Char(pLine, TCNetParamsConst::NODE_NAME, (char *) m_tTTCNetParams.aNodeName, &len) == SSCAN_OK) {
		m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_NODE_NAME;
		return;
	}

	len = 1;
	ch = ' ';
	if (Sscan::Char(pLine, TCNetParamsConst::LAYER, &ch, &len) == SSCAN_OK) {
		m_tTTCNetParams.nLayer = (uint8_t) TCNet::GetLayer((uint8_t) ch);

		if (m_tTTCNetParams.nLayer != TCNET_LAYER_UNDEFINED) {
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_LAYER;
		} else {
			m_tTTCNetParams.nLayer = TCNET_LAYER_M;
			m_tTTCNetParams.nSetList &= ~TCNET_PARAMS_MASK_LAYER;
		}
		return;
	}

	if (Sscan::Uint8(pLine, TCNetParamsConst::TIMECODE_TYPE, &uint8) == SSCAN_OK) {
		switch (uint8) {
		case 24:
			m_tTTCNetParams.nTimeCodeType = TCNET_TIMECODE_TYPE_FILM;
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_TIMECODE_TYPE;
			break;
		case 25:
			m_tTTCNetParams.nTimeCodeType = TCNET_TIMECODE_TYPE_EBU_25FPS;
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_TIMECODE_TYPE;
			break;
		case 29:
			m_tTTCNetParams.nTimeCodeType = TCNET_TIMECODE_TYPE_DF;
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_TIMECODE_TYPE;
			break;
		case 30:
			m_tTTCNetParams.nTimeCodeType = TCNET_TIMECODE_TYPE_SMPTE_30FPS;
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_TIMECODE_TYPE;
			break;
		default:
			m_tTTCNetParams.nSetList &= ~TCNET_PARAMS_MASK_TIMECODE_TYPE;
			break;
		}

		return;
	}

	if (Sscan::Uint8(pLine, TCNetParamsConst::USE_TIMECODE, &uint8) == SSCAN_OK) {
		if (uint8 != 0) {
			m_tTTCNetParams.nUseTimeCode = 1;
			m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_USE_TIMECODE;
		} else {
			m_tTTCNetParams.nUseTimeCode = 0;
			m_tTTCNetParams.nSetList &= ~TCNET_PARAMS_MASK_USE_TIMECODE;
		}

		return;
	}
}

void TCNetParams::Set(TCNet* pTCNet) {
	assert(pTCNet != 0);

	if (m_tTTCNetParams.nSetList == 0) {
		return;
	}

	if(isMaskSet(TCNET_PARAMS_MASK_NODE_NAME)) {
		pTCNet->SetNodeName(m_tTTCNetParams.aNodeName);
	}

	if(isMaskSet(TCNET_PARAMS_MASK_LAYER)) {
		pTCNet->SetLayer((TTCNetLayers) m_tTTCNetParams.nLayer);
	}

	if(isMaskSet(TCNET_PARAMS_MASK_TIMECODE_TYPE)) {
		pTCNet->SetTimeCodeType((TTCNetTimeCodeType) m_tTTCNetParams.nTimeCodeType);
	}

	if(isMaskSet(TCNET_PARAMS_MASK_USE_TIMECODE)) {
		pTCNet->SetUseTimeCode(m_tTTCNetParams.nUseTimeCode != 0);
	}
}

void TCNetParams::Dump(void) {
#ifndef NDEBUG
	if (m_tTTCNetParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, TCNetParamsConst::FILE_NAME);

	if(isMaskSet(TCNET_PARAMS_MASK_NODE_NAME)) {
		printf(" %s=%s\n", TCNetParamsConst::NODE_NAME, m_tTTCNetParams.aNodeName);
	}

	if(isMaskSet(TCNET_PARAMS_MASK_LAYER)) {
		printf(" %s=%d [Layer%c]\n", TCNetParamsConst::LAYER, m_tTTCNetParams.nLayer, TCNet::GetLayerName((TTCNetLayers)m_tTTCNetParams.nLayer));
	}

	if(isMaskSet(TCNET_PARAMS_MASK_TIMECODE_TYPE)) {
		printf(" %s=%d\n", TCNetParamsConst::TIMECODE_TYPE, m_tTTCNetParams.nTimeCodeType);
	}
#endif
}

void TCNetParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != 0);
	assert(s != 0);

	((TCNetParams *) p)->callbackFunction(s);
}
