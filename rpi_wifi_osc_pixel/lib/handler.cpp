/**
 * @file handler.cpp
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "handler.h"

#include "pixeltype.h"
#include "ws28xxdmx.h"
#include "pixeldmxparams.h"

#include "oscsimplesend.h"

#include "debug.h"

Handler::Handler(WS28xxDmx *pWS28xxDmx):
	m_pWS28xxDmx(pWS28xxDmx),
	m_nCount(PixelConfiguration::Get().GetCount()),
	m_TypeString(const_cast<char*>(pixel::pixel_get_type(PixelConfiguration::Get().GetType())))
{
	DEBUG_ENTRY

	DEBUG_EXIT
}

void Handler::Info(int32_t nHandle, uint32_t nRemoteIp, uint16_t nPortOutgoing) {
	OscSimpleSend MsgSendLedType(nHandle, nRemoteIp, nPortOutgoing, "/info/ledtype", "s", m_TypeString);
	OscSimpleSend MsgSendLedCount(nHandle, nRemoteIp, nPortOutgoing, "/info/ledcount", "i", static_cast<int>(m_nCount));
}
