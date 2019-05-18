/**
 * @file artnettimesync.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>

#include "artnettimesync.h"

#include "artnetnode.h"
#include "network.h"

ArtNetTimeSync::~ArtNetTimeSync(void) {
}

void ArtNetNode::SetTimeSyncHandler(ArtNetTimeSync *pArtNetTimeSync) {
	assert(pArtNetTimeSync != 0);

	m_pArtNetTimeSync = pArtNetTimeSync;
}

void ArtNetNode::HandleTimeSync(void) {
	struct TArtTimeSync *packet = (struct TArtTimeSync *) &(m_ArtNetPacket.ArtPacket.ArtTimeSync);

	m_pArtNetTimeSync->Handler((struct TArtNetTimeSync *)&packet->tm_sec);

	packet->Prog = 0;

	Network::Get()->SendTo(m_nHandle, (const uint8_t *) packet, (const uint16_t) sizeof(struct TArtTimeSync), m_ArtNetPacket.IPAddressFrom, (uint16_t) ARTNET_UDP_PORT);
}

