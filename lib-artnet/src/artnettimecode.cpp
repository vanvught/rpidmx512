/**
 * @file artnettimecode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <assert.h>

#include "artnettimecode.h"

#include "artnetnode.h"
#include "network.h"

ArtNetTimeCode::~ArtNetTimeCode(void) {
}

void ArtNetNode::SetTimeCodeHandler(ArtNetTimeCode *pArtNetTimeCode) {
	assert(pArtNetTimeCode != 0);

	if (pArtNetTimeCode != 0) {
		m_pArtNetTimeCode = pArtNetTimeCode;

		m_pTimeCodeData = new TArtTimeCode;
		assert(m_pTimeCodeData != 0);

		memset(m_pTimeCodeData, 0, sizeof (struct TArtTimeCode));
		memcpy(m_pTimeCodeData->Id, (const char *) NODE_ID, sizeof m_pTimeCodeData->Id );
		m_pTimeCodeData->OpCode = OP_TIMECODE;
		m_pTimeCodeData->ProtVerLo = ARTNET_PROTOCOL_REVISION;
	}
}

void ArtNetNode::HandleTimeCode(void) {
	const struct TArtTimeCode *packet = (struct TArtTimeCode *) &(m_ArtNetPacket.ArtPacket.ArtTimeCode);

	m_pArtNetTimeCode->Handler((struct TArtNetTimeCode *) &packet->Frames);
}

void ArtNetNode::SendTimeCode(const struct TArtNetTimeCode *pArtNetTimeCode) {
	assert(pArtNetTimeCode != 0);

	if (pArtNetTimeCode->Frames > 29 || pArtNetTimeCode->Hours > 59 || pArtNetTimeCode->Minutes > 59 || pArtNetTimeCode->Seconds > 59 || pArtNetTimeCode->Type > 3 ) {
		return;
	}

	if (m_pTimeCodeData == 0) {
		m_pTimeCodeData = new TArtTimeCode;
		assert(m_pTimeCodeData != 0);

		memset(m_pTimeCodeData, 0, sizeof (struct TArtTimeCode));
		memcpy(m_pTimeCodeData->Id, (const char *) NODE_ID, sizeof m_pTimeCodeData->Id );
		m_pTimeCodeData->OpCode = OP_TIMECODE;
		m_pTimeCodeData->ProtVerLo = ARTNET_PROTOCOL_REVISION;
	}

	memcpy((void *) &m_pTimeCodeData->Frames, pArtNetTimeCode, sizeof(struct TArtNetTimeCode));

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pTimeCodeData, (uint16_t) sizeof(struct TArtTimeCode), m_Node.IPAddressBroadcast, (uint16_t) ARTNET_UDP_PORT);
}

