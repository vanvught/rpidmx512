/**
 * @file oscserverparamssave.cpp
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
#include <assert.h>

#include "oscserverparms.h"
#include "oscserverconst.h"

#include "lightset.h"
#include "lightsetconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

bool OSCServerParams::Builder(const struct TOSCServerParams *ptOSCServerParams, uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (ptOSCServerParams != 0) {
		memcpy(&m_tOSCServerParams, ptOSCServerParams, sizeof(struct TOSCServerParams));
	} else {
		m_pOSCServerParamsStore->Copy(&m_tOSCServerParams);
	}

	PropertiesBuilder builder(OSCServerConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.Add(LightSetConst::PARAMS_OUTPUT, LightSet::GetOutputType(m_tOSCServerParams.tOutputType), isMaskSet(OSCSERVER_PARAMS_MASK_OUTPUT));
	isAdded &= builder.Add(OSCServerConst::PARAMS_INCOMING_PORT, (uint32_t) m_tOSCServerParams.nIncomingPort, isMaskSet(OSCSERVER_PARAMS_MASK_INCOMING_PORT));
	isAdded &= builder.Add(OSCServerConst::PARAMS_OUTGOING_PORT, (uint32_t) m_tOSCServerParams.nOutgoingPort, isMaskSet(OSCSERVER_PARAMS_MASK_OUTGOING_PORT));
	isAdded &= builder.Add(OSCServerConst::PARAMS_PATH, m_tOSCServerParams.aPath, isMaskSet(OSCSERVER_PARAMS_MASK_PATH));
	isAdded &= builder.Add(OSCServerConst::PARAMS_PATH_INFO, m_tOSCServerParams.aPathInfo, isMaskSet(OSCSERVER_PARAMS_MASK_PATH_INFO));
	isAdded &= builder.Add(OSCServerConst::PARAMS_PATH_BLACKOUT, m_tOSCServerParams.aPathBlackOut, isMaskSet(OSCSERVER_PARAMS_MASK_PATH_BLACKOUT));
	isAdded &= builder.Add(OSCServerConst::PARAMS_TRANSMISSION, (uint32_t) m_tOSCServerParams.bPartialTransmission, isMaskSet(OSCSERVER_PARAMS_MASK_TRANSMISSION));

	nSize = builder.GetSize();

	DEBUG_EXIT
	return isAdded;
}

bool OSCServerParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pOSCServerParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
