/**
 * @file apparams.cpp
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

#if defined(__GNUC__) && !defined(__clang__)	
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "apparams.h"

#include "readconfigfile.h"
#include "sscan.h"

static constexpr char PARAMS_FILE_NAME[] = "ap.txt";
static constexpr char PARAMS_PASSWORD[] = "password";

ApParams::ApParams(ApParamsStore *pApParamsStore): m_pApParamsStore(pApParamsStore) {
	memset(&m_tApParams, 0, sizeof(struct TApParams));
}

bool ApParams::Load() {
	m_tApParams.nSetList = 0;

	ReadConfigFile configfile(ApParams::StaticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pApParamsStore != nullptr) {
			m_pApParamsStore->Update(&m_tApParams);
		}
	} else if (m_pApParamsStore != nullptr) {
		m_pApParamsStore->Copy(&m_tApParams);
	} else {
		return false;
	}

	return true;
}

void ApParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pApParamsStore != nullptr);

	if (m_pApParamsStore == nullptr) {
		return;
	}

	m_tApParams.nSetList = 0;

	ReadConfigFile config(ApParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pApParamsStore->Update(&m_tApParams);
}

void ApParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nLength = 34 - 1;
	if (Sscan::Char(pLine, PARAMS_PASSWORD, m_tApParams.aPassword, nLength) == Sscan::OK) {
		m_tApParams.aPassword[nLength] = '\0';
		m_tApParams.nSetList |= ApParamsMask::PASSWORD;
		return;
	}
}

void ApParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<ApParams*>(p))->callbackFunction(s);
}

void ApParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(ApParamsMask::PASSWORD)) {
		printf(" %s=%s\n", PARAMS_PASSWORD, m_tApParams.aPassword);
	}
#endif
}
