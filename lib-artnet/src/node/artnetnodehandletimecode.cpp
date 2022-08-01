/**
 * @file artnetnodetimecode.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstring>
#include <cassert>

#include "artnetnode.h"
#include "artnet.h"
#include "artnettimecode.h"

#include "network.h"

#include "debug.h"

void ArtNetNode::HandleTimeCode() {
	const auto *pArtTimeCode = &(m_ArtNetPacket.ArtPacket.ArtTimeCode);

	m_pArtNetTimeCode->Handler(reinterpret_cast<const struct TArtNetTimeCode*>(&pArtTimeCode->Frames));
}

void ArtNetNode::SetTimeCodeIp(uint32_t nDestinationIp) {
	if (Network::Get()->IsValidIp(nDestinationIp)) {
		m_Node.IPAddressTimeCode = nDestinationIp;
	} else {
		m_Node.IPAddressTimeCode = m_Node.IPAddressBroadcast;
	}

	DEBUG_PRINTF("m_Node.IPAddressTimeCode=" IPSTR, IP2STR(m_Node.IPAddressTimeCode));
}

void ArtNetNode::SendTimeCode(const struct TArtNetTimeCode *pArtNetTimeCode) {
	assert(pArtNetTimeCode != nullptr);
	assert(pArtNetTimeCode->Frames < 30);
	assert(pArtNetTimeCode->Hours < 60);
	assert(pArtNetTimeCode->Minutes < 60);
	assert(pArtNetTimeCode->Seconds < 60);
	assert(pArtNetTimeCode->Type < 4);

	auto *pArtTimeCode = &(m_ArtNetPacket.ArtPacket.ArtTimeCode);

	memcpy(pArtTimeCode->Id, artnet::NODE_ID, sizeof(pArtTimeCode->Id));
	pArtTimeCode->OpCode = OP_TIMECODE;
	pArtTimeCode->ProtVerHi = 0;
	pArtTimeCode->ProtVerLo = artnet::PROTOCOL_REVISION;
	pArtTimeCode->Filler1 = 0;
	pArtTimeCode->Filler2 = 0;

	memcpy(&pArtTimeCode->Frames, pArtNetTimeCode, sizeof(struct TArtNetTimeCode));

	Network::Get()->SendTo(m_nHandle, pArtTimeCode, sizeof(struct TArtTimeCode), m_Node.IPAddressTimeCode, artnet::UDP_PORT);
}
