/**
 * @file oscclient.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCCLIENT_H_
#define OSCCLIENT_H_

#include <cstdint>
#include <cassert>

#include "osc.h"
#include "oscsimplesend.h"
#include "oscclientled.h"

#include "hardware.h"
#include "network.h"
#include "display.h"

#include "debug.h"

namespace oscclient {
static constexpr auto STORE = 944;				///< Configuration store in bytes
namespace defaults {
static constexpr auto PORT_OUTGOING = 8000;
static constexpr auto PORT_INCOMING = 9000;
static constexpr auto PING_DELAY_SECONDS = 10;
}  // namespace defaults
namespace max {
static constexpr uint32_t CMD_COUNT = 8;
static constexpr uint32_t CMD_PATH_LENGTH = 64;
static constexpr uint32_t LED_COUNT = 8;
static constexpr uint32_t LED_PATH_LENGTH = 48;
}  // namespace max

namespace buffer::size {
static constexpr uint32_t CMD = oscclient::max::CMD_COUNT * oscclient::max::CMD_PATH_LENGTH;
static constexpr uint32_t LED = oscclient::max::LED_COUNT * oscclient::max::LED_PATH_LENGTH;
} // namespace buffer::size

}  // namespace oscclient

class OscClient {
public:
	OscClient() :
		m_nPortOutgoing(oscclient::defaults::PORT_OUTGOING),
		m_nPortIncoming(oscclient::defaults::PORT_INCOMING),
		m_nPingDelayMillis(oscclient::defaults::PING_DELAY_SECONDS * 1000)
	{
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void Start() {
		DEBUG_ENTRY

		assert(m_nHandle == -1);
		m_nHandle = Network::Get()->Begin(m_nPortIncoming);
		assert(m_nHandle != -1);

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		assert(m_nHandle != -1);
		Network::Get()->End(m_nPortIncoming);
		m_nHandle = -1;

		DEBUG_EXIT
	}

	void Run() {
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

		const auto nBytesReceived = Network::Get()->RecvFrom(m_nHandle, reinterpret_cast<const void **>(&m_pBuffer), &nRemoteIp, &nRemotePort);

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

	void Send(const char *pPath);
	void SendCmd(uint32_t nCmd);

	void Print();

	void SetServerIP(uint32_t nServerIP) {
		m_nServerIP = nServerIP;
	}

	uint32_t GetServerIP() const {
		return m_nServerIP;
	}

	void SetPortOutgoing(uint16_t nPortOutgoing) {
		assert(nPortOutgoing > 1023);
		m_nPortOutgoing = nPortOutgoing;
	}

	uint16_t GetPortOutgoing() const {
		return m_nPortOutgoing;
	}

	void SetPortIncoming(uint16_t nPortIncoming) {
		assert(nPortIncoming > 1023);
		m_nPortIncoming = nPortIncoming;
	}

	uint16_t GetPortIncoming() const {
		return m_nPortIncoming;
	}

	void SetPingDisable(bool nPingDisable = true) {
		m_bPingDisable = nPingDisable;
	}

	bool GetPingDisable() const {
		return m_bPingDisable;
	}

	void SetPingDelay(uint32_t nPingDelay = oscclient::defaults::PING_DELAY_SECONDS) {
		if ((nPingDelay >=2) && (nPingDelay <= 60)) {
			m_nPingDelayMillis = nPingDelay * 1000;
		}
	}

	uint32_t GetPingDelay() const {
		return m_nPingDelayMillis / 1000U;
	}

	void CopyCmds(const char *pCmds, uint32_t nCount, uint32_t nLength);
	void CopyLeds(const char *pLeds, uint32_t nCount, uint32_t nLength);

	void SetLedHandler(OscClientLed *pOscClientLed) {
		assert(pOscClientLed != nullptr);
		m_pOscClientLed = pOscClientLed;
	}

private:
	bool HandleLedMessage(const uint16_t nBytesReceived);

private:
	uint16_t m_nPortOutgoing;
	uint16_t m_nPortIncoming;
	uint32_t m_nPingDelayMillis;
	uint32_t m_nServerIP { 0 };
	int32_t m_nHandle { -1 };
	uint32_t m_nCurrenMillis { 0 };
	uint32_t m_nPreviousMillis { 0 };
	uint32_t m_nPingTimeMillis { 0 };
	const char *m_pBuffer { nullptr };
	bool m_bPingDisable { false };
	bool m_bPingSent { false };
	bool m_bPongReceived { false };

	OscClientLed *m_pOscClientLed { nullptr };

	static inline char m_pCmds[oscclient::buffer::size::CMD];
	static inline char m_pLeds[oscclient::buffer::size::LED];
};

#endif /* OSCCLIENT_H_ */
