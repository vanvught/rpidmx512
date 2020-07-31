/**
 * @file e131bridgehandledmxin.cpp
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

#include <stdint.h>
#include <string.h>
#include <cassert>

#include "e131bridge.h"
#include "e131packets.h"

#include "e117const.h"

#include "network.h"
#include "hardware.h"

#include "debug.h"

void E131Bridge::FillDataPacket() {
	// Root Layer (See Section 5)
	m_pE131DataPacket->RootLayer.PreAmbleSize = __builtin_bswap16(0x0010);
	m_pE131DataPacket->RootLayer.PostAmbleSize = __builtin_bswap16(0x0000);
	memcpy(m_pE131DataPacket->RootLayer.ACNPacketIdentifier, E117Const::ACN_PACKET_IDENTIFIER, E117_PACKET_IDENTIFIER_LENGTH);
	m_pE131DataPacket->RootLayer.Vector = __builtin_bswap32(E131_VECTOR_ROOT_DATA);
	memcpy(m_pE131DataPacket->RootLayer.Cid, m_Cid, E131_CID_LENGTH);
	// E1.31 Framing Layer (See Section 6)
	m_pE131DataPacket->FrameLayer.Vector = __builtin_bswap32(E131_VECTOR_DATA_PACKET);
	memcpy(m_pE131DataPacket->FrameLayer.SourceName, m_SourceName, E131_SOURCE_NAME_LENGTH);
	m_pE131DataPacket->FrameLayer.SynchronizationAddress = __builtin_bswap16(0); // Currently not supported
	m_pE131DataPacket->FrameLayer.Options = 0;
	// Data Layer
	m_pE131DataPacket->DMPLayer.Vector = E131_VECTOR_DMP_SET_PROPERTY;
	m_pE131DataPacket->DMPLayer.Type = 0xa1;
	m_pE131DataPacket->DMPLayer.FirstAddressProperty = __builtin_bswap16(0x0000);
	m_pE131DataPacket->DMPLayer.AddressIncrement = __builtin_bswap16(0x0001);
}

void E131Bridge::HandleDmxIn() {
	assert(m_pE131DataPacket != nullptr);

	for (uint32_t i = 0 ; i < E131_MAX_UARTS; i++) {
		uint32_t nUpdatesPerSecond;

		if (m_InputPort[i].bIsEnabled) {
			uint16_t nLength;
			const uint8_t *pDmxData = m_pE131DmxIn->Handler(i, nLength, nUpdatesPerSecond);

			if (pDmxData != nullptr) {
				// Root Layer (See Section 5)
				m_pE131DataPacket->RootLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (DATA_ROOT_LAYER_LENGTH(nLength)));
				// E1.31 Framing Layer (See Section 6)
				m_pE131DataPacket->FrameLayer.FLagsLength = __builtin_bswap16((0x07 << 12) | (DATA_FRAME_LAYER_LENGTH(nLength)));
				m_pE131DataPacket->FrameLayer.Priority = m_InputPort[i].nPriority;
				m_pE131DataPacket->FrameLayer.SequenceNumber = m_InputPort[i].nSequenceNumber++;
				m_pE131DataPacket->FrameLayer.Universe = __builtin_bswap16(m_InputPort[i].nUniverse);
				// Data Layer
				m_pE131DataPacket->DMPLayer.FlagsLength = __builtin_bswap16((0x07 << 12) | (DATA_LAYER_LENGTH(nLength)));
				memcpy(m_pE131DataPacket->DMPLayer.PropertyValues, pDmxData, nLength);
				m_pE131DataPacket->DMPLayer.PropertyValueCount = __builtin_bswap16(nLength);

				Network::Get()->SendTo(m_nHandle, m_pE131DataPacket, DATA_PACKET_SIZE(nLength), m_InputPort[i].nMulticastIp, E131_DEFAULT_PORT);

				m_State.bIsReceivingDmx = true;
			} else {
				if (nUpdatesPerSecond == 0) {
					m_State.bIsReceivingDmx = false;
				}
			}
		}
	}
}

