/**
 * @file oscclientparams.cpp
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
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "oscclientparams.h"
#include "oscclientparamsconst.h"
#include "oscparamsconst.h"

#include "network.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

OscClientParams::OscClientParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct oscclientparams::Params));
	m_Params.nOutgoingPort = oscclient::defaults::PORT_OUTGOING;
	m_Params.nIncomingPort = oscclient::defaults::PORT_INCOMING;
	m_Params.nPingDelay = oscclient::defaults::PING_DELAY_SECONDS;

	assert(sizeof(m_aCmd) > strlen(OscClientParamsConst::CMD));
	strncpy(m_aCmd, OscClientParamsConst::CMD, sizeof(m_aCmd));

	assert(sizeof(m_aLed) > strlen(OscClientParamsConst::LED));
	strncpy(m_aLed, OscClientParamsConst::LED, sizeof(m_aLed));

	DEBUG_EXIT
}

void OscClientParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(OscClientParams::staticCallbackFunction, this);

	if (configfile.Read(OscClientParamsConst::FILE_NAME)) {
		OscClientParamsStore::Update(&m_Params);
	} else
#endif
	OscClientParamsStore::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OscClientParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(OscClientParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	OscClientParamsStore::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OscClientParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, OscClientParamsConst::SERVER_IP, nValue32) == Sscan::OK) {
		m_Params.nServerIp = nValue32;
		m_Params.nSetList |= oscclientparams::Mask::SERVER_IP;
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nOutgoingPort = nValue16;
			m_Params.nSetList |= oscclientparams::Mask::OUTGOING_PORT;
		} else {
			m_Params.nSetList &= ~oscclientparams::Mask::OUTGOING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nIncomingPort = nValue16;
			m_Params.nSetList |= oscclientparams::Mask::INCOMING_PORT;
		} else {
			m_Params.nSetList &= ~oscclientparams::Mask::INCOMING_PORT;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, OscClientParamsConst::PING_DISABLE, nValue8) == Sscan::OK) {
		m_Params.nPingDisable = (nValue8 != 0);
		m_Params.nSetList |= oscclientparams::Mask::PING_DISABLE;
		return;
	}

	if (Sscan::Uint8(pLine, OscClientParamsConst::PING_DELAY, nValue8) == Sscan::OK) {
		if ((nValue8 >= 2) && (nValue8 <= 60)) {
			m_Params.nPingDelay = nValue8;
			m_Params.nSetList |= oscclientparams::Mask::PING_DELAY;
		} else {
			m_Params.nSetList &= ~oscclientparams::Mask::PING_DELAY;
		}
		return;
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = static_cast<char>(i + '0');
		if (Sscan::Char(pLine, m_aCmd, reinterpret_cast<char*>(&m_Params.aCmd[i]), nValue32) == Sscan::OK) {
			if (m_Params.aCmd[i][0] == '/') {
				m_Params.nSetList |= oscclientparams::Mask::CMD;
			} else {
				m_Params.aCmd[i][0] = '\0';
			}
		}
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = static_cast<char>(i + '0');
		if (Sscan::Char(pLine, m_aLed, reinterpret_cast<char*>(&m_Params.aLed[i]), nValue32) == Sscan::OK) {
			if (m_Params.aLed[i][0] == '/') {
				m_Params.nSetList |= oscclientparams::Mask::LED;
			} else {
				m_Params.aLed[i][0] = '\0';
			}
		}
	}
}

void OscClientParams::Builder(const struct oscclientparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct oscclientparams::Params));
	} else {
		OscClientParamsStore::Copy(&m_Params);
	}

	PropertiesBuilder builder(OscClientParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddIpAddress(OscClientParamsConst::SERVER_IP, m_Params.nServerIp, isMaskSet(oscclientparams::Mask::SERVER_IP));
	builder.Add(OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort, isMaskSet(oscclientparams::Mask::OUTGOING_PORT));
	builder.Add(OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort, isMaskSet(oscclientparams::Mask::INCOMING_PORT));
	builder.Add(OscClientParamsConst::PING_DISABLE, m_Params.nPingDisable, isMaskSet(oscclientparams::Mask::PING_DISABLE));
	builder.Add(OscClientParamsConst::PING_DELAY, m_Params.nPingDelay, isMaskSet(oscclientparams::Mask::PING_DELAY));

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = static_cast<char>(i + '0');
		const char *cmd = reinterpret_cast<const char*>(&m_Params.aCmd[i]);
		builder.Add(m_aCmd, cmd, *cmd == '/');
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = static_cast<char>(i + '0');
		const char *led = reinterpret_cast<const char*>(&m_Params.aLed[i]);
		builder.Add(m_aLed, led, *led == '/');
	}

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void OscClientParams::Set(OscClient* pOscClient) {
	assert(pOscClient != nullptr);

	if (isMaskSet(oscclientparams::Mask::SERVER_IP)) {
		pOscClient->SetServerIP(m_Params.nServerIp);
	}

	if (isMaskSet(oscclientparams::Mask::OUTGOING_PORT)) {
		pOscClient->SetPortOutgoing(m_Params.nOutgoingPort);
	}

	if (isMaskSet(oscclientparams::Mask::INCOMING_PORT)) {
		pOscClient->SetPortIncoming(m_Params.nIncomingPort);
	}

	if (isMaskSet(oscclientparams::Mask::PING_DISABLE)) {
		pOscClient->SetPingDisable(m_Params.nPingDisable);
	}

	if (isMaskSet(oscclientparams::Mask::PING_DELAY)) {
		pOscClient->SetPingDelay(m_Params.nPingDelay);
	}

	if (isMaskSet(oscclientparams::Mask::CMD)) {
		pOscClient->CopyCmds(reinterpret_cast<const char*>(&m_Params.aCmd), oscclientparams::ParamsMax::CMD_COUNT, oscclientparams::ParamsMax::CMD_PATH_LENGTH);
	}

	if (isMaskSet(oscclientparams::Mask::LED)) {
		pOscClient->CopyLeds(reinterpret_cast<const char*>(&m_Params.aLed), oscclientparams::ParamsMax::LED_COUNT, oscclientparams::ParamsMax::LED_PATH_LENGTH);
	}
}

void OscClientParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OscClientParams*>(p))->callbackFunction(s);
}

void OscClientParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscClientParamsConst::FILE_NAME);

	if (isMaskSet(oscclientparams::Mask::SERVER_IP)) {
		printf(" %s=" IPSTR "\n", OscClientParamsConst::SERVER_IP, IP2STR(m_Params.nServerIp));
	}

	if (isMaskSet(oscclientparams::Mask::OUTGOING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);
	}

	if (isMaskSet(oscclientparams::Mask::INCOMING_PORT)) {
		printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);
	}

	if (isMaskSet(oscclientparams::Mask::PING_DISABLE)) {
		printf(" %s=%d [%s]\n", OscClientParamsConst::PING_DISABLE, m_Params.nPingDisable, m_Params.nPingDisable == 0 ? "No" : "Yes");
	}

	if (isMaskSet(oscclientparams::Mask::PING_DELAY)) {
		printf(" %s=%ds\n", OscClientParamsConst::PING_DELAY, m_Params.nPingDelay);
	}

	if (isMaskSet(oscclientparams::Mask::CMD)) {
		for (uint32_t i = 0; i < oscclientparams::ParamsMax::CMD_COUNT; i++) {
			m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = static_cast<char>(i + '0');
			printf(" %s=[%s]\n", m_aCmd, reinterpret_cast<char*>(&m_Params.aCmd[i]));
		}
	}

	if (isMaskSet(oscclientparams::Mask::LED)) {
		for (uint32_t i = 0; i < oscclientparams::ParamsMax::LED_COUNT; i++) {
			m_aLed[strlen(OscClientParamsConst::LED) - 1] = static_cast<char>(i + '0');
			printf(" %s=[%s]\n", m_aLed, reinterpret_cast<char*>(&m_Params.aLed[i]));
		}
	}
}
