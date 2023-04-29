/**
 * @file artnetnodehandlesync.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "artnetnode.h"
#include "artnet.h"

#include "lightsetdata.h"
#include "hardware.h"

void ArtNetNode::HandleSync() {
	m_State.IsSynchronousMode = true;
	m_State.nArtSyncMillis = Hardware::Get()->Millis();

	for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
		if ((m_OutputPort[nPortIndex].protocol == artnet::PortProtocol::ARTNET) && (m_OutputPort[nPortIndex].genericPort.bIsEnabled)) {
#if defined ( ARTNET_ENABLE_SENDDIAG )
			SendDiag("Send pending data", ARTNET_DP_LOW);
#endif
			lightset::Data::Output(m_pLightSet, nPortIndex);

			if (!m_OutputPort[nPortIndex].IsTransmitting) {
				m_pLightSet->Start(nPortIndex);
				m_OutputPort[nPortIndex].IsTransmitting = true;
			}

			lightset::Data::ClearLength(nPortIndex);
		}
	}
}
