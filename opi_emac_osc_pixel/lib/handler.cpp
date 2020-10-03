/**
 * @file handler.cpp
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
#include <assert.h>

#include "handler.h"

#include "ws28xx.h"
#include "ws28xxdmx.h"
#include "ws28xxdmxparams.h"

#include "oscsimplesend.h"

#include "debug.h"

Handler::Handler(WS28xxDmx *pWS28xxDmx):
	m_pWS28xxDmx(pWS28xxDmx),
	m_nLedCount(pWS28xxDmx->GetLEDCount()),
	m_pLedTypeString(const_cast<char*>(WS28xx::GetLedTypeString(pWS28xxDmx->GetLEDType())))
{
	DEBUG_ENTRY

	DEBUG_EXIT
}

void Handler::Blackout() {
	DEBUG_ENTRY

	m_pWS28xxDmx->Blackout(true);

	DEBUG_EXIT
}

void Handler::Update() {
	DEBUG_ENTRY

	m_pWS28xxDmx->Blackout(false);

	DEBUG_EXIT
}

void Handler::Info(int32_t nHandle, uint32_t nRemoteIp, uint16_t nPortOutgoing) {
	DEBUG_ENTRY

	OscSimpleSend MsgSendLedType(nHandle, nRemoteIp, nPortOutgoing, "/info/ledtype", "s", m_pLedTypeString);
	OscSimpleSend MsgSendLedCount(nHandle, nRemoteIp, nPortOutgoing, "/info/ledcount", "i", m_nLedCount);

	DEBUG_EXIT
}
