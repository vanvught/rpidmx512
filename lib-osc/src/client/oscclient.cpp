/**
 * @file oscclient.cpp
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "oscclient.h"
#include "oscsimplesend.h"
#include "oscsimplemessage.h"
#include "osc.h"

#include "hardware.h"
#include "network.h"
#include "display.h"

#include "debug.h"

char OscClient::m_pCmds[oscclient::buffer::size::CMD];
char OscClient::m_pLeds[oscclient::buffer::size::LED];

OscClient::OscClient() :
	m_nPortOutgoing(oscclient::defaults::PORT_OUTGOING),
	m_nPortIncoming(oscclient::defaults::PORT_INCOMING),
	m_nPingDelayMillis(oscclient::defaults::PING_DELAY_SECONDS * 1000)
{
	DEBUG_ENTRY

	DEBUG_EXIT
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

void OscClient::Run() {
	if (!m_bPingDisable) {
		m_nCurrenMillis = Hardware::Get()->Millis();

		if ((m_nCurrenMillis - m_nPreviousMillis) >= m_nPingDelayMillis) {
			OscSimpleSend MsgSend(m_nHandle, m_nServerIP, m_nPortOutgoing, "/ping", nullptr);
			m_bPingSent = true;
			m_nPreviousMillis = m_nCurrenMillis;
			m_nPingTimeMillis = m_nCurrenMillis;
			DEBUG_PUTS("Ping sent");
		}
	}

	uint32_t nRemoteIp;
	uint16_t nRemotePort;

	const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, const_cast<const void **>(reinterpret_cast<void **>(&m_pBuffer)), &nRemoteIp, &nRemotePort);

	if (__builtin_expect((nBytesReceived == 0), 1)) {
		if (m_bPingSent && ((m_nCurrenMillis - m_nPingTimeMillis) >= 1000)) {
			if (m_bPongReceived) {
				m_bPongReceived = false;
				Display::Get()->TextStatus("No /Pong");
				DEBUG_PUTS("No /Pong");
			}
		}
		return;
	}

	if (nRemoteIp != m_nServerIP) {
		DEBUG_PRINTF("Data not received from server " IPSTR , IP2STR(nRemoteIp));
		return;
	}

	if ((m_pOscClientLed != nullptr) && (HandleLedMessage(nBytesReceived))) {
		DEBUG_EXIT
		return;
	}


	if (!m_bPingDisable) {
		if (!osc::is_match(m_pBuffer, "/pong")) {
			DEBUG_PUTS(m_pBuffer);
			return;
		}


		if (!m_bPongReceived) {
			Display::Get()->TextStatus("Ping-Pong");
			DEBUG_PUTS("Ping-Pong");
		}

		m_bPongReceived = true;
		m_bPingSent = false;
	}
}

void OscClient::CopyCmds(const char *pCmds, uint32_t nCount, uint32_t nLength) {
	assert(pCmds != nullptr);

	for (uint32_t i = 0; i < std::min(nCount, oscclient::max::CMD_COUNT); i++) {
		char *dst = &m_pCmds[i * oscclient::max::CMD_PATH_LENGTH];
		strncpy(dst, &pCmds[i * nLength], oscclient::max::CMD_PATH_LENGTH - 1);
		dst[oscclient::max::CMD_PATH_LENGTH - 1] = '\0';
	}
}

void OscClient::CopyLeds(const char *pLeds, uint32_t nCount, uint32_t nLength) {
	assert(pLeds != nullptr);

	for (uint32_t i = 0; i < std::min(nCount, oscclient::max::LED_COUNT); i++) {
		char *dst = &m_pLeds[i * oscclient::max::LED_PATH_LENGTH];
		strncpy(dst, &pLeds[i * nLength], oscclient::max::LED_PATH_LENGTH - 1);
		dst[oscclient::max::LED_PATH_LENGTH - 1] = '\0';
	}
}

bool OscClient::HandleLedMessage(const uint16_t nBytesReceived) {
	DEBUG_ENTRY

	uint32_t i;

	for (i = 0; i < oscclient::max::LED_COUNT; i++) {
		const char *src = &m_pLeds[i * oscclient::max::LED_PATH_LENGTH];
		if (osc::is_match(m_pBuffer, src)) {
			DEBUG_PUTS("");
			break;
		}
	}

	if (i == oscclient::max::LED_COUNT) {
		DEBUG_EXIT
		return false;
	}

	OscSimpleMessage Msg(m_pBuffer, nBytesReceived);

	const int nArgc = Msg.GetArgc();

	if (nArgc != 1) {
		DEBUG_EXIT
		return false;
	}

	if (Msg.GetType(0) == osc::type::INT32) {
		m_pOscClientLed->SetLed(static_cast<uint8_t>(i), static_cast<uint8_t>(Msg.GetInt(0)) != 0);
		DEBUG_PRINTF("%d", Msg.GetInt(0));
	} else if (Msg.GetType(0) == osc::type::FLOAT) {
		m_pOscClientLed->SetLed(static_cast<uint8_t>(i), static_cast<uint8_t>(Msg.GetFloat(0)) != 0);
		DEBUG_PRINTF("%f", Msg.GetFloat(0));
	} else {
		return false;
	}

	DEBUG_EXIT
	return true;
}

void OscClient::Print() {
	puts("OSC Client");
	printf(" Server ip-address :" IPSTR "\n", IP2STR(m_nServerIP));
	printf(" Outgoing Port     : %d\n", m_nPortOutgoing);
	printf(" Incoming Port     : %d\n", m_nPortIncoming);
	printf(" Disable /ping     : %s\n", m_bPingDisable ? "Yes" : "No");

	if (!m_bPingDisable) {
		printf(" Ping delay        : %ds\n", m_nPingDelayMillis / 1000);
	}

	for (uint32_t i = 0; i < oscclient::max::CMD_COUNT; i++) {
		const char *p = &m_pCmds[i * oscclient::max::CMD_PATH_LENGTH];
		if (*p != '\0') {
			printf("  cmd%c             : [%s]\n", i + '0', p);
		}
	}

	for (uint32_t i = 0; i < oscclient::max::LED_COUNT; i++) {
		const char *p = &m_pLeds[i * oscclient::max::LED_PATH_LENGTH];
		if (*p != '\0') {
			printf("  led%c             : [%s]\n", i + '0', p);
		}
	}
}
