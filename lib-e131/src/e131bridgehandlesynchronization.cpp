/**
 * @file e131bridgehandlesynchronization.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "e131bridge.h"

#include "lightsetdata.h"
#include "ledblink.h"

#include "debug.h"

void E131Bridge::HandleSynchronization() {
	// 6.3.3.1 Synchronization Address Usage in an E1.31 Synchronization Packet
	// Receivers may ignore Synchronization Packets sent to multicast addresses
	// which do not correspond to their Synchronization Address.
	//
	// NOTE: There is no multicast addresses (To Ip) available
	// We just check if SynchronizationAddress is published by a Source

	const auto nSynchronizationAddress = __builtin_bswap16(m_E131.E131Packet.Synchronization.FrameLayer.UniverseNumber);

	if ((nSynchronizationAddress != m_State.nSynchronizationAddressSourceA) && (nSynchronizationAddress != m_State.nSynchronizationAddressSourceB)) {
		LedBlink::Get()->SetMode(ledblink::Mode::NORMAL);
		DEBUG_PUTS("");
		return;
	}

	m_State.SynchronizationTime = m_nCurrentPacketMillis;

	for (uint32_t i = 0; i < E131::PORTS; i++) {
		if (m_OutputPort[i].genericPort.bIsEnabled) {
//			m_pLightSet->SetData(i, m_OutputPort[i].data, m_OutputPort[i].nLength);
			lightset::Data::Output(m_pLightSet, i);

			if (!m_OutputPort[i].IsTransmitting) {
				m_pLightSet->Start(i);
				m_OutputPort[i].IsTransmitting = true;
			}

//			m_OutputPort[i].nLength = 0;
			lightset::Data::ClearLength(i);

		}
	}

	if (m_pE131Sync != nullptr) {
		m_pE131Sync->Handler();
	}
}
