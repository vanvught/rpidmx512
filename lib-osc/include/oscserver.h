/**
 * @file oscserver.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef OSCSERVER_H_
#define OSCSERVER_H_

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "oscsimplesend.h"

#include "hardware.h"
#include "network.h"

#include "lightset.h"

#include "debug.h"


namespace osc::server {
struct DefaultPort {
	static constexpr auto INCOMING = 8000U;
	static constexpr auto OUTGOING = 9000U;
};

struct Max {
	static constexpr auto PATH_LENGTH = 128U;
};
} // namespace osc::server


class OscServerHandler {
public:
	virtual ~OscServerHandler() {}
	virtual void Blackout()=0;
	virtual void Update()=0;
	virtual void Info(int32_t nHandle, uint32_t nRemoteIp, uint16_t nPortOutgoing)=0;
};

class OscServer {
public:
	OscServer();

	void Start() {
		DEBUG_ENTRY

		assert(m_nHandle == -1);
		m_nHandle = Network::Get()->Begin(m_nPortIncoming, StaticCallbackFunction);
		assert(m_nHandle != -1);

		Hardware::Get()->SetMode(hardware::ledblink::Mode::NORMAL);

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		if (m_pLightSet != nullptr) {
			m_pLightSet->Stop(0);
		}

		assert(m_nHandle != -1);
		Network::Get()->End(m_nPortIncoming);
		m_nHandle = -1;

		DEBUG_EXIT
	}

	void Print() {
		puts("OSC Server");
		printf(" Incoming Port        : %d\n", m_nPortIncoming);
		printf(" Outgoing Port        : %d\n", m_nPortOutgoing);
		printf(" DMX Path             : [%s][%s]\n", s_aPath, s_aPathSecond);
		printf("  Blackout Path       : [%s]\n", s_aPathBlackOut);
		printf(" Partial Transmission : %s\n", m_bPartialTransmission ? "Yes" : "No");
	}

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort);

	void SetOutput(LightSet *pLightSet) {
		assert(pLightSet != nullptr);
		m_pLightSet = pLightSet;
	}

	void SetOscServerHandler(OscServerHandler *pOscServerHandler) {
		assert(pOscServerHandler != nullptr);
		m_pOscServerHandler = pOscServerHandler;
	}

	void SetPortIncoming(const uint16_t nPortIncoming = osc::server::DefaultPort::INCOMING) {
		assert(nPortIncoming > 1023);
		m_nPortIncoming = nPortIncoming;
	}

	uint16_t GetPortIncoming() const {
		return m_nPortIncoming;
	}

	void SetPortOutgoing(const uint16_t nPortOutgoing) {
		assert(nPortOutgoing > 1023);
		m_nPortOutgoing = nPortOutgoing;
	}

	uint16_t GetPortOutgoing() const {
		return m_nPortOutgoing;
	}

	void SetPath(const char *pPath);

	const char*GetPath() {
		return s_aPath;
	}

	void SetPathInfo(const char *pPathInfo);

	const char*GetPathInfo() {
		return s_aPathInfo;
	}

	void SetPathBlackOut(const char *pPathBlackOut);

	const char*GetPathBlackOut() {
		return s_aPathBlackOut;
	}

	void SetPartialTransmission(bool bPartialTransmission = false) {
		m_bPartialTransmission = bPartialTransmission;
	}

	bool IsPartialTransmission() const {
		return m_bPartialTransmission;
	}

	void SetEnableNoChangeUpdate(bool bEnableNoChangeUpdate) {
		m_bEnableNoChangeUpdate = bEnableNoChangeUpdate;
	}
	bool GetEnableNoChangeUpdate() {
		return m_bEnableNoChangeUpdate;
	}

	static OscServer *Get() {
		return s_pThis;
	}

private:
	int GetChannel(const char *p);
	bool IsDmxDataChanged(const uint8_t *pData, uint16_t nStartChannel, uint32_t nLength);

	/**
	 * @brief Static callback function for receiving UDP packets.
	 *
	 * @param pBuffer Pointer to the packet buffer.
	 * @param nSize Size of the packet buffer.
	 * @param nFromIp IP address of the sender.
	 * @param nFromPort Port number of the sender.
	 */
	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	uint16_t m_nPortIncoming { osc::server::DefaultPort::INCOMING };
	uint16_t m_nPortOutgoing { osc::server::DefaultPort::OUTGOING };
	int32_t m_nHandle { -1 };
	uint32_t m_nLastChannel { 0 };

	bool m_bPartialTransmission { false };
	bool m_bEnableNoChangeUpdate { false };
	bool m_bIsRunning { false };
	char m_Os[32];

	OscServerHandler *m_pOscServerHandler { nullptr };
	LightSet *m_pLightSet { nullptr };

	const char *m_pModel;
	const char *m_pSoC;

	static inline char s_aPath[osc::server::Max::PATH_LENGTH];
	static inline char s_aPathSecond[osc::server::Max::PATH_LENGTH];
	static inline char s_aPathInfo[osc::server::Max::PATH_LENGTH];
	static inline char s_aPathBlackOut[osc::server::Max::PATH_LENGTH];

	static inline uint8_t s_pData[lightset::dmx::UNIVERSE_SIZE];
	static inline uint8_t s_pOsc[lightset::dmx::UNIVERSE_SIZE];

	static inline OscServer *s_pThis;
};

#endif /* OSCSERVER_H_ */
