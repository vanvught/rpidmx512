/**
 * @file showfileprotocolnodee131.h
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_
#define PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_

#include <cstdint>
#include <cstring>

#include "e131bridge.h"
#include "e131.h"
#include "e117const.h"

#include "hardware.h"

#include "debug.h"

class ShowFileProtocol {
public:
	ShowFileProtocol() {
		DEBUG_ENTRY
		// Root Layer (See Section 5)
		m_E131DataPacket.RootLayer.PreAmbleSize = __builtin_bswap16(0x0010);
		m_E131DataPacket.RootLayer.PostAmbleSize = __builtin_bswap16(0x0000);
		memcpy(m_E131DataPacket.RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, e117::PACKET_IDENTIFIER_LENGTH);
		m_E131DataPacket.RootLayer.Vector = __builtin_bswap32(e131::vector::root::DATA);
		Hardware::Get()->GetUuid(m_E131DataPacket.RootLayer.Cid);
		// E1.31 Framing Layer (See Section 6)
		m_E131DataPacket.FrameLayer.Vector = __builtin_bswap32(e131::vector::data::PACKET);
		memcpy(m_E131DataPacket.FrameLayer.SourceName, E131Bridge::Get()->GetSourceName(), e131::SOURCE_NAME_LENGTH);
		m_E131DataPacket.FrameLayer.SynchronizationAddress = __builtin_bswap16(0);
		m_E131DataPacket.FrameLayer.Priority = e131::priority::DEFAULT;
		m_E131DataPacket.FrameLayer.Options = 0;
		// Data Layer
		m_E131DataPacket.DMPLayer.Vector = e131::vector::dmp::SET_PROPERTY;
		m_E131DataPacket.DMPLayer.Type = 0xa1;
		m_E131DataPacket.DMPLayer.FirstAddressProperty = __builtin_bswap16(0x0000);
		m_E131DataPacket.DMPLayer.AddressIncrement = __builtin_bswap16(0x0001);
		m_E131DataPacket.DMPLayer.PropertyValues[0] = 0;

		DEBUG_EXIT
	}

	void SetSynchronizationAddress([[maybe_unused]] const uint16_t SynchronizationAddress) {
	}

	void Start() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void Stop() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void Record() {
		DEBUG_ENTRY

		DEBUG_EXIT
	}

	void DmxOut(const uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength) {
		nLength++; // Add 1 for SC
		// Root Layer (See Section 5)
		m_E131DataPacket.RootLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(nLength))));
		// E1.31 Framing Layer (See Section 6)
		m_E131DataPacket.FrameLayer.FLagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(nLength))));
		m_E131DataPacket.FrameLayer.SequenceNumber = m_nSequenceNumber++;
		m_E131DataPacket.FrameLayer.Universe = __builtin_bswap16(nUniverse);
		// Data Layer
		m_E131DataPacket.DMPLayer.FlagsLength = __builtin_bswap16(static_cast<uint16_t>((0x07 << 12) | (DATA_LAYER_LENGTH(nLength))));
		memcpy(m_E131DataPacket.DMPLayer.PropertyValues, &pDmxData[1], nLength - 1);
		m_E131DataPacket.DMPLayer.PropertyValueCount = __builtin_bswap16(static_cast<uint16_t>(nLength));

		E131Bridge::Get()->HandleShowFile(&m_E131DataPacket);
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
	TE131DataPacket m_E131DataPacket;
	uint8_t m_nSequenceNumber { 0 };
};

#endif /* PROTOCOLS_SHOWFILEPROTOCOLNODEE131_H_ */
