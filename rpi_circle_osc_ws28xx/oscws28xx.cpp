/**
 * @file oscws28xx.cpp
 *
 */
/*
 * Circle - A C++ bare metal environment for Raspberry Pi
 * Copyright (C) 2014-2015  R. Stange <rsta2@o2online.de>
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

#include <circle/version.h>
#include <circle/machineinfo.h>
#include <circle/string.h>
#include <circle/util.h>
#include <circle/logger.h>
#include <assert.h>
#include <Properties/propertiesfile.h>

#include "oscws28xx.h"
#include "osc.h"
#include "oscsend.h"
#include "oscmessage.h"
#include "ws28xxstripe.h"

#define PORT_REMOTE	9000
#define PROPERTIES_FILE		"devices.txt"

static const char FromOscWS28xx[] = "oscws28xx";

static const char sLedTypes[3][8] = { "WS2801", "WS2812", "WS2812B" };

COSCWS28xx::COSCWS28xx (CNetSubSystem *pNetSubSystem, CDevice *pTarget, CFATFileSystem *pFileSystem, unsigned nLocalPort)
:	OSCServer (pNetSubSystem, nLocalPort),
	m_pNetSubSystem(pNetSubSystem),
	m_pTarget(pTarget),
	m_pLEDStripe (0),
	m_LEDType (WS2801),
	m_nLEDCount (170),
	m_Properties(PROPERTIES_FILE, pFileSystem),
	m_Blackout(FALSE)
{
	m_RGBColour[0] = 0;
	m_RGBColour[1] = 0;
	m_RGBColour[2] = 0;

	if (!m_Properties.Load()) {
		CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "Error loading properties from %s (line %u)", PROPERTIES_FILE, m_Properties.GetErrorLine());
	} else {
		const char *pType = m_Properties.GetString("led_type");
		if (pType == 0) {
			CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "No led_type configured");
			CLogger::Get ()->Write(FromOscWS28xx, LogNotice, "Using default type : WS2801");
		} else {
			if (strcmp(pType, sLedTypes[WS2801]) == 0) {
				m_LEDType = WS2801;
			} else if (strcmp(pType, sLedTypes[WS2812]) == 0) {
				m_LEDType = WS2812;
			} else if (strcmp(pType, sLedTypes[WS2812B]) == 0) {
				m_LEDType = WS2812B;
			} else {
				CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "Wrong led_type configured (%s)", pType);
			}
		}

		unsigned nLEDCount = m_Properties.GetNumber("led_count");
		if (nLEDCount == 0) {
			CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "No or wrong led_count configured");
			CLogger::Get ()->Write(FromOscWS28xx, LogNotice, "Using default led count : 170");
		} else {
			m_nLEDCount = nLEDCount;
		}
	}

	assert(m_pLEDStripe == 0);
	m_pLEDStripe = new CWS28XXStripe(m_LEDType, m_nLEDCount);
	assert(m_pLEDStripe != 0);

	m_pLEDStripe->Initialize();
}

COSCWS28xx::~COSCWS28xx(void) {
	m_pNetSubSystem = 0;
	delete m_pLEDStripe;
	m_pLEDStripe = 0;
}

void COSCWS28xx::MessageReceived(u8 *Buffer, int BytesReceived, CIPAddress *ForeignIP) {

	if (OSC::isMatch((const char*) Buffer, "/ping")) {
		OSCSend MsgSend(&m_Socket, ForeignIP, PORT_REMOTE, "/pong", 0);
	} else if (OSC::isMatch((const char*) Buffer, "/dmx1/blackout")) {
		OSCMessage Msg(Buffer, (unsigned) BytesReceived);
		m_Blackout = (unsigned)Msg.GetFloat(0) == 1;
		if (m_Blackout) {
			if (!m_pLEDStripe->Blackout()) {
				CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "LED stripe blackout failed!");
			}
		} else {
			if (!m_pLEDStripe->Update()) {
				CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "LED stripe update failed!");
			}
		}
	} else if (OSC::isMatch((const char*) Buffer, "/dmx1/*")) {
		OSCMessage Msg(Buffer, (unsigned) BytesReceived);

		const char *p = (const char *) Buffer + 6;
		const unsigned dmx_channel = (unsigned) (*p - '0');
		const unsigned dmx_value = (unsigned) Msg.GetFloat(0);

		const unsigned index = dmx_channel - 1;	// DMX channel starts with 1

		if (index < 3) {
			m_RGBColour[index] = dmx_value;
		}

		CString ColorMessage;
		ColorMessage.Format("\rR:%03u G:%03u B:%03u", (unsigned) m_RGBColour[0], (unsigned) m_RGBColour[1], (unsigned) m_RGBColour[2]);
		m_pTarget->Write(ColorMessage, ColorMessage.GetLength());

		for (unsigned j = 0; j < m_nLEDCount; j++) {
			m_pLEDStripe->SetLED(j, m_RGBColour[0], m_RGBColour[1], m_RGBColour[2]);
		}

		if (!m_Blackout) {
			if (!m_pLEDStripe->Update()) {
				CLogger::Get ()->Write(FromOscWS28xx, LogWarning, "LED stripe update failed!");
			}
		}
	} else if (OSC::isMatch((const char*) Buffer, "/2")) {
		OSCSend MsgSendInfo(&m_Socket, ForeignIP, PORT_REMOTE, "/info/os", "s", CIRCLE_NAME " " CIRCLE_VERSION_STRING);
		OSCSend MsgSendModel(&m_Socket, ForeignIP, PORT_REMOTE, "/info/model", "s", m_MachineInfo.GetMachineName());
		OSCSend MsgSendSoc(&m_Socket, ForeignIP, PORT_REMOTE, "/info/soc", "s", m_MachineInfo.GetSoCName());
		OSCSend MsgSendLedType(&m_Socket, ForeignIP, PORT_REMOTE, "/info/ledtype", "s", sLedTypes[m_LEDType]);
		OSCSend MsgSendLedCount(&m_Socket, ForeignIP, PORT_REMOTE, "/info/ledcount", "i", m_nLEDCount);
	}
}
