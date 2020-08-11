/**
 * @file handlertlc59711.cpp
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

#include "handlertlc59711.h"

#include "tlc59711dmx.h"
#include "tlc59711dmxparams.h"

#include "oscsimplesend.h"

#include "debug.h"

HandlerTLC59711::HandlerTLC59711(TLC59711Dmx *pTLC59711Dmx):
	m_pTLC59711Dmx(pTLC59711Dmx),
	m_nLedCount(pTLC59711Dmx->GetLEDCount()),
	m_pLedTypeString(const_cast<char*>(TLC59711DmxParams::GetLedTypeString(pTLC59711Dmx->GetLEDType())))
{
	DEBUG_ENTRY

	DEBUG_EXIT
}

void HandlerTLC59711::Blackout() {
	DEBUG_ENTRY

	m_pTLC59711Dmx->Blackout(true);

	DEBUG_EXIT
}

void HandlerTLC59711::Update() {
	DEBUG_ENTRY

	m_pTLC59711Dmx->Blackout(false);

	DEBUG_EXIT
}

void HandlerTLC59711::Info(int32_t nHandle, uint32_t nRemoteIp, uint16_t nPortOutgoing) {
	DEBUG_ENTRY

	OscSimpleSend MsgSendLedType(nHandle, nRemoteIp, nPortOutgoing, "/info/ledtype", "s", m_pLedTypeString);
	OscSimpleSend MsgSendLedCount(nHandle, nRemoteIp, nPortOutgoing, "/info/ledcount", "i", m_nLedCount);

	DEBUG_EXIT
}
