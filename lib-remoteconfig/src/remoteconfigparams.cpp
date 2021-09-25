/**
 * @file remoteconfigparams.cpp
 *
 */
/* Copyright (C) 2019-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "remoteconfigparams.h"
#include "remoteconfig.h"
#include "remoteconfigconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

RemoteConfigParams::RemoteConfigParams(RemoteConfigParamsStore* pTRemoteConfigParamsStore): m_pRemoteConfigParamsStore(pTRemoteConfigParamsStore) {
	memset(&m_tRemoteConfigParams, 0, sizeof(struct TRemoteConfigParams));
}

bool RemoteConfigParams::Load() {
	m_tRemoteConfigParams.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(RemoteConfigParams::staticCallbackFunction, this);

	if (configfile.Read(RemoteConfigConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pRemoteConfigParamsStore != nullptr) {
			m_pRemoteConfigParamsStore->Update(&m_tRemoteConfigParams);
		}
	} else
#endif
	if (m_pRemoteConfigParamsStore != nullptr) {
		m_pRemoteConfigParamsStore->Copy(&m_tRemoteConfigParams);
	} else {
		return false;
	}

	return true;
}

void RemoteConfigParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pRemoteConfigParamsStore != nullptr);

	m_tRemoteConfigParams.nSetList = 0;

	ReadConfigFile config(RemoteConfigParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRemoteConfigParamsStore->Update(&m_tRemoteConfigParams);
}

void RemoteConfigParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_tRemoteConfigParams.nSetList |= nMask;
	} else {
		m_tRemoteConfigParams.nSetList &= ~nMask;
	}
}

void RemoteConfigParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, RemoteConfigParamsMask::DISABLE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE_WRITE, nValue8) == Sscan::OK) {
		SetBool(nValue8, RemoteConfigParamsMask::DISABLE_WRITE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_REBOOT, nValue8) == Sscan::OK) {
		SetBool(nValue8, RemoteConfigParamsMask::ENABLE_REBOOT);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_UPTIME, nValue8) == Sscan::OK) {
		SetBool(nValue8, RemoteConfigParamsMask::ENABLE_UPTIME);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_FACTORY, nValue8) == Sscan::OK) {
		SetBool(nValue8, RemoteConfigParamsMask::ENABLE_FACTORY);
		return;
	}

	uint32_t nLength = remoteconfig::DISPLAY_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, RemoteConfigConst::PARAMS_DISPLAY_NAME, m_tRemoteConfigParams.aDisplayName, nLength) == Sscan::OK) {
		m_tRemoteConfigParams.aDisplayName[nLength] = '\0';
		m_tRemoteConfigParams.nSetList |= RemoteConfigParamsMask::DISPLAY_NAME;
		return;
	}
}

void RemoteConfigParams::Builder(const struct TRemoteConfigParams *pRemoteConfigParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pRemoteConfigParams != nullptr) {
		memcpy(&m_tRemoteConfigParams, pRemoteConfigParams, sizeof(struct TRemoteConfigParams));
	} else {
		m_pRemoteConfigParamsStore->Copy(&m_tRemoteConfigParams);
	}

	PropertiesBuilder builder(RemoteConfigConst::PARAMS_FILE_NAME, pBuffer, nLength);

	builder.Add(RemoteConfigConst::PARAMS_DISABLE, isMaskSet(RemoteConfigParamsMask::DISABLE));
	builder.Add(RemoteConfigConst::PARAMS_DISABLE_WRITE, isMaskSet(RemoteConfigParamsMask::DISABLE_WRITE));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_REBOOT, isMaskSet(RemoteConfigParamsMask::ENABLE_REBOOT));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_UPTIME, isMaskSet(RemoteConfigParamsMask::ENABLE_UPTIME));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_FACTORY, isMaskSet(RemoteConfigParamsMask::ENABLE_FACTORY));

	builder.Add(RemoteConfigConst::PARAMS_DISPLAY_NAME, m_tRemoteConfigParams.aDisplayName, isMaskSet(RemoteConfigParamsMask::DISPLAY_NAME));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return;
}

void RemoteConfigParams::Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pRemoteConfigParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);

	return;
}

void RemoteConfigParams::Set(RemoteConfig* pRemoteConfig) {
	assert(pRemoteConfig != nullptr);

	pRemoteConfig->SetDisable(isMaskSet(RemoteConfigParamsMask::DISABLE));
	pRemoteConfig->SetDisableWrite(isMaskSet(RemoteConfigParamsMask::DISABLE_WRITE));
	pRemoteConfig->SetEnableReboot(isMaskSet(RemoteConfigParamsMask::ENABLE_REBOOT));
	pRemoteConfig->SetEnableUptime(isMaskSet(RemoteConfigParamsMask::ENABLE_UPTIME));
	pRemoteConfig->SetEnableFactory(isMaskSet(RemoteConfigParamsMask::ENABLE_FACTORY));

	if (isMaskSet(RemoteConfigParamsMask::DISPLAY_NAME)) {
		pRemoteConfig->SetDisplayName(m_tRemoteConfigParams.aDisplayName);
	}
}

void RemoteConfigParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RemoteConfigParams*>(p))->callbackFunction(s);
}
