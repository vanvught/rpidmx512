/**
 * @file oscserverparamssave.cpp
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

#include "oscserverparms.h"
#include "oscserverconst.h"
#include "oscparamsconst.h"

#include "lightsetconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

void OSCServerParams::Builder(const struct TOSCServerParams *ptOSCServerParams, char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	if (ptOSCServerParams != nullptr) {
		memcpy(&m_tOSCServerParams, ptOSCServerParams, sizeof(struct TOSCServerParams));
	} else {
		m_pOSCServerParamsStore->Copy(&m_tOSCServerParams);
	}

	PropertiesBuilder builder(OSCServerConst::PARAMS_FILE_NAME, pBuffer, nLength);

	builder.Add(OscParamsConst::INCOMING_PORT, m_tOSCServerParams.nIncomingPort, isMaskSet(OSCServerParamsMask::INCOMING_PORT));
	builder.Add(OscParamsConst::OUTGOING_PORT, m_tOSCServerParams.nOutgoingPort, isMaskSet(OSCServerParamsMask::OUTGOING_PORT));
	builder.Add(OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath, isMaskSet(OSCServerParamsMask::PATH));
	builder.Add(OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, isMaskSet(OSCServerParamsMask::PATH_INFO));
	builder.Add(OSCServerConst::PARAMS_PATH_BLACKOUT, m_tOSCServerParams.aPathBlackOut, isMaskSet(OSCServerParamsMask::PATH_BLACKOUT));
	builder.Add(OSCServerConst::PARAMS_TRANSMISSION, m_tOSCServerParams.bPartialTransmission, isMaskSet(OSCServerParamsMask::TRANSMISSION));

	builder.Add(LightSetConst::PARAMS_ENABLE_NO_CHANGE_UPDATE, m_tOSCServerParams.bEnableNoChangeUpdate, isMaskSet(OSCServerParamsMask::ENABLE_NO_CHANGE_OUTPUT));

	nSize = builder.GetSize();

	DEBUG_EXIT
}

void OSCServerParams::Save(char *pBuffer, uint32_t nLength, uint32_t &nSize) {
	DEBUG_ENTRY

	if (m_pOSCServerParamsStore == nullptr) {
		nSize = 0;
		DEBUG_EXIT
		return;
	}

	Builder(nullptr, pBuffer, nLength, nSize);
}
