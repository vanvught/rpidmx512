/**
 * @file e131bridge.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <circle/util.h>
#include <circle/time.h>
#include <circle/timer.h>
#include <circle/net/socket.h>
#include <circle/net/ipaddress.h>
#include <circle/net/in.h>
#include <circle/logger.h>

static const char FromE131Bridge[] = "e131bridge";

#include "e131.h"
#include "e131bridge.h"

#include "blinktask.h"
#include "lightset.h"

static 	CIPAddress IPAddressFrom;

E131Bridge::E131Bridge(CNetSubSystem *pNet, CActLED *pActLED) :
		m_pNet(pNet),
		m_Socket(m_pNet, IPPROTO_UDP),
		m_pLightSet(0)
{

	m_pBlinkTask = new CBlinkTask (pActLED, 1);
}

E131Bridge::~E131Bridge(void) {
	if (m_pLightSet != 0) {
		m_pLightSet->Stop();
		m_pLightSet = 0;
	}
}

void E131Bridge::SetOutput(LightSet *pLightSet) {
	assert(pLightSet != 0);
	m_pLightSet = pLightSet;
}

int E131Bridge::HandlePacket(void) {
	const char *packet = (char *)&(m_E131Packet.E131);

	u16	nForeignPort;
	const int nBytesReceived = m_Socket.ReceiveFrom ((void *)packet, sizeof m_E131Packet.E131, MSG_DONTWAIT, &IPAddressFrom, &nForeignPort);

	if (nBytesReceived < 0) {
		CLogger::Get()->Write(FromE131Bridge, LogPanic, "Cannot receive");
		return nBytesReceived;
	}

	m_E131Packet.length = nBytesReceived;

	if (nBytesReceived == 0) {
		m_E131Packet.IPAddressFrom = 0;
		return 0;
	}

	m_E131Packet.IPAddressFrom = IPAddressFrom;

	return 0;
}
