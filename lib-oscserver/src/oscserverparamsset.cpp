/**
 * @file oscserverparamsset.cpp
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

#include <assert.h>

#include "oscserverparms.h"
#include "oscserver.h"

void OSCServerParams::Set(OscServer *pOscServer) {
	assert(pOscServer != 0);

	if (isMaskSet(OSCSERVER_PARAMS_MASK_INCOMING_PORT)) {
		pOscServer->SetPortIncoming(m_tOSCServerParams.nIncomingPort);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_OUTGOING_PORT)) {
		pOscServer->SetPortOutgoing(m_tOSCServerParams.nOutgoingPort);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH)) {
		pOscServer->SetPath(m_tOSCServerParams.aPath);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH_INFO)) {
		pOscServer->SetPathInfo(m_tOSCServerParams.aPathInfo);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_PATH_BLACKOUT)) {
		pOscServer->SetPathBlackOut(m_tOSCServerParams.aPathBlackOut);
	}

	if (isMaskSet(OSCSERVER_PARAMS_MASK_TRANSMISSION)) {
		pOscServer->SetPartialTransmission(m_tOSCServerParams.bPartialTransmission);
	}
}
