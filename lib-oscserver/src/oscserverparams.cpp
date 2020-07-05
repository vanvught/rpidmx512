/**
 * @file oscserverparams.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "oscserverparms.h"
#include "oscserverconst.h"
#include "osc.h"
#include "oscparamsconst.h"

#include "lightsetconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

OSCServerParams::OSCServerParams(OSCServerParamsStore *pOSCServerParamsStore): m_pOSCServerParamsStore(pOSCServerParamsStore) {
	memset(&m_tOSCServerParams, 0, sizeof(struct TOSCServerParams));
	m_tOSCServerParams.nIncomingPort = osc::port::DEFAULT_INCOMING;
	m_tOSCServerParams.nOutgoingPort = osc::port::DEFAULT_OUTGOING;
}

bool OSCServerParams::Load() {
	m_tOSCServerParams.nSetList = 0;

	ReadConfigFile configfile(OSCServerParams::staticCallbackFunction, this);

	if (configfile.Read(OSCServerConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pOSCServerParamsStore != nullptr) {
			m_pOSCServerParamsStore->Update(&m_tOSCServerParams);
		}
	} else if (m_pOSCServerParamsStore != nullptr) {
		m_pOSCServerParamsStore->Copy(&m_tOSCServerParams);
	} else {
		return false;
	}

	return true;
}

void OSCServerParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pOSCServerParamsStore != nullptr);

	if (m_pOSCServerParamsStore == nullptr) {
		return;
	}

	m_tOSCServerParams.nSetList = 0;

	ReadConfigFile config(OSCServerParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pOSCServerParamsStore->Update(&m_tOSCServerParams);
}

void OSCServerParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nLength;

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tOSCServerParams.nIncomingPort = nValue16;
			m_tOSCServerParams.nSetList |= OSCServerParamsMask::INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tOSCServerParams.nOutgoingPort = nValue16;
			m_tOSCServerParams.nSetList |= OSCServerParamsMask::OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, OSCServerConst::PARAMS_TRANSMISSION, nValue8) == Sscan::OK) {
		m_tOSCServerParams.bPartialTransmission = (nValue8 != 0);
		m_tOSCServerParams.nSetList |= OSCServerParamsMask::TRANSMISSION;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPath) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath, nLength) == Sscan::OK) {
		m_tOSCServerParams.nSetList |= OSCServerParamsMask::PATH;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPathInfo) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, nLength) == Sscan::OK) {
		m_tOSCServerParams.nSetList |= OSCServerParamsMask::PATH_INFO;
		return;
	}

	nLength = sizeof(m_tOSCServerParams.aPathBlackOut) - 1;
	if (Sscan::Char(pLine, OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathBlackOut, nLength) == Sscan::OK) {
		m_tOSCServerParams.nSetList |= OSCServerParamsMask::PATH_BLACKOUT;
		return;
	}

	if (Sscan::Uint8(pLine, LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, nValue8) == Sscan::OK) {
		m_tOSCServerParams.bEnableNoChangeUpdate = (nValue8 != 0);
		m_tOSCServerParams.nSetList |= OSCServerParamsMask::ENABLE_NO_CHANGE_OUTPUT;
		return;
	}
}

void OSCServerParams::Dump() {
#ifndef NDEBUG
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OSCServerConst::PARAMS_FILE_NAME);

	if (isMaskSet(OSCServerParamsMask::INCOMING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_tOSCServerParams.nIncomingPort);
	}

	if (isMaskSet(OSCServerParamsMask::OUTGOING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_tOSCServerParams.nOutgoingPort);
	}

	if (isMaskSet(OSCServerParamsMask::PATH)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath);
	}

	if (isMaskSet(OSCServerParamsMask::PATH_INFO)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo);
	}

	if (isMaskSet(OSCServerParamsMask::PATH_BLACKOUT)) {
		printf(" %s=%s\n", OSCServerConst::PARAMS_PATH_BLACKOUT, m_tOSCServerParams.aPathBlackOut);
	}

	if (isMaskSet(OSCServerParamsMask::TRANSMISSION)) {
		printf(" %s=%d\n", OSCServerConst::PARAMS_TRANSMISSION, m_tOSCServerParams.bPartialTransmission);
	}

	if(isMaskSet(OSCServerParamsMask::ENABLE_NO_CHANGE_OUTPUT)) {
		printf(" %s=%d\n", LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tOSCServerParams.bEnableNoChangeUpdate);
	}
#endif
}

void OSCServerParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OSCServerParams*>(p))->callbackFunction(s);
}

