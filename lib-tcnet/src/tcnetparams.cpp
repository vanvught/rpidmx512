/**
 * @file tcnetparams.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "tcnetparams.h"
#include "tcnetparamsconst.h"
#include "tcnetconst.h"
#include "tcnet.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace tcnetparams;

TCNetParams::TCNetParams() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

	memset(m_Params.aNodeName, '\0', TCNET_NODE_NAME_LENGTH);
	m_Params.nLayer = static_cast<uint8_t>(tcnet::Layer::LAYER_M);
	m_Params.nTimeCodeType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);

	DEBUG_EXIT
}

void TCNetParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(TCNetParams::StaticCallbackFunction, this);

	if (configfile.Read(TCNetParamsConst::FILE_NAME)) {
		TCNetParamsStore::Update(&m_Params);
	} else
#endif
		TCNetParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void TCNetParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(TCNetParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	TCNetParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void TCNetParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char ch;
	uint8_t nValue8;

	uint32_t nLength = TCNET_NODE_NAME_LENGTH;
	if (Sscan::Char(pLine, TCNetParamsConst::NODE_NAME, m_Params.aNodeName, nLength) == Sscan::OK) {
		m_Params.nSetList |= Mask::NODE_NAME;
		return;
	}

	nLength = 1;
	ch = ' ';
	if (Sscan::Char(pLine, TCNetParamsConst::LAYER, &ch, nLength) == Sscan::OK) {
		const tcnet::Layer tLayer = TCNet::GetLayer(ch);

		if ((tLayer != tcnet::Layer::LAYER_UNDEFINED) && (tLayer != tcnet::Layer::LAYER_M)) {
			m_Params.nLayer = static_cast<uint8_t>(tLayer);
			m_Params.nSetList |= Mask::LAYER;
		} else {
			m_Params.nLayer = static_cast<uint8_t>(tcnet::Layer::LAYER_M);
			m_Params.nSetList &= ~Mask::LAYER;
		}
		return;
	}

	if (Sscan::Uint8(pLine, TCNetParamsConst::TIMECODE_TYPE, nValue8) == Sscan::OK) {
		switch (nValue8) {
		case 24:
			m_Params.nTimeCodeType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_FILM);
			m_Params.nSetList |= Mask::TIMECODE_TYPE;
			break;
		case 25:
			m_Params.nTimeCodeType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_EBU_25FPS);
			m_Params.nSetList |= Mask::TIMECODE_TYPE;
			break;
		case 29:
			m_Params.nTimeCodeType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_DF);
			m_Params.nSetList |= Mask::TIMECODE_TYPE;
			break;
		case 30:
			m_Params.nTimeCodeType = static_cast<uint8_t>(tcnet::TimeCodeType::TIMECODE_TYPE_SMPTE_30FPS);
			m_Params.nSetList &= ~Mask::TIMECODE_TYPE;
			break;
		default:
			m_Params.nSetList &= ~Mask::TIMECODE_TYPE;
			break;
		}

		return;
	}

	if (Sscan::Uint8(pLine, TCNetParamsConst::USE_TIMECODE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= Mask::USE_TIMECODE;
		} else {
			m_Params.nSetList &= ~Mask::USE_TIMECODE;
		}
		return;
	}
}

void TCNetParams::Builder(const struct Params *pTTCNetParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(m_Params.nTimeCodeType < (sizeof(TCNetConst::FPS) / sizeof(TCNetConst::FPS[0])));

	if (pTTCNetParams != nullptr) {
		memcpy(&m_Params, pTTCNetParams, sizeof(struct Params));
	} else {
		TCNetParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(TCNetParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(Mask::NODE_NAME)) {
		assert(TCNet::Get() != nullptr);
		strncpy(m_Params.aNodeName, TCNet::Get()->GetNodeName(), TCNET_NODE_NAME_LENGTH);
		m_Params.aNodeName[TCNET_NODE_NAME_LENGTH - 1] = '\0';
	}

	char aName[2];
	aName[0] = TCNet::GetLayerName(static_cast<tcnet::Layer>(m_Params.nLayer));
	aName[1] = '\0';

	builder.Add(TCNetParamsConst::NODE_NAME, m_Params.aNodeName, isMaskSet(Mask::NODE_NAME));
	builder.Add(TCNetParamsConst::LAYER, aName, isMaskSet(Mask::LAYER));
	builder.Add(TCNetParamsConst::TIMECODE_TYPE, TCNetConst::FPS[m_Params.nTimeCodeType], isMaskSet(Mask::TIMECODE_TYPE));
	builder.Add(TCNetParamsConst::USE_TIMECODE, isMaskSet(Mask::USE_TIMECODE), isMaskSet(Mask::USE_TIMECODE));

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);

	DEBUG_EXIT
}

void TCNetParams::Set(TCNet *pTCNet) {
	assert(pTCNet != nullptr);

	if (m_Params.nSetList == 0) {
		return;
	}

	if (isMaskSet(Mask::NODE_NAME)) {
		pTCNet->SetNodeName(reinterpret_cast<const char*>(m_Params.aNodeName));
	}

	if (isMaskSet(Mask::LAYER)) {
		pTCNet->SetLayer(static_cast<tcnet::Layer>(m_Params.nLayer));
	}

	if (isMaskSet(Mask::TIMECODE_TYPE)) {
		pTCNet->SetTimeCodeType(static_cast<tcnet::TimeCodeType>(m_Params.nTimeCodeType));
	}

	pTCNet->SetUseTimeCode(isMaskSet(Mask::USE_TIMECODE));
}

void TCNetParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<TCNetParams *>(p))->callbackFunction(s);
}

void TCNetParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, TCNetParamsConst::FILE_NAME);

	if (isMaskSet(Mask::NODE_NAME)) {
		printf(" %s=%s\n", TCNetParamsConst::NODE_NAME, m_Params.aNodeName);
	}

	if (isMaskSet(Mask::LAYER)) {
		printf(" %s=%d [Layer%c]\n", TCNetParamsConst::LAYER, m_Params.nLayer, TCNet::GetLayerName(static_cast<tcnet::Layer>(m_Params.nLayer)));
	}

	if (isMaskSet(Mask::TIMECODE_TYPE)) {
		printf(" %s=%d\n", TCNetParamsConst::TIMECODE_TYPE, m_Params.nTimeCodeType);
	}
}
