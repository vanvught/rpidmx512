/**
 * @file oscclientparamssave.cpp
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
#include <stdbool.h>
#include <assert.h>

#include "oscclientparams.h"
#include "oscclientparamsconst.h"

#include "propertiesbuilder.h"

#include "debug.h"

bool OscClientParams::Builder(const struct TOscClientParams* ptOscClientParams, uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != 0);

	if (ptOscClientParams != 0) {
		memcpy(&m_tOscClientParams, ptOscClientParams, sizeof(struct TOscClientParams));
	} else {
		m_pOscClientParamsStore->Copy(&m_tOscClientParams);
	}

	PropertiesBuilder builder(OscClientParamsConst::PARAMS_FILE_NAME, pBuffer, nLength);

	bool isAdded = builder.AddIpAddress(OscClientParamsConst::PARAMS_SERVER_IP, m_tOscClientParams.nServerIp, isMaskSet(OSCCLIENT_PARAMS_MASK_SERVER_IP));
	isAdded &= builder.Add(OscClientParamsConst::PARAMS_OUTGOING_PORT, (uint32_t) m_tOscClientParams.nOutgoingPort, isMaskSet(OSCCLIENT_PARAMS_MASK_OUTGOING_PORT));
	isAdded &= builder.Add(OscClientParamsConst::PARAMS_INCOMING_PORT, (uint32_t) m_tOscClientParams.nIncomingPort, isMaskSet(OSCCLIENT_PARAMS_MASK_INCOMING_PORT));
	isAdded &= builder.Add(OscClientParamsConst::PARAMS_PING_DISABLE, (uint32_t) m_tOscClientParams.nPingDisable, isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DISABLE));
	isAdded &= builder.Add(OscClientParamsConst::PARAMS_PING_DELAY, (uint32_t) m_tOscClientParams.nPingDelay, isMaskSet(OSCCLIENT_PARAMS_MASK_PING_DELAY));

	for (uint32_t i = 0; i < OSCCLIENT_PARAMS_CMD_MAX_COUNT; i++) {
		m_aCmd[strlen(OscClientParamsConst::PARAMS_CMD) - 1] = i + '0';
		const char *cmd = (const char *) &m_tOscClientParams.aCmd[i];
		isAdded &= builder.Add(m_aCmd, cmd, *cmd == '/');
	}

	nSize = builder.GetSize();

	DEBUG_EXIT
	return isAdded;
}

bool OscClientParams::Save(uint8_t* pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	if (m_pOscClientParamsStore == 0) {
		nSize = 0;
		DEBUG_EXIT
		return false;
	}

	return Builder(0, pBuffer, nLength, nSize);
}
