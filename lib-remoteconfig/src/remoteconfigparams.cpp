/**
 * @file remoteconfigparams.cpp
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "remoteconfigparams.h"
#include "remoteconfig.h"
#include "remoteconfigconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

#define BOOL2STRING(b)			(b) ? "Yes" : "No"

RemoteConfigParams::RemoteConfigParams(RemoteConfigParamsStore* pTRemoteConfigParamsStore): m_pRemoteConfigParamsStore(pTRemoteConfigParamsStore) {
	uint8_t *p = (uint8_t *) &m_tRemoteConfigParams;

	for (uint32_t i = 0; i < sizeof(struct TRemoteConfigParams); i++) {
		*p++ = 0;
	}
}

RemoteConfigParams::~RemoteConfigParams(void) {
	m_tRemoteConfigParams.nSetList = 0;
}

bool RemoteConfigParams::Load(void) {
	m_tRemoteConfigParams.nSetList = 0;

	ReadConfigFile configfile(RemoteConfigParams::staticCallbackFunction, this);

	if (configfile.Read(RemoteConfigConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pRemoteConfigParamsStore != 0) {
			m_pRemoteConfigParamsStore->Update(&m_tRemoteConfigParams);
		}
	} else if (m_pRemoteConfigParamsStore != 0) {
		m_pRemoteConfigParamsStore->Copy(&m_tRemoteConfigParams);
	} else {
		return false;
	}

	return true;
}

void RemoteConfigParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pRemoteConfigParamsStore != 0);

	m_tRemoteConfigParams.nSetList = 0;

	ReadConfigFile config(RemoteConfigParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pRemoteConfigParamsStore->Update(&m_tRemoteConfigParams);
}

void RemoteConfigParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint8_t len;

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE, &value8) == SSCAN_OK) {
		m_tRemoteConfigParams.bDisabled = (value8 != 0);
		m_tRemoteConfigParams.nSetList |= REMOTE_CONFIG_PARAMS_DISABLED;
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_DISABLE_WRITE, &value8) == SSCAN_OK) {
		m_tRemoteConfigParams.bDisableWrite = (value8 != 0);
		m_tRemoteConfigParams.nSetList |= REMOTE_CONFIG_PARAMS_DISABLE_WRITE;
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_REBOOT, &value8) == SSCAN_OK) {
		m_tRemoteConfigParams.bEnableReboot = (value8 != 0);
		m_tRemoteConfigParams.nSetList |= REMOTE_CONFIG_PARAMS_ENABLE_REBOOT;
		return;
	}

	if (Sscan::Uint8(pLine, RemoteConfigConst::PARAMS_ENABLE_UPTIME, &value8) == SSCAN_OK) {
		m_tRemoteConfigParams.bEnableUptime = (value8 != 0);
		m_tRemoteConfigParams.nSetList |= REMOTE_CONFIG_PARAMS_ENABLE_UPTIME;
		return;
	}

	len = REMOTE_CONFIG_DISPLAY_NAME_LENGTH - 1;
	if (Sscan::Char(pLine, RemoteConfigConst::PARAMS_DISPLAY_NAME, (char *) m_tRemoteConfigParams.aDisplayName, &len) == SSCAN_OK) {
		m_tRemoteConfigParams.aDisplayName[len] = '\0';
		m_tRemoteConfigParams.nSetList |= REMOTE_CONFIG_PARAMS_DISPLAY_NAME;
		return;
	}
}

bool RemoteConfigParams::Builder(const struct TRemoteConfigParams* pRemoteConfigParams, uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (pRemoteConfigParams != 0) {
		memcpy(&m_tRemoteConfigParams, pRemoteConfigParams, sizeof(struct TRemoteConfigParams));
	} else {
		m_pRemoteConfigParamsStore->Copy(&m_tRemoteConfigParams);
	}

	PropertiesBuilder builder(RemoteConfigConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(RemoteConfigConst::PARAMS_DISABLE, (uint32_t) m_tRemoteConfigParams.bDisabled, isMaskSet(REMOTE_CONFIG_PARAMS_DISABLED));
	isAdded &= builder.Add(RemoteConfigConst::PARAMS_DISABLE_WRITE, (uint32_t) m_tRemoteConfigParams.bDisableWrite, isMaskSet(REMOTE_CONFIG_PARAMS_DISABLE_WRITE));
	isAdded &= builder.Add(RemoteConfigConst::PARAMS_ENABLE_REBOOT, (uint32_t) m_tRemoteConfigParams.bEnableReboot, isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_REBOOT));
	isAdded &= builder.Add(RemoteConfigConst::PARAMS_ENABLE_UPTIME, (uint32_t) m_tRemoteConfigParams.bEnableUptime, isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_UPTIME));
	isAdded &= builder.Add(RemoteConfigConst::PARAMS_DISPLAY_NAME, (char *) m_tRemoteConfigParams.aDisplayName, isMaskSet(REMOTE_CONFIG_PARAMS_DISPLAY_NAME));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return isAdded;
}

bool RemoteConfigParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pRemoteConfigParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}

void RemoteConfigParams::Set(RemoteConfig* pRemoteConfig) {
	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISABLED)) {
		pRemoteConfig->SetDisable(m_tRemoteConfigParams.bDisabled);
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISABLE_WRITE)) {
		pRemoteConfig->SetDisableWrite(m_tRemoteConfigParams.bDisableWrite);
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_REBOOT)) {
		pRemoteConfig->SetEnableReboot(m_tRemoteConfigParams.bEnableReboot);
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_UPTIME)) {
		pRemoteConfig->SetEnableUptime(m_tRemoteConfigParams.bEnableUptime);
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISPLAY_NAME)) {
		pRemoteConfig->SetDisplayName((const char *)m_tRemoteConfigParams.aDisplayName);
	}
}

void RemoteConfigParams::Dump(void) {
#ifndef NDEBUG
	if (m_tRemoteConfigParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, RemoteConfigConst::PARAMS_FILE_NAME);

	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISABLED)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_DISABLE, (int) m_tRemoteConfigParams.bDisabled, BOOL2STRING(m_tRemoteConfigParams.bDisabled));
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISABLE_WRITE)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_DISABLE_WRITE, (int) m_tRemoteConfigParams.bDisableWrite, BOOL2STRING(m_tRemoteConfigParams.bDisableWrite));
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_REBOOT)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_ENABLE_REBOOT, (int) m_tRemoteConfigParams.bEnableReboot, BOOL2STRING(m_tRemoteConfigParams.bEnableReboot));
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_ENABLE_UPTIME)) {
		printf(" %s=%d [%s]\n", RemoteConfigConst::PARAMS_ENABLE_UPTIME, (int) m_tRemoteConfigParams.bEnableUptime, BOOL2STRING(m_tRemoteConfigParams.bEnableUptime));
	}

	if (isMaskSet(REMOTE_CONFIG_PARAMS_DISPLAY_NAME)) {
		printf(" %s=%s\n", RemoteConfigConst::PARAMS_DISPLAY_NAME, (char *) m_tRemoteConfigParams.aDisplayName);
	}
#endif
}

void RemoteConfigParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((RemoteConfigParams *) p)->callbackFunction(s);
}

bool RemoteConfigParams::isMaskSet(uint32_t nMask) const {
	return (m_tRemoteConfigParams.nSetList & nMask) == nMask;
}
