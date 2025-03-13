/**
 * @file oscserverparams.cpp
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined (DEBUG_OSCSERVER)
# undef NDEBUG
#endif

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif
#include <cassert>

#include "oscserverparams.h"
#include "oscserverparamsconst.h"
#include "osc.h"
#include "oscparamsconst.h"

#include "dmxnodeparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

namespace osc::serverparams::store {
static void Update(const struct osc::server::Params *pParams) {
	ConfigStore::Get()->Update(configstore::Store::OSC, pParams, sizeof(struct osc::server::Params));
}

static void Copy(struct osc::server::Params *pParams) {
	ConfigStore::Get()->Copy(configstore::Store::OSC, pParams, sizeof(struct osc::server::Params));
}
}  // namespace osc::serverparams::store

OSCServerParams::OSCServerParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void OSCServerParams::Load() {
	DEBUG_ENTRY

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(OSCServerParams::StaticCallbackFunction, this);

	if (configfile.Read(OscServerParamsConst::FILE_NAME)) {
		osc::serverparams::store::Update(&m_Params);
	} else
#endif
		osc::serverparams::store::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OSCServerParams::Load(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	memset(&m_Params, 0, sizeof(struct osc::server::Params));

	ReadConfigFile config(OSCServerParams::StaticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	osc::serverparams::store::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OSCServerParams::CallbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nIncomingPort = nValue16;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nOutgoingPort = nValue16;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, OscServerParamsConst::TRANSMISSION, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= osc::server::ParamsMask::PARTIAL_TRANSMISSION;
		}
		return;
	}

	uint32_t nLength = sizeof(m_Params.aPath) - 1;

	if (Sscan::Char(pLine, OscServerParamsConst::PATH, m_Params.aPath, nLength) == Sscan::OK) {
		m_Params.aPath[nLength] = '\0';
		return;
	}

	nLength = sizeof(m_Params.aPathInfo) - 1;

	if (Sscan::Char(pLine, OscServerParamsConst::PATH_INFO, m_Params.aPathInfo, nLength) == Sscan::OK) {
		m_Params.aPathInfo[nLength] = '\0';
		return;
	}

	nLength = sizeof(m_Params.aPathBlackOut) - 1;

	if (Sscan::Char(pLine, OscServerParamsConst::PATH_INFO, m_Params.aPathBlackOut, nLength) == Sscan::OK) {
		m_Params.aPathBlackOut[nLength] = '\0';
		return;
	}
}

void OSCServerParams::Set() {
	auto& oscServer = OscServer::Get();

	oscServer.SetPortIncoming(m_Params.nIncomingPort);
	oscServer.SetPortOutgoing(m_Params.nOutgoingPort);
	oscServer.SetPath(m_Params.aPath);
	oscServer.SetPathInfo(m_Params.aPathInfo);
	oscServer.SetPathBlackOut(m_Params.aPathBlackOut);
	oscServer.SetPartialTransmission(IsMaskSet(osc::server::ParamsMask::PARTIAL_TRANSMISSION));
}

void OSCServerParams::Builder(const osc::server::Params *ptOSCServerParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptOSCServerParams != nullptr) {
		memcpy(&m_Params, ptOSCServerParams, sizeof(osc::server::Params));
	} else {
		osc::serverparams::store::Copy(&m_Params);
	}

	PropertiesBuilder builder(OscServerParamsConst::FILE_NAME, pBuffer, nLength);

	auto& oscServer = OscServer::Get();

	if (m_Params.nIncomingPort == 0) {
		m_Params.nIncomingPort = oscServer.GetPortIncoming();
	}

	builder.Add(OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);

	if (m_Params.nOutgoingPort == 0) {
		m_Params.nOutgoingPort = oscServer.GetPortOutgoing();
	}

	builder.Add(OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);

	const auto isPathSet = (m_Params.aPath[0] != 0);

	if (!isPathSet) {
		strncpy(m_Params.aPath, oscServer.GetPath(), sizeof(m_Params.aPath) - 1);
	}

	builder.Add(OscServerParamsConst::PATH, m_Params.aPath, isPathSet);

	const auto isPathInfoSet = (m_Params.aPathInfo[0] != 0);

	if (!isPathInfoSet) {
		strncpy(m_Params.aPathInfo, oscServer.GetPathInfo(), sizeof(m_Params.aPathInfo) - 1);
	}

	builder.Add(OscServerParamsConst::PATH_INFO, m_Params.aPathInfo, isPathInfoSet);

	const auto isPathBlackOutSet = (m_Params.aPathBlackOut[0] != 0);

	if (!isPathBlackOutSet) {
		strncpy(m_Params.aPathBlackOut, oscServer.GetPathBlackOut(), sizeof(m_Params.aPathBlackOut) - 1);
	}

	builder.Add(OscServerParamsConst::PATH_BLACKOUT, m_Params.aPathBlackOut, isPathBlackOutSet);

	builder.Add(OscServerParamsConst::TRANSMISSION, IsMaskSet(osc::server::ParamsMask::PARTIAL_TRANSMISSION), IsMaskSet(osc::server::ParamsMask::PARTIAL_TRANSMISSION));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void OSCServerParams::StaticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OSCServerParams*>(p))->CallbackFunction(s);
}

void OSCServerParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscServerParamsConst::FILE_NAME);
	printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);
	printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);
	printf(" %s=%s\n", OscServerParamsConst::PATH, m_Params.aPath);
	printf(" %s=%s\n", OscServerParamsConst::PATH_INFO, m_Params.aPathInfo);
	printf(" %s=%s\n", OscServerParamsConst::PATH_BLACKOUT, m_Params.aPathBlackOut);
	printf(" %s=%d\n", OscServerParamsConst::TRANSMISSION, IsMaskSet(osc::server::ParamsMask::PARTIAL_TRANSMISSION));
}
