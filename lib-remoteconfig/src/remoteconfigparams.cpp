/**
 * @file remoteconfigparams.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cassert>

#include "remoteconfigparams.h"
#include "remoteconfig.h"
#include "remoteconfigconst.h"


#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

RemoteConfigParams::RemoteConfigParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct remoteconfigparams::Params));

	DEBUG_EXIT
}

void RemoteConfigParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(RemoteConfigParams::staticCallbackFunction, this);

	if (configfile.Read(RemoteConfigConst::PARAMS_FILE_NAME)) {
		RemoteConfigParamsStore::Update(&m_Params);
	} else
#endif
		RemoteConfigParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RemoteConfigParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(RemoteConfigParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	RemoteConfigParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void RemoteConfigParams::SetBool(const uint8_t nValue, const uint32_t nMask) {
	if (nValue != 0) {
		m_Params.nSetList |= nMask;
	} else {
		m_Params.nSetList &= ~nMask;
	}
}

void RemoteConfigParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE, nValue8) == Sscan::OK) {
		SetBool(nValue8, remoteconfigparams::Mask::DISABLE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE_WRITE, nValue8) == Sscan::OK) {
		SetBool(nValue8, remoteconfigparams::Mask::DISABLE_WRITE);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_REBOOT, nValue8) == Sscan::OK) {
		SetBool(nValue8, remoteconfigparams::Mask::ENABLE_REBOOT);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_UPTIME, nValue8) == Sscan::OK) {
		SetBool(nValue8, remoteconfigparams::Mask::ENABLE_UPTIME);
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_FACTORY, nValue8) == Sscan::OK) {
		SetBool(nValue8, remoteconfigparams::Mask::ENABLE_FACTORY);
		return;
	}

	uint32_t nLength = remoteconfig::DISPLAY_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, RemoteConfigConst::PARAMS_DISPLAY_NAME, m_Params.aDisplayName, nLength) == Sscan::OK) {
		m_Params.aDisplayName[nLength] = '\0';
		m_Params.nSetList |= remoteconfigparams::Mask::DISPLAY_NAME;
		return;
	}
}

void RemoteConfigParams::Builder(const struct remoteconfigparams::Params *pRemoteConfigParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pRemoteConfigParams != nullptr) {
		memcpy(&m_Params, pRemoteConfigParams, sizeof(struct remoteconfigparams::Params));
	} else {
		RemoteConfigParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(RemoteConfigConst::PARAMS_FILE_NAME, pBuffer, nLength);

	builder.Add(RemoteConfigConst::PARAMS_DISABLE, isMaskSet(remoteconfigparams::Mask::DISABLE));
	builder.Add(RemoteConfigConst::PARAMS_DISABLE_WRITE, isMaskSet(remoteconfigparams::Mask::DISABLE_WRITE));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_REBOOT, isMaskSet(remoteconfigparams::Mask::ENABLE_REBOOT));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_UPTIME, isMaskSet(remoteconfigparams::Mask::ENABLE_UPTIME));
	builder.Add(RemoteConfigConst::PARAMS_ENABLE_FACTORY, isMaskSet(remoteconfigparams::Mask::ENABLE_FACTORY));

	builder.Add(RemoteConfigConst::PARAMS_DISPLAY_NAME, m_Params.aDisplayName, isMaskSet(remoteconfigparams::Mask::DISPLAY_NAME));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return;
}

void RemoteConfigParams::Set(RemoteConfig* pRemoteConfig) {
	assert(pRemoteConfig != nullptr);

	pRemoteConfig->SetDisable(isMaskSet(remoteconfigparams::Mask::DISABLE));
	pRemoteConfig->SetDisableWrite(isMaskSet(remoteconfigparams::Mask::DISABLE_WRITE));
	pRemoteConfig->SetEnableReboot(isMaskSet(remoteconfigparams::Mask::ENABLE_REBOOT));
	pRemoteConfig->SetEnableUptime(isMaskSet(remoteconfigparams::Mask::ENABLE_UPTIME));
	pRemoteConfig->SetEnableFactory(isMaskSet(remoteconfigparams::Mask::ENABLE_FACTORY));

	if (isMaskSet(remoteconfigparams::Mask::DISPLAY_NAME)) {
		pRemoteConfig->SetDisplayName(m_Params.aDisplayName);
	}
}

void RemoteConfigParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<RemoteConfigParams*>(p))->callbackFunction(s);
}

void RemoteConfigParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RemoteConfigConst::PARAMS_FILE_NAME);
	printf(" %s=%d\n", RemoteConfigConst::PARAMS_DISABLE, isMaskSet(remoteconfigparams::Mask::DISABLE));
	printf(" %s=%d\n", RemoteConfigConst::PARAMS_DISABLE_WRITE, isMaskSet(remoteconfigparams::Mask::DISABLE_WRITE));
	printf(" %s=%d\n", RemoteConfigConst::PARAMS_ENABLE_REBOOT, isMaskSet(remoteconfigparams::Mask::ENABLE_REBOOT));
	printf(" %s=%d\n", RemoteConfigConst::PARAMS_ENABLE_UPTIME, isMaskSet(remoteconfigparams::Mask::ENABLE_UPTIME));
	printf(" %s=%d\n", RemoteConfigConst::PARAMS_ENABLE_FACTORY, isMaskSet(remoteconfigparams::Mask::ENABLE_FACTORY));
	printf(" %s=%s\n", RemoteConfigConst::PARAMS_DISPLAY_NAME, m_Params.aDisplayName);
}
