/**
 * @file oscclientparams.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_OSCCLIENT)
# undef NDEBUG
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

#include "configstore.h"
#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

namespace oscclientparams::store {
static void Update(const struct oscclientparams::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::OSC_CLIENT, pParams, sizeof(struct oscclientparams::Params));
}

static void Copy(struct oscclientparams::Params *pParams) {
	ConfigStore::Get()->Copy(configstore::Store::OSC_CLIENT, pParams, sizeof(struct oscclientparams::Params));
}
}  // namespace oscclientparams::store

OscClientParams::OscClientParams() {
	DEBUG_ENTRY

	assert(sizeof(m_aCmd) > strlen(OscClientParamsConst::CMD));
	strncpy(m_aCmd, OscClientParamsConst::CMD, sizeof(m_aCmd));

	assert(sizeof(m_aLed) > strlen(OscClientParamsConst::LED));
	strncpy(m_aLed, OscClientParamsConst::LED, sizeof(m_aLed));

	DEBUG_EXIT
}

void OscClientParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(OscClientParams::StaticCallbackFunction, this);

	if (configfile.Read(OscClientParamsConst::FILE_NAME)) {
		oscclientparams::store::Update(&m_Params);
	} else
#endif
		oscclientparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OscClientParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(m_Params));

	ReadConfigFile config(OscClientParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	oscclientparams::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OscClientParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint32_t nValue32;

	if (Sscan::IpAddress(pLine, OscClientParamsConst::SERVER_IP, nValue32) == Sscan::OK) {
		m_Params.nServerIp = nValue32;
		return;
	}

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nOutgoingPort = nValue16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nIncomingPort = nValue16;
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
		}
		return;
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = static_cast<char>(i + '0');
		if (Sscan::Char(pLine, m_aCmd, reinterpret_cast<char *>(&m_Params.aCmd[i]), nValue32) == Sscan::OK) {
			if (m_Params.aCmd[i][0] == '/') {
				m_Params.nSetList |= oscclientparams::Mask::CMD;
			} else {
				m_Params.aCmd[i][0] = '\0';
			}
		}
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = static_cast<char>(i + '0');
		if (Sscan::Char(pLine, m_aLed, reinterpret_cast<char *>(&m_Params.aLed[i]), nValue32) == Sscan::OK) {
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

	auto& oscClient = OscClient::Get();

	if (pParams != nullptr) {
		memcpy(&m_Params, pParams, sizeof(struct oscclientparams::Params));
	} else {
		oscclientparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(OscClientParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddIpAddress(OscClientParamsConst::SERVER_IP, m_Params.nServerIp, IsMaskSet(oscclientparams::Mask::SERVER_IP));

	if (m_Params.nOutgoingPort == 0) {
		m_Params.nOutgoingPort = oscClient.GetPortOutgoing();
	}

	builder.Add(OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);

	if (m_Params.nIncomingPort == 0) {
		m_Params.nIncomingPort = oscClient.GetPortIncoming();
	}

	builder.Add(OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);

	builder.Add(OscClientParamsConst::PING_DISABLE, m_Params.nPingDisable, IsMaskSet(oscclientparams::Mask::PING_DISABLE));

	if (m_Params.nPingDelay == 0) {
		m_Params.nPingDelay = oscClient.GetPingDelaySeconds();
	}

	builder.Add(OscClientParamsConst::PING_DELAY, m_Params.nPingDelay);

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

void OscClientParams::Set() {
	auto& oscClient = OscClient::Get();

	oscClient.SetServerIP(m_Params.nServerIp);
	oscClient.SetPortOutgoing(m_Params.nOutgoingPort);
	oscClient.SetPortIncoming(m_Params.nIncomingPort);
	oscClient.SetPingDisable(m_Params.nPingDisable);
	oscClient.SetPingDelaySeconds(m_Params.nPingDelay);
	oscClient.CopyCmds(reinterpret_cast<const char *>(&m_Params.aCmd), oscclientparams::ParamsMax::CMD_COUNT, oscclientparams::ParamsMax::CMD_PATH_LENGTH);
	oscClient.CopyLeds(reinterpret_cast<const char *>(&m_Params.aLed), oscclientparams::ParamsMax::LED_COUNT, oscclientparams::ParamsMax::LED_PATH_LENGTH);
}

void OscClientParams::StaticCallbackFunction(void* p, const char* s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OscClientParams*>(p))->CallbackFunction(s);
}

void OscClientParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscClientParamsConst::FILE_NAME);

	printf(" %s=" IPSTR "\n", OscClientParamsConst::SERVER_IP, IP2STR(m_Params.nServerIp));
	printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);
	printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);
	printf(" %s=%d [%s]\n", OscClientParamsConst::PING_DISABLE, m_Params.nPingDisable, m_Params.nPingDisable == 0 ? "No" : "Yes");
	printf(" %s=%ds\n", OscClientParamsConst::PING_DELAY, m_Params.nPingDelay);

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = static_cast<char>(i + '0');
		printf(" %s=[%s]\n", m_aCmd, reinterpret_cast<char *>(&m_Params.aCmd[i]));
	}

	for (uint32_t i = 0; i < oscclientparams::ParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = static_cast<char>(i + '0');
		printf(" %s=[%s]\n", m_aLed, reinterpret_cast<char *>(&m_Params.aLed[i]));
	}
}
