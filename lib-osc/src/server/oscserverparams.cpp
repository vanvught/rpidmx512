/**
 * @file oscserverparams.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "oscserverparams.h"
#include "oscserverparamsconst.h"
#include "osc.h"
#include "oscparamsconst.h"

#include "lightsetparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "debug.h"

using namespace osc::server;

OSCServerParams::OSCServerParams() {
	DEBUG_ENTRY

	memset(&m_Params, 0, sizeof(struct Params));
	m_Params.nIncomingPort = osc::port::DEFAULT_INCOMING;
	m_Params.nOutgoingPort = osc::port::DEFAULT_OUTGOING;

	DEBUG_EXIT
}

void OSCServerParams::Load() {
	DEBUG_ENTRY

	m_Params.nSetList = 0;

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(OSCServerParams::staticCallbackFunction, this);

	if (configfile.Read(OscServerParamsConst::FILE_NAME)) {
		StoreOscServer::Update(&m_Params);
	} else
#endif
		StoreOscServer::Copy(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OSCServerParams::Load(const char* pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	m_Params.nSetList = 0;

	ReadConfigFile config(OSCServerParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	StoreOscServer::Update(&m_Params);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void OSCServerParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	uint16_t nValue16;

	if (Sscan::Uint16(pLine, OscParamsConst::INCOMING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nIncomingPort = nValue16;
			m_Params.nSetList |= ParamsMask::INCOMING_PORT;
		} else {
			m_Params.nIncomingPort = osc::port::DEFAULT_INCOMING;
			m_Params.nSetList &= ~ParamsMask::INCOMING_PORT;
		}
		return;
	}

	if (Sscan::Uint16(pLine, OscParamsConst::OUTGOING_PORT, nValue16) == Sscan::OK) {
		if (nValue16 > 1023) {
			m_Params.nOutgoingPort = nValue16;
			m_Params.nSetList |= ParamsMask::OUTGOING_PORT;
		} else {
			m_Params.nOutgoingPort = osc::port::DEFAULT_OUTGOING;
			m_Params.nSetList &= ~ParamsMask::OUTGOING_PORT;
		}
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pLine, OscServerParamsConst::TRANSMISSION, nValue8) == Sscan::OK) {
		if (nValue8 != 0) {
			m_Params.nSetList |= ParamsMask::TRANSMISSION;
		} else {
			m_Params.nSetList &= ~ParamsMask::TRANSMISSION;
		}
		m_Params.bPartialTransmission = (nValue8 != 0);
		return;
	}

	uint32_t nLength = sizeof(m_Params.aPath) - 1;
	if (Sscan::Char(pLine, OscServerParamsConst::PATH, m_Params.aPath, nLength) == Sscan::OK) {
		m_Params.nSetList |= ParamsMask::PATH;
		return;
	}

	nLength = sizeof(m_Params.aPathInfo) - 1;
	if (Sscan::Char(pLine, OscServerParamsConst::PATH_INFO, m_Params.aPathInfo, nLength) == Sscan::OK) {
		m_Params.nSetList |= ParamsMask::PATH_INFO;
		return;
	}

	nLength = sizeof(m_Params.aPathBlackOut) - 1;
	if (Sscan::Char(pLine, OscServerParamsConst::PATH_INFO, m_Params.aPathBlackOut, nLength) == Sscan::OK) {
		m_Params.nSetList |= ParamsMask::PATH_BLACKOUT;
		return;
	}
}

void OSCServerParams::Set(OscServer *pOscServer) {
	assert(pOscServer != nullptr);

	if (isMaskSet(ParamsMask::INCOMING_PORT)) {
		pOscServer->SetPortIncoming(m_Params.nIncomingPort);
	}

	if (isMaskSet(ParamsMask::OUTGOING_PORT)) {
		pOscServer->SetPortOutgoing(m_Params.nOutgoingPort);
	}

	if (isMaskSet(ParamsMask::PATH)) {
		pOscServer->SetPath(m_Params.aPath);
	}

	if (isMaskSet(ParamsMask::PATH_INFO)) {
		pOscServer->SetPathInfo(m_Params.aPathInfo);
	}

	if (isMaskSet(ParamsMask::PATH_BLACKOUT)) {
		pOscServer->SetPathBlackOut(m_Params.aPathBlackOut);
	}

	if (isMaskSet(ParamsMask::TRANSMISSION)) {
		pOscServer->SetPartialTransmission(m_Params.bPartialTransmission);
	}
}

void OSCServerParams::Builder(const osc::server::Params *ptOSCServerParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptOSCServerParams != nullptr) {
		memcpy(&m_Params, ptOSCServerParams, sizeof(osc::server::Params));
	} else {
		StoreOscServer::Copy(&m_Params);
	}

	PropertiesBuilder builder(OscServerParamsConst::FILE_NAME, pBuffer, nLength);

	if (!isMaskSet(ParamsMask::PATH)) {
		strncpy(m_Params.aPath, OscServer::Get()->GetPath(), sizeof(m_Params.aPath) - 1);
	}

	if (!isMaskSet(ParamsMask::PATH_INFO)) {
		strncpy(m_Params.aPathInfo, OscServer::Get()->GetPathInfo(), sizeof(m_Params.aPathInfo) - 1);
	}

	if (!isMaskSet(ParamsMask::PATH_BLACKOUT)) {
		strncpy(m_Params.aPathBlackOut, OscServer::Get()->GetPathBlackOut(), sizeof(m_Params.aPathBlackOut) - 1);
	}

	builder.Add(OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort, isMaskSet(ParamsMask::INCOMING_PORT));
	builder.Add(OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort, isMaskSet(ParamsMask::OUTGOING_PORT));
	builder.Add(OscServerParamsConst::PATH, m_Params.aPath, isMaskSet(ParamsMask::PATH));
	builder.Add(OscServerParamsConst::PATH_INFO, m_Params.aPathInfo, isMaskSet(ParamsMask::PATH_INFO));
	builder.Add(OscServerParamsConst::PATH_BLACKOUT, m_Params.aPathBlackOut, isMaskSet(ParamsMask::PATH_BLACKOUT));
	builder.Add(OscServerParamsConst::TRANSMISSION, m_Params.bPartialTransmission, isMaskSet(ParamsMask::TRANSMISSION));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void OSCServerParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<OSCServerParams*>(p))->callbackFunction(s);
}

void OSCServerParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, OscServerParamsConst::FILE_NAME);
	printf(" %s=%d\n", OscParamsConst::INCOMING_PORT, m_Params.nIncomingPort);
	printf(" %s=%d\n", OscParamsConst::OUTGOING_PORT, m_Params.nOutgoingPort);
	printf(" %s=%s\n", OscServerParamsConst::PATH, m_Params.aPath);
	printf(" %s=%s\n", OscServerParamsConst::PATH_INFO, m_Params.aPathInfo);
	printf(" %s=%s\n", OscServerParamsConst::PATH_BLACKOUT, m_Params.aPathBlackOut);
	printf(" %s=%d\n", OscServerParamsConst::TRANSMISSION, m_Params.bPartialTransmission);
}
