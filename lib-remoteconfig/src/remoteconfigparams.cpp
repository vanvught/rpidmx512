/**
 * @file remoteconfigparams.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

	ReadConfigFile configfile(RemoteConfigParams::staticCallbackFunction, this);

	if (configfile.Read(RemoteConfigConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pRemoteConfigParamsStore != nullptr) {
			m_pRemoteConfigParamsStore->Update(&m_tRemoteConfigParams);
		}
	} else if (m_pRemoteConfigParamsStore != nullptr) {
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

void RemoteConfigParams::SetBool(const uint8_t nValue, bool& nProperty, const uint32_t nMask) {
	if (nValue != 0) {
		nProperty = true;
		m_tRemoteConfigParams.nSetList |= nMask;
	} else {
		nProperty = false;
		m_tRemoteConfigParams.nSetList &= ~nMask;
	}
}

void RemoteConfigParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tRemoteConfigParams.bDisable, RemoteConfigParamsMask::DISABLE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE_WRITE, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tRemoteConfigParams.bDisableWrite, RemoteConfigParamsMask::DISABLE_WRITE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_REBOOT, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tRemoteConfigParams.bEnableReboot, RemoteConfigParamsMask::ENABLE_REBOOT);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_UPTIME, nValue8) == Sscan::OK) {
		SetBool(nValue8, m_tRemoteConfigParams.bEnableUptime, RemoteConfigParamsMask::ENABLE_UPTIME);
		return;
	}

	uint32_t nLength = REMOTE_CONFIG_DISPLAY_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, RemoteConfigConst::PARAMS_DISPLAY_NAME, m_tRemoteConfigParams.aDisplayName, nLength) == Sscan::OK) {
		m_tRemoteConfigParams.aDisplayName[nLength] = '\0';
		m_tRemoteConfigParams.nSetList |= RemoteConfigParamsMask::DISPLAY_NAME;
		return;
	}
}

void RemoteConfigParams::Builder(const struct TRemoteConfigParams *pRemoteConfigParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pRemoteConfigParams != nullptr) {
		memcpy(&m_tRemoteConfigParams, pRemoteConfigParams, sizeof(struct TRemoteConfigParams));
	} else {
		m_pRemoteConfigParamsStore->Copy(&m_tRemoteConfigParams);
	}

	PropertiesBuilder builder(RemoteConfigConst::PARAMS_FILE_NAME, pBuffer, nLength);

	builder.Add(RemoteConfigConst::PARAMS_DISABLE, m_tRemoteConfigParams.bDisable, isMaskSet(RemoteConfigParamsMask::DISABLE));
	builder.Add(RemoteConfigConst::PARAMS_DISABLE_WRITE, m_tRemoteConfigParams.bDisableWrite, isMaskSet(RemoteConfigParamsMask::DISABLE_WRITE));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_REBOOT, m_tRemoteConfigParams.bEnableReboot, isMaskSet(RemoteConfigParamsMask::ENABLE_REBOOT));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_UPTIME, m_tRemoteConfigParams.bEnableUptime, isMaskSet(RemoteConfigParamsMask::ENABLE_UPTIME));
	builder.Add(RemoteConfigConst::PARAMS_DISPLAY_NAME, m_tRemoteConfigParams.aDisplayName, isMaskSet(RemoteConfigParamsMask::DISPLAY_NAME));

//	builder.AddComment("RDMNet LLRP Only (not used, yet)");
//	builder.Add(RemoteConfigConst::PARAMS_DISABLE_RDMNET_LLRP_ONLY, m_tRemoteConfigParams.bDisableRdmNetLlrpOnly, isMaskSet(RemoteConfigParamsMask::DISABLE_RDMNET_LLRP_ONY));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return;
}

void RemoteConfigParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
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

	if (isMaskSet(RemoteConfigParamsMask::DISABLE)) {
		pRemoteConfig->SetDisable(m_tRemoteConfigParams.bDisable);
	}

	if (isMaskSet(RemoteConfigParamsMask::DISABLE_WRITE)) {
		pRemoteConfig->SetDisableWrite(m_tRemoteConfigParams.bDisableWrite);
	}

	if (isMaskSet(RemoteConfigParamsMask::ENABLE_REBOOT)) {
		pRemoteConfig->SetEnableReboot(m_tRemoteConfigParams.bEnableReboot);
	}

	if (isMaskSet(RemoteConfigParamsMask::ENABLE_UPTIME)) {
		pRemoteConfig->SetEnableUptime(m_tRemoteConfigParams.bEnableUptime);
	}

	if (isMaskSet(RemoteConfigParamsMask::DISPLAY_NAME)) {
		pRemoteConfig->SetDisplayName(m_tRemoteConfigParams.aDisplayName);
	}
}

void RemoteConfigParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RemoteConfigParams*>(p))->callbackFunction(s);
}
