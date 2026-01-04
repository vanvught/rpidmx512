/**
 * @file showfileprotocolnodeartnet.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_
#define PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_

#include <cstdint>
#include <cstring>

#include "artnetnode.h"
#include "artnet.h"

 #include "firmware/debug/debug_debug.h"

class ShowFileProtocol {
public:
	ShowFileProtocol() {
		DEBUG_ENTRY();

		memcpy(m_ArtDmx.Id, artnet::kNodeId, sizeof(m_ArtDmx.Id));
		m_ArtDmx.OpCode = static_cast<uint16_t>(artnet::OpCodes::kOpDmx);
		m_ArtDmx.ProtVerHi = 0;
		m_ArtDmx.ProtVerLo = artnet::kProtocolRevision;

		DEBUG_EXIT();
	}

	void SetSynchronizationAddress([[maybe_unused]] const uint16_t synchronization_address) {
	}

	void Start() {
		DEBUG_ENTRY();

		DEBUG_EXIT();
	}

	void Stop() {
		DEBUG_ENTRY();

		ArtNetNode::Get()->SetRecordShowfile(false);

		DEBUG_EXIT();
	}

	void Record() {
		DEBUG_ENTRY();

		ArtNetNode::Get()->SetRecordShowfile(true);

		DEBUG_EXIT();
	}

	void DmxOut(const uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength) {
		memcpy(m_ArtDmx.data, pDmxData, nLength);

		if ((nLength & 0x1) == 0x1) {
			m_ArtDmx.data[nLength] = 0x00;
			nLength++;
		}

		m_ArtDmx.Sequence = m_nSequence++;
		m_ArtDmx.Physical = static_cast<uint8_t>(dmxnode::kMaxPorts + 1U);
		m_ArtDmx.PortAddress = nUniverse;
		m_ArtDmx.LengthHi = static_cast<uint8_t>((nLength & 0xFF00) >> 8);
		m_ArtDmx.Length = static_cast<uint8_t>(nLength & 0xFF);

		ArtNetNode::Get()->HandleShowFile(&m_ArtDmx);
	}

	void DmxSync() {
	}

	void DmxBlackout() {
	}

	void DmxMaster([[maybe_unused]] const uint32_t nMaster) {
	}

	void DoRunCleanupProcess([[maybe_unused]] bool bDoRun) {
	}

	void Run() {
	}

	bool IsSyncDisabled() {
		return false;
	}

	void Print() {}

private:
	artnet::ArtDmx m_ArtDmx;
	uint8_t m_nSequence { 0 };
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLNODEARTNET_H_ */
