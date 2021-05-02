/**
 * @file oscclientparamssave.cpp
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
#include <cassert>

#include "oscclientparams.h"
#include "oscclientparamsconst.h"
#include "oscparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

void OscClientParams::Builder(const struct TOscClientParams* ptOscClientParams, char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptOscClientParams != nullptr) {
		memcpy(&m_tOscClientParams, ptOscClientParams, sizeof(struct TOscClientParams));
	} else {
		m_pOscClientParamsStore->Copy(&m_tOscClientParams);
	}

	PropertiesBuilder builder(OscClientParamsConst::FILE_NAME, pBuffer, nLength);

	builder.AddIpAddress(OscClientParamsConst::SERVER_IP, m_tOscClientParams.nServerIp, isMaskSet(OscClientParamsMask::SERVER_IP));
	builder.Add(OscParamsConst::OUTGOING_PORT, m_tOscClientParams.nOutgoingPort, isMaskSet(OscClientParamsMask::OUTGOING_PORT));
	builder.Add(OscParamsConst::INCOMING_PORT, m_tOscClientParams.nIncomingPort, isMaskSet(OscClientParamsMask::INCOMING_PORT));
	builder.Add(OscClientParamsConst::PING_DISABLE, m_tOscClientParams.nPingDisable, isMaskSet(OscClientParamsMask::PING_DISABLE));
	builder.Add(OscClientParamsConst::PING_DELAY, m_tOscClientParams.nPingDelay, isMaskSet(OscClientParamsMask::PING_DELAY));

	for (uint32_t i = 0; i < OscClientParamsMax::CMD_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::CMD) - 1] = i + '0';
		const char *cmd = reinterpret_cast<const char*>(&m_tOscClientParams.aCmd[i]);
		builder.Add(m_aCmd, cmd, *cmd == '/');
	}

	for (uint32_t i = 0; i < OscClientParamsMax::LED_COUNT; i++) {
		m_aLed[strlen(OscClientParamsConst::LED) - 1] = i + '0';
		const char *led = reinterpret_cast<const char*>(&m_tOscClientParams.aLed[i]);
		builder.Add(m_aLed, led, *led == '/');
	}

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void OscClientParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pOscClientParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
