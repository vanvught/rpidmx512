/**
 * @file showfileosc.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef SHOWFILEOSC_H_
#define SHOWFILEOSC_H_

#if !defined (CONFIG_SHOWFILE_ENABLE_OSC)
# error This file should not be included
#endif

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "showfiledisplay.h"
#include "osc.h"

#include "network.h"

#include "debug.h"

namespace showfileosc {
static constexpr char CMD_PATH[] = "/showfile/";
static constexpr uint32_t PATH_LENGTH = sizeof(CMD_PATH) - 1;
static constexpr uint32_t MAX_CMD_LENGTH = 128;
static constexpr uint32_t MAX_FILES_ENTRIES = 10;
}  // namespace showfileosc

class ShowFileOSC {
public:
	ShowFileOSC(uint16_t nPortIncoming = osc::port::DEFAULT_INCOMING, uint16_t nPortOutgoing = osc::port::DEFAULT_OUTGOING) : m_nPortIncoming(nPortIncoming), m_nPortOutgoing(nPortOutgoing) {
		DEBUG_ENTRY

		assert(s_pThis == nullptr);
		s_pThis = this;

		m_nHandle = Network::Get()->Begin(m_nPortIncoming, StaticCallbackFunction);
		assert(m_nHandle != -1);

		DEBUG_EXIT
	}

	~ShowFileOSC() {
		DEBUG_ENTRY

		Network::Get()->End(m_nPortIncoming);

		DEBUG_EXIT
	}

	void Input(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		assert(pBuffer != nullptr);

		m_pBuffer = pBuffer;
		m_nRemoteIp = nFromIp;
		m_nBytesReceived = nSize;
		m_nRemotePort = nFromPort;

		if (memcmp(m_pBuffer, showfileosc::CMD_PATH, showfileosc::PATH_LENGTH) == 0) {
			Process();
		}
	}

	void Print() {
		puts("OSC Server");
		printf(" Path : [%s]\n", showfileosc::CMD_PATH);
		printf(" Incoming port : %u\n", m_nPortIncoming);
		printf(" Outgoing port : %u\n", m_nPortOutgoing);
	}

	void SetPortIncoming(const uint16_t nPortIncoming) {
		assert(m_nHandle == -1);
		m_nPortIncoming = nPortIncoming;
	}
	uint16_t GetPortIncoming() const {
		return m_nPortIncoming;
	}

	void SetPortOutgoing(const uint16_t nPortOutgoing) {
		m_nPortOutgoing = nPortOutgoing;
	}

	uint16_t GetPortOutgoing() const {
		return m_nPortOutgoing;
	}

private:
	void Process();
	void SendStatus();
	void ShowFiles();

	void static StaticCallbackFunction(const uint8_t *pBuffer, uint32_t nSize, uint32_t nFromIp, uint16_t nFromPort) {
		s_pThis->Input(pBuffer, nSize, nFromIp, nFromPort);
	}

private:
	uint16_t m_nPortIncoming;
	uint16_t m_nPortOutgoing;
	int32_t m_nHandle { -1 };
	const uint8_t *m_pBuffer { nullptr };
	uint32_t m_nRemoteIp { 0 };
	uint32_t m_nBytesReceived { 0 };
	uint16_t m_nRemotePort { 0 };
	static inline ShowFileOSC *s_pThis;
};

#endif /* SHOWFILEOSC_H_ */
