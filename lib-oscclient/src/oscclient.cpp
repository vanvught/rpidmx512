/**
 * @file oscclient.cpp
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

#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include "oscclient.h"
#include "oscsimplesend.h"
#include "oscsimplemessage.h"
#include "osc.h"

#include "hardware.h"
#include "network.h"

#if defined(BARE_METAL) || defined (RASPPI)
 #include "display.h"
 #include "display7segment.h"
#endif

#include "debug.h"

namespace buffer {
namespace size {
static constexpr auto PATH = 32 + OscClientMax::LED_PATH_LENGTH;
static constexpr auto CMD = OscClientMax::CMD_COUNT * OscClientMax::CMD_PATH_LENGTH * sizeof(uint8_t);
static constexpr auto LED = OscClientMax::LED_COUNT * OscClientMax::LED_PATH_LENGTH * sizeof(uint8_t);
}  // namespace size
}  // namespace buffer

OscClient::OscClient() :
	m_nPortOutgoing(OscClientDefault::PORT_OUTGOING),
	m_nPortIncoming(OscClientDefault::PORT_INCOMING),
	m_nPingDelayMillis(OscClientDefault::PING_DELAY_SECONDS * 1000)
{
	m_pBuffer = new char[buffer::size::PATH];
	assert(m_pBuffer != nullptr);

	m_pCmds = new char[buffer::size::CMD];
	assert(m_pCmds != nullptr);
	memset(m_pCmds, 0, buffer::size::CMD);

	m_pLeds = new char[buffer::size::LED];
	assert(m_pLeds != nullptr);
	memset(m_pLeds, 0, buffer::size::LED);
}

OscClient::~OscClient() {
	Stop();
}

void OscClient::Start() {
	DEBUG_ENTRY

	m_nHandle = Network::Get()->Begin(m_nPortIncoming);
	assert(m_nHandle != -1);

	DEBUG_EXIT
}

void OscClient::Stop() {
	m_nHandle = Network::Get()->End(m_nPortIncoming);
}

int OscClient::Run() {
	if (!m_bPingDisable) {
		m_nCurrenMillis = Hardware::Get()->Millis();

		if ((m_nCurrenMillis - m_nPreviousMillis) >= m_nPingDelayMillis) {
			OscSimpleSend MsgSend(m_nHandle, m_nServerIP, m_nPortOutgoing, "/ping", nullptr);
			m_bPingSent = true;
			m_nPreviousMillis = m_nCurrenMillis;
			m_nPingTimeMillis = m_nCurrenMillis;
		}

		uint32_t nRemoteIp;
		uint16_t nRemotePort;
		m_nBytesReceived = Network::Get()->RecvFrom(m_nHandle, m_pBuffer, buffer::size::PATH, &nRemoteIp, &nRemotePort);

		if (__builtin_expect((m_nBytesReceived == 0), 1)) {
			if (m_bPingSent && ((m_nCurrenMillis - m_nPingTimeMillis) >= 1000)) {
				if (m_bPongReceived) {
					m_bPongReceived = false;
#if defined(BARE_METAL) || defined (RASPPI)
					Display::Get()->TextStatus("No /Pong", Display7SegmentMessage::ERROR_OSCCLIENT_PING_PONG);
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

		if ((m_pOscClientLed != nullptr) && (!HandleLedMessage())) {
			if (!OSC::isMatch(m_pBuffer, "/pong")) {
				DEBUG_PUTS(m_pBuffer);
				return 0;
			}
		}

		if (!m_bPongReceived) {
#if defined(BARE_METAL) || defined (RASPPI)
			Display::Get()->TextStatus("Ping-Pong", Display7SegmentMessage::INFO_OSCCLIENT_PING_PONG);
#endif
			DEBUG_PUTS("Ping-Pong");
		}

		m_bPongReceived = true;
		m_bPingSent = false;

		return m_nBytesReceived;
	}

	return 0;
}

void OscClient::Print() {
	printf("OSC Client\n");
	printf(" Server ip-address :" IPSTR "\n", IP2STR(m_nServerIP));
	printf(" Outgoing Port     : %d\n", m_nPortOutgoing);
	printf(" Incoming Port     : %d\n", m_nPortIncoming);
	printf(" Disable /ping     : %s\n", m_bPingDisable ? "Yes" : "No");

	if (!m_bPingDisable) {
		printf(" Ping delay        : %ds\n", m_nPingDelayMillis / 1000);
	}

	for (uint32_t i = 0; i < OscClientMax::CMD_COUNT; i++) {
		const char *p = &m_pCmds[i * OscClientMax::CMD_PATH_LENGTH];
		if (*p != '\0') {
			printf("  cmd%c             : [%s]\n", i + '0', p);
		}
	}

	for (uint32_t i = 0; i < OscClientMax::LED_COUNT; i++) {
		const char *p = &m_pLeds[i * OscClientMax::LED_PATH_LENGTH];
		if (*p != '\0') {
			printf("  led%c             : [%s]\n", i + '0', p);
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

void OscClient::SetPingDelay(uint32_t nPingDelay) {
	if ((nPingDelay >=2) && (nPingDelay <= 60)) {
		m_nPingDelayMillis = nPingDelay * 1000;
	}
}

void OscClient::CopyCmds(const char *pCmds, uint32_t nCount, uint32_t nLength) {
	assert(pCmds != nullptr);

	for (uint32_t i = 0; i < std::min(nCount, OscClientMax::CMD_COUNT); i++) {
		char *dst = &m_pCmds[i * OscClientMax::CMD_PATH_LENGTH];
		strncpy(dst, &pCmds[i * nLength], OscClientMax::CMD_PATH_LENGTH - 1);
		dst[OscClientMax::CMD_PATH_LENGTH - 1] = '\0';
	}
}

void OscClient::CopyLeds(const char *pLeds, uint32_t nCount, uint32_t nLength) {
	assert(pLeds != nullptr);

	for (uint32_t i = 0; i < std::min(nCount, OscClientMax::LED_COUNT); i++) {
		char *dst = &m_pLeds[i * OscClientMax::LED_PATH_LENGTH];
		strncpy(dst, &pLeds[i * nLength], OscClientMax::LED_PATH_LENGTH - 1);
		dst[OscClientMax::LED_PATH_LENGTH - 1] = '\0';
	}
}

void OscClient::SetLedHandler(OscClientLed *pOscClientLed) {
	assert(pOscClientLed != nullptr);

	m_pOscClientLed = pOscClientLed;
}

bool OscClient::HandleLedMessage() {
	DEBUG_ENTRY

	uint32_t i;

	for (i = 0; i < OscClientMax::LED_COUNT; i++) {
		const char *src = &m_pLeds[i * OscClientMax::LED_PATH_LENGTH];
		if (OSC::isMatch(m_pBuffer, src)) {
			DEBUG_PUTS("");
			break;
		}
	}

	if (i == OscClientMax::LED_COUNT) {
		DEBUG_EXIT
		return false;
	}

	OscSimpleMessage Msg(m_pBuffer, m_nBytesReceived);

	const int nArgc = Msg.GetArgc();

	if (nArgc != 1) {
		DEBUG_EXIT
		return false;
	}

	if (Msg.GetType(0) == osc::type::INT32) {
		m_pOscClientLed->SetLed(i, Msg.GetInt(0) != 0);
		DEBUG_PRINTF("%d", Msg.GetInt(0));
	} else if (Msg.GetType(0) == osc::type::FLOAT) {
		m_pOscClientLed->SetLed(i, Msg.GetFloat(0) != 0);
		DEBUG_PRINTF("%f", Msg.GetFloat(0));
	} else {
		return false;
	}

	DEBUG_EXIT
	return true;
}
