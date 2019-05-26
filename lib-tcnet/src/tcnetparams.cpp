/**
 * @file tcnetparams.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <ctype.h>
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

void TCNetParams::Load(const char* pBuffer, uint32_t nLength) {
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

bool TCNetParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pTCNetParamsStore == 0) {
		nSize = 0;
		return false;
	}

	char name[2];
	name[0] = TCNet::GetLayerName((TTCNetLayers) m_tTTCNetParams.nLayer);
	name[1] = '\0';

	m_pTCNetParamsStore->Copy(&m_tTTCNetParams);

	PropertiesBuilder builder(TCNetParamsConst::FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(TCNetParamsConst::NODE_NAME, (const char *)m_tTTCNetParams.aNodeName, isMaskSet(TCNET_PARAMS_MASK_NODE_NAME));
	isAdded &= builder.Add(TCNetParamsConst::LAYER, (const char *)name, isMaskSet(TCNET_PARAMS_MASK_LAYER));

	nSize = builder.GetSize();

	return isAdded;
}

void TCNetParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t len;
	char ch;

	len = TCNET_NODE_NAME_LENGTH;
	if (Sscan::Char(pLine, TCNetParamsConst::NODE_NAME, (char *) m_tTTCNetParams.aNodeName, &len) == SSCAN_OK) {
		m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_NODE_NAME;
		return;
	}

	len = 1;
	ch = ' ';
	if (Sscan::Char(pLine, TCNetParamsConst::LAYER, &ch, &len) == SSCAN_OK) {
		m_tTTCNetParams.nLayer = (uint8_t) GetLayer((uint8_t) ch);
		m_tTTCNetParams.nSetList |= TCNET_PARAMS_MASK_LAYER;
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
#endif
}

TTCNetLayers TCNetParams::GetLayer(uint8_t nChar) {
	switch (toupper((int) nChar)) {
	case '1':
	case '2':
	case '3':
	case '4':
		return (TTCNetLayers) (nChar - (uint8_t) '1');
		break;
	case 'A':
		return TCNET_LAYER_A;
		break;
	case 'B':
		return TCNET_LAYER_B;
		break;
	case 'M':
		return TCNET_LAYER_M;
		break;
	case 'C':
		return TCNET_LAYER_C;
		break;
	default:
		break;
	}

	return TCNET_LAYER_UNDEFINED;
}

void TCNetParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((TCNetParams *) p)->callbackFunction(s);
}

bool TCNetParams::isMaskSet(uint32_t nMask) const {
	return (m_tTTCNetParams.nSetList & nMask) == nMask;
}

