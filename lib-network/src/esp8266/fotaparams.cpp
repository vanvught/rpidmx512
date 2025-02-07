/**
 * @file fotaparams.cpp
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

#include "fotaparams.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"

static constexpr char PARAMS_FILE_NAME[] = "fota.txt";
static constexpr char PARAMS_SERVER[] = "server";

FotaParams::FotaParams(FotaParamsStore *pFotaParamsStore): m_pFotaParamsStore(pFotaParamsStore) {
	memset(&m_tFotaParams, 0, sizeof(struct TFotaParams));
}

bool FotaParams::Load() {
	m_tFotaParams.nSetList = 0;

	ReadConfigFile configfile(FotaParams::StaticCallbackFunction, this);

	if (configfile.Read(PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pFotaParamsStore != nullptr) {
			m_pFotaParamsStore->Update(&m_tFotaParams);
		}
	} else if (m_pFotaParamsStore != nullptr) {
		m_pFotaParamsStore->Copy(&m_tFotaParams);
	} else {
		return false;
	}

	return true;
}

void FotaParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pFotaParamsStore != nullptr);

	if (m_pFotaParamsStore == nullptr) {
		return;
	}

	m_tFotaParams.nSetList = 0;

	ReadConfigFile config(FotaParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pFotaParamsStore->Update(&m_tFotaParams);
}

void FotaParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, PARAMS_SERVER, nValue32) == Sscan::OK) {
		m_tFotaParams.nServerIp = nValue32;
		m_tFotaParams.nSetList |= FotaParamsMask::SERVER;
		return;
	}
}

void FotaParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<FotaParams*>(p))->callbackFunction(s);
}

void FotaParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, PARAMS_FILE_NAME);

	if (isMaskSet(FotaParamsMask::SERVER)) {
		printf(" %s=" IPSTR "\n",  PARAMS_SERVER, IP2STR(m_tFotaParams.nServerIp));
	}
#endif
}

