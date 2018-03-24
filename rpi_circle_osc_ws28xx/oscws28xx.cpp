/**
 * @file oscws28xx.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "circle/version.h"
#include "circle/machineinfo.h"
#include "circle/string.h"
#include "circle/util.h"
#include "circle/logger.h"

#include "networkcircle.h"

#include "ws28xxstripeparams.h"
#include "ws28xxstripe.h"

#include "oscws28xx.h"
#include "osc.h"
#include "oscsend.h"
#include "oscmessage.h"

COSCWS28xx::COSCWS28xx(CInterruptSystem *pInterrupt, CDevice *pTarget, unsigned nRemotePort) :
		m_pInterrupt(pInterrupt),
		m_pTarget(pTarget),
		m_nRemotePort(nRemotePort),
		m_pLEDStripe(0),
		m_LEDType(WS2801),
		m_nLEDCount(170),
		m_Blackout(FALSE)
{
	if (m_DeviceParams.Load()) {
		m_DeviceParams.Dump();
		m_LEDType = m_DeviceParams.GetLedType();
		m_nLEDCount = m_DeviceParams.GetLedCount();
	}

	for (unsigned i = 0; i < sizeof m_RGBWColour; i++) {
		m_RGBWColour[i] = 0;
	}

}

COSCWS28xx::~COSCWS28xx(void) {
	Stop();
}

void COSCWS28xx::Start(void) {
	assert(m_pLEDStripe == 0);
	m_pLEDStripe = new WS28XXStripe(m_pInterrupt, m_LEDType, m_nLEDCount);
	assert(m_pLEDStripe != 0);

	m_pLEDStripe->Initialize();
	m_pLEDStripe->Blackout();
}

void COSCWS28xx::Stop(void) {
	m_pLEDStripe->Blackout();

	delete m_pLEDStripe;
	m_pLEDStripe = 0;

	m_Blackout = true;
}

void COSCWS28xx::Run(void) {
	uint16_t from_port;
	uint32_t from_ip;

	const int len = Network::Get()->RecvFrom((const uint8_t *) m_packet, (const uint16_t) FRAME_BUFFER_SIZE, &from_ip, &from_port);

	if (len == 0) {
		return;
	}

	if (OSC::isMatch((const char*) m_packet, "/ping")) {
		OSCSend MsgSend(from_ip, m_nRemotePort, "/pong", 0);
	} else if (OSC::isMatch((const char*) m_packet, "/dmx1/blackout")) {
		OSCMessage Msg(m_packet, (unsigned) len);
		m_Blackout = (unsigned)Msg.GetFloat(0) == 1;

		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}

		if (m_Blackout) {
			m_pLEDStripe->Blackout();
		} else {
			m_pLEDStripe->Update();
		}
	} else if (OSC::isMatch((const char*) m_packet, "/dmx1/*")) {
		OSCMessage Msg(m_packet, (unsigned) len);

		const char *p = (const char *) m_packet + 6;
		const unsigned dmx_channel = (unsigned) (*p - '0');
		const unsigned dmx_value = (unsigned) Msg.GetFloat(0);

		const unsigned index = dmx_channel - 1;	// DMX channel starts with 1

		if (index < 4) {
			m_RGBWColour[index] = dmx_value;
		}

		CString ColorMessage;
		if (m_LEDType == SK6812W) {
			ColorMessage.Format("\rR:%03u G:%03u B:%03u W:%03u", (unsigned) m_RGBWColour[0], (unsigned) m_RGBWColour[1], (unsigned) m_RGBWColour[2], (unsigned) m_RGBWColour[3]);
		} else {
			ColorMessage.Format("\rR:%03u G:%03u B:%03u", (unsigned) m_RGBWColour[0], (unsigned) m_RGBWColour[1], (unsigned) m_RGBWColour[2]);
		}
		m_pTarget->Write(ColorMessage, ColorMessage.GetLength());

		while (m_pLEDStripe->IsUpdating()) {
			// wait for completion
		}

		if (m_LEDType == SK6812W) {
			for (unsigned j = 0; j < m_nLEDCount; j++) {
				m_pLEDStripe->SetLED(j, m_RGBWColour[0], m_RGBWColour[1], m_RGBWColour[2], m_RGBWColour[3]);
			}
		} else {
			for (unsigned j = 0; j < m_nLEDCount; j++) {
				m_pLEDStripe->SetLED(j, m_RGBWColour[0], m_RGBWColour[1], m_RGBWColour[2]);
			}
		}

		if (!m_Blackout) {
			m_pLEDStripe->Update();
		}
	} else if (OSC::isMatch((const char*) m_packet, "/2")) {
		OSCSend MsgSendModel(from_ip, m_nRemotePort, "/info/model", "s", m_MachineInfo.GetMachineName());
		OSCSend MsgSendSoc(from_ip, m_nRemotePort, "/info/soc", "s", m_MachineInfo.GetSoCName());
		OSCSend MsgSendInfo(from_ip, m_nRemotePort, "/info/os", "s", CIRCLE_NAME " " CIRCLE_VERSION_STRING);
		OSCSend MsgSendLedType(from_ip, m_nRemotePort, "/info/ledtype", "s", WS28XXStripeParams::GetLedTypeString(m_LEDType));
		OSCSend MsgSendLedCount(from_ip, m_nRemotePort, "/info/ledcount", "i", m_nLEDCount);
	}
}
