/**
 * @file oscclientparams.cpp
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
 #pragma GCC push_options
 #pragma GCC optimize ("Os")
#endif

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <cassert>

#include "oscclientparams.h"
#include "oscclientparamsconst.h"
#include "oscparamsconst.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

OscClientParams::OscClientParams(OscClientParamsStore* pOscClientParamsStore): m_pOscClientParamsStore(pOscClientParamsStore) {
	memset(&m_tOscClientParams, 0, sizeof(struct TOscClientParams));
	m_tOscClientParams.nOutgoingPort = OscClientDefault::PORT_OUTGOING;
	m_tOscClientParams.nIncomingPort = OscClientDefault::PORT_INCOMING;
	m_tOscClientParams.nPingDelay = OscClientDefault::PING_DELAY_SECONDS;

	assert(sizeof(m_aCmd) > strlen(OscClientParamsConst::CMD));
	strncpy(m_aCmd, OscClientParamsConst::CMD, sizeof(m_aCmd));

	assert(sizeof(m_aLed) > strlen(OscClientParamsConst::LED));
	strncpy(m_aLed, OscClientParamsConst::LED, sizeof(m_aLed));
}

bool OscClientParams::Load() {
	m_tOscClientParams.nSetList = 0;

	ReadConfigFile configfile(OscClientParams::staticCallbackFunction, this);

	if (configfile.Read(OscClientParamsConst::FILE_NAME)) {
		// There is a configuration file
		if (m_pOscClientParamsStore != nullptr) {
			m_pOscClientParamsStore->Update(&m_tOscClientParams);
		}
	} else if (m_pOscClientParamsStore != nullptr) {
		m_pOscClientParamsStore->Copy(&m_tOscClientParams);
	} else {
		return false;
	}

	return true;
}

void OscClientParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != nullptr);
	assert(nLength != 0);
	assert(m_pOscClientParamsStore != nullptr);

	if (m_pOscClientParamsStore == nullptr) {
		return;
	}

	m_tOscClientParams.nSetList = 0;

	ReadConfigFile config(OscClientParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pOscClientParamsStore->Update(&m_tOscClientParams);
}

void OscClientParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, OscClientParamsConst::SERVER_IP, nValue32) == Sscan::OK) {
		m_tOscClientParams.nServerIp = nValue32;
		m_tOscClientParams.nSetList |= OscClientParamsMask::SERVER_IP;
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tOscClientParams.nOutgoingPort = nValue16;
			m_tOscClientParams.nSetList |= OscClientParamsMask::OUTGOING_PORT;
		} else {
			m_tOscClientParams.nSetList &= ~OscClientParamsMask::OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_tOscClientParams.nIncomingPort = nValue16;
			m_tOscClientParams.nSetList |= OscClientParamsMask::INCOMING_PORT;
		} else {
			m_tOscClientParams.nSetList &= ~OscClientParamsMask::INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, OscClientParamsConst::PING_DISABLE, nValue8) == Sscan::OK) {
		m_tOscClientParams.nPingDisable = (nValue8 != 0);
		m_tOscClientParams.nSetList |= OscClientParamsMask::PING_DISABLE;
		return;
	}

	if (Sscan::Uint8(pLine, OscClientParamsConst::PING_DELAY, nValue8) == Sscan::OK) {
		if ((nValue8 >= 2) && (nValue8 <= 60)) {
			m_tOscClientParams.nPingDelay = nValue8;
			m_tOscClientParams.nSetList |= OscClientParamsMask::PING_DELAY;
		} else {
			m_tOscClientParams.nSetList &= ~OscClientParamsMask::PING_DELAY;
		}
		return;
	}

	for (uint32_t i = 0; i < OscClientParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = i + '0';
		nValue32 = OscClientParamsMax::CMD_PATH_LENGTH;
		if (Sscan::Char(pLine, m_aCmd, reinterpret_cast<char*>(&m_tOscClientParams.aCmd[i]), nValue32) == Sscan::OK) {
			if (m_tOscClientParams.aCmd[i][0] == '/') {
				m_tOscClientParams.nSetList |= OscClientParamsMask::CMD;
			} else {
				m_tOscClientParams.aCmd[i][0] = '\0';
			}
		}
	}

	for (uint32_t i = 0; i < OscClientParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = i + '0';
		nValue32 = OscClientParamsMax::LED_PATH_LENGTH;
		if (Sscan::Char(pLine, m_aLed, reinterpret_cast<char*>(&m_tOscClientParams.aLed[i]), nValue32) == Sscan::OK) {
			if (m_tOscClientParams.aLed[i][0] == '/') {
				m_tOscClientParams.nSetList |= OscClientParamsMask::LED;
			} else {
				m_tOscClientParams.aLed[i][0] = '\0';
			}
		}
	}
}

void OscClientParams::Set(OscClient* pOscClient) {
	assert(pOscClient != nullptr);

	if (isMaskSet(OscClientParamsMask::SERVER_IP)) {
		pOscClient->SetServerIP(m_tOscClientParams.nServerIp);
	}

	if (isMaskSet(OscClientParamsMask::OUTGOING_PORT)) {
		pOscClient->SetPortOutgoing(m_tOscClientParams.nOutgoingPort);
	}

	if (isMaskSet(OscClientParamsMask::INCOMING_PORT)) {
		pOscClient->SetPortIncoming(m_tOscClientParams.nIncomingPort);
	}

	if (isMaskSet(OscClientParamsMask::PING_DISABLE)) {
		pOscClient->SetPingDisable(m_tOscClientParams.nPingDisable);
	}

	if (isMaskSet(OscClientParamsMask::PING_DELAY)) {
		pOscClient->SetPingDelay(m_tOscClientParams.nPingDelay);
	}

	if (isMaskSet(OscClientParamsMask::CMD)) {
		pOscClient->CopyCmds(reinterpret_cast<const char*>(&m_tOscClientParams.aCmd), OscClientParamsMax::CMD_COUNT, OscClientParamsMax::CMD_PATH_LENGTH);
	}

	if (isMaskSet(OscClientParamsMask::LED)) {
		pOscClient->CopyLeds(reinterpret_cast<const char*>(&m_tOscClientParams.aLed), OscClientParamsMax::LED_COUNT, OscClientParamsMax::LED_PATH_LENGTH);
	}
}

void OscClientParams::Dump() {
#ifndef NDEBUG
	if (m_tOscClientParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscClientParamsConst::FILE_NAME);

	if (isMaskSet(OscClientParamsMask::SERVER_IP)) {
		printf(" %s=" IPSTR "\n", OscClientParamsConst::SERVER_IP, IP2STR(m_tOscClientParams.nServerIp));
	}

	if (isMaskSet(OscClientParamsMask::OUTGOING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_tOscClientParams.nOutgoingPort);
	}

	if (isMaskSet(OscClientParamsMask::INCOMING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_tOscClientParams.nIncomingPort);
	}

	if (isMaskSet(OscClientParamsMask::PING_DISABLE)) {
		printf(" %s=%d [%s]\n", OscClientParamsConst::PING_DISABLE, m_tOscClientParams.nPingDisable, m_tOscClientParams.nPingDisable == 0 ? "No" : "Yes");
	}

	if (isMaskSet(OscClientParamsMask::PING_DELAY)) {
		printf(" %s=%ds\n", OscClientParamsConst::PING_DELAY, m_tOscClientParams.nPingDelay);
	}

	if (isMaskSet(OscClientParamsMask::CMD)) {
		for (uint32_t i = 0; i < OscClientParamsMax::CMD_COUNT; i++) {
			m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = i + '0';
			printf(" %s=[%s]\n", m_aCmd, reinterpret_cast<char*>(&m_tOscClientParams.aCmd[i]));
		}
	}

	if (isMaskSet(OscClientParamsMask::LED)) {
		for (uint32_t i = 0; i < OscClientParamsMax::LED_COUNT; i++) {
			m_aLed[strlen(OscClientParamsConst::LED) - 1] = i + '0';
			printf(" %s=[%s]\n", m_aLed, reinterpret_cast<char*>(&m_tOscClientParams.aLed[i]));
		}
	}
#endif
}

void OscClientParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OscClientParams*>(p))->callbackFunction(s);
}
