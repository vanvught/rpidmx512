/**
 * @file oscclient.cpp
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "oscclient.h"
#include "oscsend.h"
#include "osc.h"

#include "hardware.h"
#include "network.h"
#include "display.h"

#include "display7segment.h"

#include "debug.h"

#ifndef MIN
 #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define OSCCLIENT_BUFFER_SIZE 		64
#define OSCCLIENT_CMD_BUFFER_SIZE 	(OSCCLIENT_CMD_MAX_COUNT * OSCCLIENT_CMD_MAX_PATH_LENGTH * sizeof(uint8_t))

OscClient::OscClient(void):
	m_nServerIP(0),
	m_nPortOutgoing(OSCCLIENT_DEFAULT_PORT_OUTGOING),
	m_nPortIncoming(OSCCLIENT_DEFAULT_PORT_INCOMING),
	m_nHandle(-1),
	m_bPingDisable(false),
	m_nPingDelay(OSCCLIENT_DEFAULT_PING_DELAY),
	m_bPingSent(false),
	m_bPongReceived(false),
	m_nCurrentTime(0),
	m_nPreviousTime(0),
	m_nPingTime(0)
{
	m_pBuffer = new uint8_t[OSCCLIENT_BUFFER_SIZE];
	assert(m_pBuffer != 0);

	m_pCmds = new uint8_t[OSCCLIENT_CMD_BUFFER_SIZE];
	assert(m_pCmds != 0);

	memset((void *)m_pCmds, 0, OSCCLIENT_CMD_BUFFER_SIZE);
}

OscClient::~OscClient(void) {
	Stop();
}

void OscClient::Start(void) {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void OscClient::Stop(void) {
	m_nHandle = Network::Get()->End(m_nPortIncoming);
}

int OscClient::Run(void) {
	if (!m_bPingDisable) {
		m_nCurrentTime = Hardware::Get()->GetTime();

		if ((m_nCurrentTime - m_nPreviousTime) >= m_nPingDelay) {
			OSCSend MsgSend(m_nHandle, m_nServerIP, m_nPortOutgoing, "/ping", 0);
			m_bPingSent = true;
			m_nPreviousTime = m_nCurrentTime;
			m_nPingTime = m_nCurrentTime;
		}

		uint32_t nRemoteIp;
		uint16_t nRemotePort;
		const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, OSCCLIENT_BUFFER_SIZE, &nRemoteIp, &nRemotePort);

		if (__builtin_expect((nBytesReceived == 0), 1)) {
			if (m_bPingSent && ((m_nCurrentTime - m_nPingTime) >= 1)) {
				if (m_bPongReceived) {
					m_bPongReceived = false;
#if defined(BARE_METAL) || defined (RASPPI)
					Display::Get()->TextStatus("No /Pong", DISPLAY_7SEGMENT_MSG_ERROR_OSCCLIENT_PING_PONG);
#endif
					DEBUG_PUTS("No /Pong");
				}
			}
			return 0;
		}

		if (nRemoteIp != m_nServerIP) {
			DEBUG_PRINTF("Data not received from server " IPSTR , IP2STR(nRemoteIp));
			return 0;
		}

		if (!OSC::isMatch((const char*) m_pBuffer, "/pong")) {
			DEBUG_PUTS(m_pBuffer);
			return 0;
		}

		if (!m_bPongReceived) {
#if defined(BARE_METAL) || defined (RASPPI)
			Display::Get()->TextStatus("Ping-Pong", DISPLAY_7SEGMENT_MSG_INFO_OSCCLIENT_PING_PONG);
#endif
			DEBUG_PUTS("Ping-Pong");
		}

		m_bPongReceived = true;
		m_bPingSent = false;

		return nBytesReceived;
	}

	return 0;
}

void OscClient::Print(void) {
	printf("OSC Client configuration\n");
	printf(" Server ip-address :" IPSTR "\n", IP2STR(m_nServerIP));
	printf(" Outgoing Port     : %d\n", m_nPortOutgoing);
	printf(" Incoming Port     : %d\n", m_nPortIncoming);
	printf(" Disable /ping     : %s\n", m_bPingDisable ? "Yes" : "No");

	if (!m_bPingDisable) {
		printf(" Ping delay        : %ds\n", m_nPingDelay);
	}

	for (uint32_t i = 0; i < OSCCLIENT_CMD_MAX_COUNT; i++) {
		const char *p = (const char *) &m_pCmds[i * OSCCLIENT_CMD_MAX_PATH_LENGTH];
		if (*p != '\0') {
			printf("  cmd%c             : [%s]\n", i + '0', p);
		}
	}
}

void OscClient::SetPortOutgoing(uint16_t nPortOutgoing) {
	assert(nPortOutgoing > 1023);
	m_nPortOutgoing = nPortOutgoing;
}

void OscClient::SetPortIncoming(uint16_t nPortIncoming) {
	assert(nPortIncoming > 1023);
	m_nPortIncoming = nPortIncoming;
}

void OscClient::SetPingDelay(uint16_t nPingDelay) {
	if ((nPingDelay >=2) && (nPingDelay <= 60)) {
		m_nPingDelay = nPingDelay;
	}
}

void OscClient::CopyCmds(const uint8_t* pCmds, uint32_t nCount, uint32_t nLength) {
	assert(pCmds != 0);

	for (uint32_t i = 0; i < MIN(nCount, OSCCLIENT_CMD_MAX_COUNT); i++) {

		const uint8_t *src = &pCmds[i * nLength];
		uint8_t *dst = &m_pCmds[i * OSCCLIENT_CMD_MAX_PATH_LENGTH];

		strncpy((char *)dst, (const char *) src, OSCCLIENT_CMD_MAX_PATH_LENGTH);
		dst[OSCCLIENT_CMD_MAX_PATH_LENGTH - 1] = '\0';
	}
}
