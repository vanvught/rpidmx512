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

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "oscclientparams.h"
#include "oscclientparamsconst.h"
#include "oscconst.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

OscClientParams::OscClientParams(OscClientParamsStore* pOscClientParamsStore): m_pOscClientParamsStore(pOscClientParamsStore) {
	memset(&m_tOscClientParams, 0, sizeof(struct TOscClientParams));
	m_tOscClientParams.nOutgoingPort = OSCCLIENT_DEFAULT_PORT_OUTGOING;
	m_tOscClientParams.nIncomingPort = OSCCLIENT_DEFAULT_PORT_INCOMING;
	m_tOscClientParams.nPingDelay = OSCCLIENT_DEFAULT_PING_DELAY_SECONDS;

	assert(sizeof(m_aCmd) > strlen(OscClientParamsConst::PARAMS_CMD));
	strncpy(m_aCmd, OscClientParamsConst::PARAMS_CMD, sizeof(m_aCmd));

	assert(sizeof(m_aLed) > strlen(OscClientParamsConst::PARAMS_LED));
	strncpy(m_aLed, OscClientParamsConst::PARAMS_LED, sizeof(m_aLed));
}

OscClientParams::~OscClientParams(void) {
	m_tOscClientParams.nSetList = 0;
}

bool OscClientParams::Load(void) {
	m_tOscClientParams.nSetList = 0;

	ReadConfigFile configfile(OscClientParams::staticCallbackFunction, this);

	if (configfile.Read(OscClientParamsConst::PARAMS_FILE_NAME)) {
		// There is a configuration file
		if (m_pOscClientParamsStore != 0) {
			m_pOscClientParamsStore->Update(&m_tOscClientParams);
		}
	} else if (m_pOscClientParamsStore != 0) {
		m_pOscClientParamsStore->Copy(&m_tOscClientParams);
	} else {
		return false;
	}

	return true;
}

void OscClientParams::Load(const char* pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pOscClientParamsStore != 0);

	if (m_pOscClientParamsStore == 0) {
		return;
	}

	m_tOscClientParams.nSetList = 0;

	ReadConfigFile config(OscClientParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pOscClientParamsStore->Update(&m_tOscClientParams);
}

void OscClientParams::callbackFunction(const char *pLine) {
	assert(pLine != 0);

	uint8_t nValue8;
	uint16_t nValue16;
	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, OscClientParamsConst::PARAMS_SERVER_IP, &nValue32) == SSCAN_OK) {
		m_tOscClientParams.nServerIp = nValue32;
		m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_SERVER_IP;
		return;
	}

	if (Sscan::Uint16(pLine, OscConst::PARAMS_OUTGOING_PORT, &nValue16) == SSCAN_OK) {
		if (nValue16 > 1023) {
			m_tOscClientParams.nOutgoingPort = nValue16;
			m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_OUTGOING_PORT;
		} else {
			m_tOscClientParams.nSetList &= ~OSCCLIENT_PARAMS_MASK_OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscConst::PARAMS_INCOMING_PORT, &nValue16) == SSCAN_OK) {
		if (nValue16 > 1023) {
			m_tOscClientParams.nIncomingPort = nValue16;
			m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_INCOMING_PORT;
		} else {
			m_tOscClientParams.nSetList &= ~OSCCLIENT_PARAMS_MASK_INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint8(pLine, OscClientParamsConst::PARAMS_PING_DISABLE, &nValue8) == SSCAN_OK) {
		m_tOscClientParams.nPingDisable = (nValue8 != 0);
		m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_PING_DISABLE;
		return;
	}

	if (Sscan::Uint8(pLine, OscClientParamsConst::PARAMS_PING_DELAY, &nValue8) == SSCAN_OK) {
		if ((nValue8 >= 2) && (nValue8 <= 60)) {
			m_tOscClientParams.nPingDelay = nValue8;
			m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_PING_DELAY;
		} else {
			m_tOscClientParams.nSetList &= ~OSCCLIENT_PARAMS_MASK_PING_DELAY;
		}
		return;
	}

	for (uint32_t i = 0; i < OSCCLIENT_PARAMS_CMD_MAX_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::PARAMS_CMD) - 1] = i + '0';
		nValue8 = OSCCLIENT_PARAMS_CMD_MAX_PATH_LENGTH;
		if (Sscan::Char(pLine, m_aCmd, reinterpret_cast<char*>(&m_tOscClientParams.aCmd[i]), &nValue8) == SSCAN_OK) {
			if (m_tOscClientParams.aCmd[i][0] == '/') {
				m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_CMD;
			} else {
				m_tOscClientParams.aCmd[i][0] = '\0';
			}
		}
	}

	for (uint32_t i = 0; i < OSCCLIENT_PARAMS_LED_MAX_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::PARAMS_LED) - 1] = i + '0';
		nValue8 = OSCCLIENT_PARAMS_LED_MAX_PATH_LENGTH;
		if (Sscan::Char(pLine, m_aLed, reinterpret_cast<char*>(&m_tOscClientParams.aLed[i]), &nValue8) == SSCAN_OK) {
			if (m_tOscClientParams.aLed[i][0] == '/') {
				m_tOscClientParams.nSetList |= OSCCLIENT_PARAMS_MASK_LED;
			} else {
				m_tOscClientParams.aLed[i][0] = '\0';
			}
		}
	}
}

void OscClientParams::Set(OscClient* pOscClient) {
	assert(pOscClient != 0);

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_SERVER_IP)) {
		pOscClient->SetServerIP(m_tOscClientParams.nServerIp);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_OUTGOING_PORT)) {
		pOscClient->SetPortOutgoing(m_tOscClientParams.nOutgoingPort);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_INCOMING_PORT)) {
		pOscClient->SetPortIncoming(m_tOscClientParams.nIncomingPort);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DISABLE)) {
		pOscClient->SetPingDisable(m_tOscClientParams.nPingDisable);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DELAY)) {
		pOscClient->SetPingDelay(m_tOscClientParams.nPingDelay);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_CMD)) {
		pOscClient->CopyCmds(reinterpret_cast<const char*>(&m_tOscClientParams.aCmd), OSCCLIENT_PARAMS_CMD_MAX_COUNT, OSCCLIENT_PARAMS_CMD_MAX_PATH_LENGTH);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_LED)) {
		pOscClient->CopyLeds(reinterpret_cast<const char*>(&m_tOscClientParams.aLed), OSCCLIENT_PARAMS_LED_MAX_COUNT, OSCCLIENT_PARAMS_LED_MAX_PATH_LENGTH);
	}
}

void OscClientParams::Dump(void) {
#ifndef NDEBUG
	if (m_tOscClientParams.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscClientParamsConst::PARAMS_FILE_NAME);

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_SERVER_IP)) {
		printf(" %s=" IPSTR "\n", OscClientParamsConst::PARAMS_SERVER_IP, IP2STR(m_tOscClientParams.nServerIp));
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_OUTGOING_PORT)) {
		printf(" %s=%d\n", OscConst::PARAMS_OUTGOING_PORT, m_tOscClientParams.nOutgoingPort);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_INCOMING_PORT)) {
		printf(" %s=%d\n", OscConst::PARAMS_INCOMING_PORT, m_tOscClientParams.nIncomingPort);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DISABLE)) {
		printf(" %s=%d [%s]\n", OscClientParamsConst::PARAMS_PING_DISABLE, m_tOscClientParams.nPingDisable, m_tOscClientParams.nPingDisable == 0 ? "No" : "Yes");
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DELAY)) {
		printf(" %s=%ds\n", OscClientParamsConst::PARAMS_PING_DELAY, m_tOscClientParams.nPingDelay);
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_CMD)) {
		for (uint32_t i = 0; i < OSCCLIENT_PARAMS_CMD_MAX_COUNT; i++) {
			m_aCmd[strlen(OscClientParamsConst::PARAMS_CMD) - 1] = i + '0';
			printf(" %s=[%s]\n", m_aCmd, reinterpret_cast<char*>(&m_tOscClientParams.aCmd[i]));
		}
	}

	if (isMaskSet(OSCCLIENT_PARAMS_MASK_LED)) {
		for (uint32_t i = 0; i < OSCCLIENT_PARAMS_LED_MAX_COUNT; i++) {
			m_aLed[strlen(OscClientParamsConst::PARAMS_LED) - 1] = i + '0';
			printf(" %s=[%s]\n", m_aLed, reinterpret_cast<char*>(&m_tOscClientParams.aLed[i]));
		}
	}
#endif
}

void OscClientParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	(static_cast<OscClientParams*>(p))->callbackFunction(s);
}
