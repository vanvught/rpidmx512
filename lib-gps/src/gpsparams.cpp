/**
 * @file gpsparams.cpp
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

#include "gpsparams.h"
#include "gpsparamsconst.h"
#include "gps.h"
#include "gpsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

GPSParams::GPSParams(GPSParamsStore *pGPSParamsStore): m_pGPSParamsStore(pGPSParamsStore) {
	DEBUG_ENTRY
	DEBUG_PRINTF("sizeof(struct TGPSParams) = %d", static_cast<int>(sizeof(struct TGPSParams)));

	memset(&m_tTGPSParams, 0, sizeof(struct TGPSParams));

	m_tTGPSParams.nModule = static_cast<uint8_t>(GPSModule::UNDEFINED);

	DEBUG_EXIT
}

bool GPSParams::Load() {
	m_tTGPSParams.nSetList = 0;

	ReadConfigFile configfile(GPSParams::staticCallbackFunction, this);

	if (configfile.Read(GPSParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pGPSParamsStore != nullptr) {
			m_pGPSParamsStore->Update(&m_tTGPSParams);
		}
	} else if (m_pGPSParamsStore != nullptr) {
		m_pGPSParamsStore->Copy(&m_tTGPSParams);
	} else {
		return false;
	}

	return true;
}

void GPSParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);

	assert(m_pGPSParamsStore != nullptr);

	if (m_pGPSParamsStore == nullptr) {
		return;
	}

	m_tTGPSParams.nSetList = 0;

	ReadConfigFile config(GPSParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pGPSParamsStore->Update(&m_tTGPSParams);
}

void GPSParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char moduleName[16];
	uint32_t nLength = sizeof(moduleName) - 1;

	if (Sscan::Char(pLine, GPSParamsConst::MODULE, moduleName, nLength) == Sscan::OK) {
		moduleName[nLength] = '\0';
		m_tTGPSParams.nModule = static_cast<uint8_t>(GPS::GetModule(moduleName));

		if (m_tTGPSParams.nModule != static_cast<uint8_t>(GPSModule::UNDEFINED)) {
			m_tTGPSParams.nSetList |= GPSParamsMask::MODULE;
		} else {
			m_tTGPSParams.nSetList &= ~GPSParamsMask::MODULE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, GPSParamsConst::ENABLE, nValue8) == Sscan::OK) {
		m_tTGPSParams.nEnable = !(nValue8 == 0);
		m_tTGPSParams.nSetList |= GPSParamsMask::ENABLE;
		return;
	}

	float f;

	if (Sscan::Float(pLine, GPSParamsConst::UTC_OFFSET, f) == Sscan::OK) {
		if ((static_cast<int32_t>(f) >= -12) && (static_cast<int32_t>(f) <= 14)) {
			m_tTGPSParams.fUtcOffset = f;
			m_tTGPSParams.nSetList |= GPSParamsMask::UTC_OFFSET;
			return;
		} else {
			m_tTGPSParams.fUtcOffset = 0.0;
			m_tTGPSParams.nSetList &= ~GPSParamsMask::UTC_OFFSET;
			return;
		}
	}
}

void GPSParams::Builder(const struct TGPSParams *pGPSParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	assert(pBuffer != nullptr);

	if (pGPSParams != nullptr) {
		memcpy(&m_tTGPSParams, pGPSParams, sizeof(struct TGPSParams));
	} else {
		m_pGPSParamsStore->Copy(&m_tTGPSParams);
	}

	PropertiesBuilder builder(GPSParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(GPSParamsConst::MODULE, GPS::GetModuleName(static_cast<GPSModule>(m_tTGPSParams.nModule)), isMaskSet(GPSParamsMask::MODULE));
	builder.Add(GPSParamsConst::ENABLE, m_tTGPSParams.nEnable, isMaskSet(GPSParamsMask::ENABLE));
	builder.Add(GPSParamsConst::UTC_OFFSET, m_tTGPSParams.fUtcOffset, isMaskSet(GPSParamsMask::UTC_OFFSET));

	nSize = builder.GetSize();
}

void GPSParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pGPSParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	DEBUG_EXIT
}

void GPSParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<GPSParams *>(p))->callbackFunction(s);
}
