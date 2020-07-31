/**
 * @file artnettimecode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <string.h>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"
#include "artnettimecode.h"

#include "network.h"

void ArtNetNode::SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
	assert(pArtNetTimeCode != nullptr);

	if (pArtNetTimeCode != nullptr) {
		m_pArtNetTimeCode = pArtNetTimeCode;

		m_pTimeCodeData = new TArtTimeCode;
		assert(m_pTimeCodeData != nullptr);

		memset(m_pTimeCodeData, 0, sizeof (struct TArtTimeCode));
		memcpy(m_pTimeCodeData->Id, artnet::NODE_ID, sizeof m_pTimeCodeData->Id );
		m_pTimeCodeData->OpCode = OP_TIMECODE;
		m_pTimeCodeData->ProtVerLo = ArtNet::PROTOCOL_REVISION;
	}
}

void ArtNetNode::HandleTimeCode() {
	const struct TArtTimeCode *pArtTimeCode = &(m_ArtNetPacket.ArtPacket.ArtTimeCode);

	m_pArtNetTimeCode->Handler(reinterpret_cast<const struct TArtNetTimeCode*>(&pArtTimeCode->Frames));
}

void ArtNetNode::SendTimeCode(const struct TArtNetTimeCode *pArtNetTimeCode) {
	assert(pArtNetTimeCode != nullptr);

	if (pArtNetTimeCode->Frames > 29 || pArtNetTimeCode->Hours > 59 || pArtNetTimeCode->Minutes > 59 || pArtNetTimeCode->Seconds > 59 || pArtNetTimeCode->Type > 3 ) {
		return;
	}

	if (m_pTimeCodeData == nullptr) {
		m_pTimeCodeData = new TArtTimeCode;
		assert(m_pTimeCodeData != nullptr);

		memset(m_pTimeCodeData, 0, sizeof (struct TArtTimeCode));
		memcpy(m_pTimeCodeData->Id, artnet::NODE_ID, sizeof m_pTimeCodeData->Id );
		m_pTimeCodeData->OpCode = OP_TIMECODE;
		m_pTimeCodeData->ProtVerLo = ArtNet::PROTOCOL_REVISION;
	}

	memcpy(&m_pTimeCodeData->Frames, pArtNetTimeCode, sizeof(struct TArtNetTimeCode));

	Network::Get()->SendTo(m_nHandle, m_pTimeCodeData, sizeof(struct TArtTimeCode), m_Node.IPAddressBroadcast, ArtNet::UDP_PORT);
}

