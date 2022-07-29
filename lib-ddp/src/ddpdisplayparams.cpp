/**
 * @file ddpdisplayparams.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <ddpdisplaypixelconfiguration.h>
#include <cstdint>
#include <cstring>
#include <cassert>

#include "ddpdisplayparams.h"
#include "ddpdisplayparamsconst.h"
#include "pixeltype.h"
#include "pixelpatterns.h"
#include "devicesparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "debug.h"

DdpDisplayParams::DdpDisplayParams(DdpDisplayParamsStore *pDdpDisplayParamsStore): m_pDdpDisplayParamsStore(pDdpDisplayParamsStore) {
	memset(&m_tTDdpDisplayParams, 0, sizeof(struct TDdpDisplayParams));
	m_tTDdpDisplayParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
	m_tTDdpDisplayParams.nMap = static_cast<uint8_t>(pixel::Map::UNDEFINED);
}

bool DdpDisplayParams::Load() {
	m_tTDdpDisplayParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(DdpDisplayParams::staticCallbackFunction, this);

	if (configfile.Read(DdpDisplayParamsConst::FILE_NAME)) {
		if (m_pDdpDisplayParamsStore != nullptr) {
			m_pDdpDisplayParamsStore->Update(&m_tTDdpDisplayParams);
		}
	} else
#endif
	if (m_pDdpDisplayParamsStore != nullptr) {
		m_pDdpDisplayParamsStore->Copy(&m_tTDdpDisplayParams);
	} else {
		return false;
	}

	return true;
}

void DdpDisplayParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pDdpDisplayParamsStore != nullptr);

	if (m_pDdpDisplayParamsStore == nullptr) {
		return;
	}

	m_tTDdpDisplayParams.nSetList = 0;

	ReadConfigFile config(DdpDisplayParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pDdpDisplayParamsStore->Update(&m_tTDdpDisplayParams);
}

void DdpDisplayParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char cBuffer[16];

	uint32_t nLength = pixel::TYPES_MAX_NAME_LENGTH;

	if (Sscan::Char(pLine, DevicesParamsConst::TYPE, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		const auto type = PixelType::GetType(cBuffer);

		if (type != pixel::Type::UNDEFINED) {
			m_tTDdpDisplayParams.nType = static_cast<uint8_t>(type);
			m_tTDdpDisplayParams.nSetList |= DdpDisplayParamsMask::TYPE;
		} else {
			m_tTDdpDisplayParams.nType = static_cast<uint8_t>(pixel::defaults::TYPE);
			m_tTDdpDisplayParams.nSetList &= ~DdpDisplayParamsMask::TYPE;
		}
		return;
	}

	nLength = 3;

	if (Sscan::Char(pLine, DevicesParamsConst::MAP, cBuffer, nLength) == Sscan::OK) {
		cBuffer[nLength] = '\0';
		const auto map = PixelType::GetMap(cBuffer);

		if (map != pixel::Map::UNDEFINED) {
			m_tTDdpDisplayParams.nSetList |= DdpDisplayParamsMask::MAP;
		} else {
			m_tTDdpDisplayParams.nSetList &= ~DdpDisplayParamsMask::MAP;
		}

		m_tTDdpDisplayParams.nMap = static_cast<uint8_t>(map);
		return;
	}

	float fValue;
	uint8_t nValue8;

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T0H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_tTDdpDisplayParams.nSetList |= DdpDisplayParamsMask::LOW_CODE;
		} else {
			m_tTDdpDisplayParams.nSetList &= ~DdpDisplayParamsMask::LOW_CODE;
		}

		m_tTDdpDisplayParams.nLowCode = nValue8;
		return;
	}

	if (Sscan::Float(pLine, DevicesParamsConst::LED_T1H, fValue) == Sscan::OK) {
		if ((nValue8 = PixelType::ConvertTxH(fValue)) != 0) {
			m_tTDdpDisplayParams.nSetList |= DdpDisplayParamsMask::HIGH_CODE;
		} else {
			m_tTDdpDisplayParams.nSetList &= ~DdpDisplayParamsMask::HIGH_CODE;
		}

		m_tTDdpDisplayParams.nHighCode = nValue8;
		return;
	}

	uint16_t nValue16;

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplayparams::MAX_PORTS; nPortIndex++) {
		if (Sscan::Uint16(pLine, DdpDisplayParamsConst::COUNT_PORT[nPortIndex], nValue16) == Sscan::OK) {
			if ((nValue16 != 0) &&  (nValue16 <= ddpdisplayparams::MAX_COUNT)) {
				m_tTDdpDisplayParams.nCount[nPortIndex] = nValue16;
				m_tTDdpDisplayParams.nSetList |= (DdpDisplayParamsMask::COUNT_A << nPortIndex);
			} else {
				m_tTDdpDisplayParams.nCount[nPortIndex] = 0;
				m_tTDdpDisplayParams.nSetList &= ~(DdpDisplayParamsMask::COUNT_A << nPortIndex);
			}
		}
	}

	if (Sscan::Uint8(pLine, DdpDisplayParamsConst::TEST_PATTERN, nValue8) == Sscan::OK) {
		if ((nValue8 != static_cast<uint8_t>(pixelpatterns::Pattern::NONE)) && (nValue8 < static_cast<uint8_t>(pixelpatterns::Pattern::LAST))) {
			m_tTDdpDisplayParams.nTestPattern = nValue8;
			m_tTDdpDisplayParams.nSetList |= DdpDisplayParamsMask::TEST_PATTERN;
		} else {
			m_tTDdpDisplayParams.nTestPattern = static_cast<uint8_t>(pixelpatterns::Pattern::NONE);
			m_tTDdpDisplayParams.nSetList &= ~DdpDisplayParamsMask::TEST_PATTERN;
		}
		return;
	}
}


void DdpDisplayParams::Builder(const struct TDdpDisplayParams *pDdpDisplayParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (pDdpDisplayParams != nullptr) {
		memcpy(&m_tTDdpDisplayParams, pDdpDisplayParams, sizeof(struct TDdpDisplayParams));
	} else {
		m_pDdpDisplayParamsStore->Copy(&m_tTDdpDisplayParams);
	}

	PropertiesBuilder builder(DdpDisplayParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_tTDdpDisplayParams.nType)), isMaskSet(DdpDisplayParamsMask::TYPE));

	builder.AddComment("Overwrite datasheet");
	if (!isMaskSet(DdpDisplayParamsMask::MAP)) {
		m_tTDdpDisplayParams.nMap = static_cast<uint8_t>(PixelConfiguration::GetRgbMapping(static_cast<pixel::Type>(m_tTDdpDisplayParams.nType)));
	}
	builder.Add(DevicesParamsConst::MAP, PixelType::GetMap(static_cast<pixel::Map>(m_tTDdpDisplayParams.nMap)), isMaskSet(DdpDisplayParamsMask::MAP));

	if (!isMaskSet(DdpDisplayParamsMask::LOW_CODE) || !isMaskSet(DdpDisplayParamsMask::HIGH_CODE)) {
		uint8_t nLowCode;
		uint8_t nHighCode;

		PixelConfiguration::GetTxH(static_cast<pixel::Type>(m_tTDdpDisplayParams.nType), nLowCode, nHighCode);

		if (!isMaskSet(DdpDisplayParamsMask::LOW_CODE)) {
			m_tTDdpDisplayParams.nLowCode = nLowCode;
		}


		if (!isMaskSet(DdpDisplayParamsMask::HIGH_CODE)) {
			m_tTDdpDisplayParams.nHighCode = nHighCode;
		}
	}

	builder.AddComment("Overwrite timing (us)");
	builder.Add(DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_tTDdpDisplayParams.nLowCode), isMaskSet(DdpDisplayParamsMask::LOW_CODE), 2);
	builder.Add(DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_tTDdpDisplayParams.nHighCode), isMaskSet(DdpDisplayParamsMask::HIGH_CODE), 2);

	builder.AddComment("Port configuration");

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplayparams::MAX_PORTS; nPortIndex++) {
		builder.Add(DdpDisplayParamsConst::COUNT_PORT[nPortIndex], m_tTDdpDisplayParams.nCount[nPortIndex], isMaskSet(DdpDisplayParamsMask::COUNT_A << nPortIndex));
	}

	builder.AddComment("Test pattern");
	builder.Add(DdpDisplayParamsConst::TEST_PATTERN, m_tTDdpDisplayParams.nTestPattern, isMaskSet(DdpDisplayParamsMask::TEST_PATTERN));

	nSize = builder.GetSize();
}

void DdpDisplayParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	if (m_pDdpDisplayParamsStore == nullptr) {
		nSize = 0;
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}

void DdpDisplayParams::Set(DdpDisplayPixelConfiguration *pPixelConfiguration) {
	assert(pPixelConfiguration != nullptr);

	// Pixel

	if (isMaskSet(DdpDisplayParamsMask::TYPE)) {
		pPixelConfiguration->SetType(static_cast<pixel::Type>(m_tTDdpDisplayParams.nType));
	}

	if (isMaskSet(DdpDisplayParamsMask::MAP)) {
		pPixelConfiguration->SetMap(static_cast<pixel::Map>(m_tTDdpDisplayParams.nMap));
	}

	if (isMaskSet(DdpDisplayParamsMask::LOW_CODE)) {
		pPixelConfiguration->SetLowCode(m_tTDdpDisplayParams.nLowCode);
	}

	if (isMaskSet(DdpDisplayParamsMask::HIGH_CODE)) {
		pPixelConfiguration->SetHighCode(m_tTDdpDisplayParams.nHighCode);
	}

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplayparams::MAX_PORTS; nPortIndex++) {
		pPixelConfiguration->SetCountPort(nPortIndex, m_tTDdpDisplayParams.nCount[nPortIndex]);
	}
}

void DdpDisplayParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<DdpDisplayParams*>(p))->callbackFunction(s);
}

void DdpDisplayParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, DdpDisplayParamsConst::FILE_NAME);

	if (isMaskSet(DdpDisplayParamsMask::TYPE)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::TYPE, PixelType::GetType(static_cast<pixel::Type>(m_tTDdpDisplayParams.nType)), static_cast<int>(m_tTDdpDisplayParams.nType));
	}

	if (isMaskSet(DdpDisplayParamsMask::MAP)) {
		printf(" %s=%d [%s]\n", DevicesParamsConst::MAP, static_cast<int>(m_tTDdpDisplayParams.nMap), PixelType::GetMap(static_cast<pixel::Map>(m_tTDdpDisplayParams.nMap)));
	}

	if (isMaskSet(DdpDisplayParamsMask::LOW_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T0H, PixelType::ConvertTxH(m_tTDdpDisplayParams.nLowCode), m_tTDdpDisplayParams.nLowCode);
	}

	if (isMaskSet(DdpDisplayParamsMask::HIGH_CODE)) {
		printf(" %s=%.2f [0x%X]\n", DevicesParamsConst::LED_T1H, PixelType::ConvertTxH(m_tTDdpDisplayParams.nHighCode), m_tTDdpDisplayParams.nHighCode);
	}

	for (uint32_t nPortIndex = 0; nPortIndex < ddpdisplayparams::MAX_PORTS; nPortIndex++) {
		if (isMaskSet(DdpDisplayParamsMask::COUNT_A << nPortIndex)) {
			printf(" %s=%d\n", DdpDisplayParamsConst::COUNT_PORT[nPortIndex], m_tTDdpDisplayParams.nCount[nPortIndex]);
		}
	}

	if (isMaskSet(DdpDisplayParamsMask::TEST_PATTERN)) {
		printf(" %s=%d\n", DdpDisplayParamsConst::TEST_PATTERN, m_tTDdpDisplayParams.nTestPattern);
	}
#endif
}
