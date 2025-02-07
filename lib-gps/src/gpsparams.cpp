/**
 * @file gpsparams.cpp
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "gpsparams.h"
#include "gpsparamsconst.h"
#include "gps.h"
#include "gpsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

GPSParams::GPSParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct gpsparams::Params));

	m_Params.nModule = static_cast<uint8_t>(GPSModule::UNDEFINED);

	DEBUG_EXIT
}

void GPSParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(GPSParams::StaticCallbackFunction, this);

	if (configfile.Read(GPSParamsConst::FILE_NAME)) {
		GPSParamsStore::Update(&m_Params);
	} else
#endif
		GPSParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void GPSParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(GPSParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	GPSParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void GPSParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	char moduleName[16];
	uint32_t nLength = sizeof(moduleName) - 1;

	if (Sscan::Char(pLine, GPSParamsConst::MODULE, moduleName, nLength) == Sscan::OK) {
		moduleName[nLength] = '\0';
		m_Params.nModule = static_cast<uint8_t>(GPS::GetModule(moduleName));

		if (m_Params.nModule != static_cast<uint8_t>(GPSModule::UNDEFINED)) {
			m_Params.nSetList |= gpsparams::Mask::MODULE;
		} else {
			m_Params.nSetList &= ~gpsparams::Mask::MODULE;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, GPSParamsConst::ENABLE, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= gpsparams::Mask::ENABLE;
		} else {
			m_Params.nSetList &= ~gpsparams::Mask::ENABLE;
		}
		return;
	}

	float f;

	if (Sscan::Float(pLine, GPSParamsConst::UTC_OFFSET, f) == Sscan::OK) {
		if ((static_cast<int32_t>(f) >= -12) && (static_cast<int32_t>(f) <= 14) && (static_cast<int32_t>(f) != 0)) {
			m_Params.fUtcOffset = f;
			m_Params.nSetList |= gpsparams::Mask::UTC_OFFSET;
			return;
		} else {
			m_Params.fUtcOffset = 0.0;
			m_Params.nSetList &= ~gpsparams::Mask::UTC_OFFSET;
			return;
		}
	}
}

void GPSParams::Builder(const struct gpsparams::Params *pGPSParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	assert(pBuffer != nullptr);

	if (pGPSParams != nullptr) {
		memcpy(&m_Params, pGPSParams, sizeof(struct gpsparams::Params));
	} else {
		GPSParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(GPSParamsConst::FILE_NAME, pBuffer, nLength);

	builder.Add(GPSParamsConst::MODULE, GPS::GetModuleName(static_cast<GPSModule>(m_Params.nModule)), isMaskSet(gpsparams::Mask::MODULE));
	builder.Add(GPSParamsConst::ENABLE, isMaskSet(gpsparams::Mask::ENABLE));
	builder.Add(GPSParamsConst::UTC_OFFSET, m_Params.fUtcOffset, isMaskSet(gpsparams::Mask::UTC_OFFSET));

	nSize = builder.GetSize();
}

void GPSParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<GPSParams *>(p))->callbackFunction(s);
}

void GPSParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, GPSParamsConst::FILE_NAME);
	printf(" %s=%d [%s]\n", GPSParamsConst::MODULE, static_cast<int>(m_Params.nModule), GPS::GetModuleName(static_cast<GPSModule>(m_Params.nModule)));
	printf(" %s=%d\n", GPSParamsConst::ENABLE, isMaskSet(gpsparams::Mask::ENABLE));
	printf(" %s=%1.1f\n", GPSParamsConst::UTC_OFFSET, m_Params.fUtcOffset);
}
