/**
 * @file artnetcontroller.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <time.h>

#if defined (BARE_METAL)
 #include "util.h"
#else
 #include <string.h>
#endif

#include "artnet.h"

#include "artnetcontroller.h"
#include "artnetpolltable.h"

#include "network.h"

#define ARTNET_UDP_PORT				0x1936
#define ARTNET_MIN_HEADER_SIZE		12
#define ARTNET_PROTOCOL_REVISION	14							///< Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
#define ARTNET_ID					"Art-Net"

#define POLL_INTERVAL_MIN			8	//< Seconds

ArtNetController::ArtNetController(void) : m_nLastPollTime(0), m_IPAddressLocal(0), m_IPAddressBroadcast(0), m_nPollInterVal(POLL_INTERVAL_MIN) {
	m_pArtNetPacket = new (struct TArtNetPacket);

	memset((void *) &m_ArtNetPoll, 0, sizeof(struct TArtPoll));
	memcpy((void *) &m_ArtNetPoll, (const char *) ARTNET_ID, 8);
	m_ArtNetPoll.OpCode = OP_POLL;
	m_ArtNetPoll.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;
	m_ArtNetPoll.TalkToMe = TTM_SEND_ARTP_ON_CHANGE;

	memset((void *) &m_ArtIpProg, 0, sizeof(struct TArtIpProg));
	memcpy((void *) &m_ArtIpProg, (const char *) ARTNET_ID, 8);
	m_ArtIpProg.OpCode = OP_IPPROG;
	m_ArtIpProg.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;
}

ArtNetController::~ArtNetController(void) {
	delete m_pArtNetPacket;
}

void ArtNetController::Start(void) {
	m_IPAddressLocal = Network::Get()->GetIp();
	m_IPAddressBroadcast = m_IPAddressLocal | ~(Network::Get()->GetNetmask());

	Network::Get()->Begin(ARTNET_UDP_PORT);
}

void ArtNetController::Stop(void) {
}

void ArtNetController::SetPollInterval(const uint8_t nPollInterval) {
	if (nPollInterval < POLL_INTERVAL_MIN) {
		return;
	}

	m_nPollInterVal = nPollInterval;
}

const uint8_t ArtNetController::GetPollInterval(void) {
	return m_nPollInterVal;
}

void ArtNetController::SendPoll(void) {
	const time_t nTime = time(NULL);

	if (nTime - m_nLastPollTime >= m_nPollInterVal) {
		Network::Get()->SendTo((const uint8_t *)&m_ArtNetPoll, sizeof(struct TArtPoll), m_IPAddressBroadcast, ARTNET_UDP_PORT);
		m_nLastPollTime= nTime;
	}
}

void ArtNetController::HandlePollReply(void) {
#ifndef NDEBUG
	time_t ltime = time(NULL);
	struct tm tm = *localtime(&ltime);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif

	SendIpProg();
}

void ArtNetController::SendIpProg(void) {
	Network::Get()->SendTo((const uint8_t *)&m_ArtIpProg, sizeof(struct TArtIpProg), m_pArtNetPacket->IPAddressFrom, ARTNET_UDP_PORT);
}

void ArtNetController::HandleIpProgReply(void) {
	Add(&m_pArtNetPacket->ArtPacket.ArtIpProgReply);
}

void ArtNetController::SendIpProg(const uint32_t nRemoteIp, const struct TArtNetIpProg *pArtNetIpProg) {
	struct TArtIpProg ArtIpProg;

	memset((void *) &ArtIpProg, 0, sizeof(struct TArtIpProg));
	memcpy((void *) &ArtIpProg, (const char *) ARTNET_ID, 8);
	ArtIpProg.OpCode = OP_IPPROG;
	ArtIpProg.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;

	memcpy(&ArtIpProg.Command, pArtNetIpProg, sizeof (struct TArtNetIpProg));

	Network::Get()->SendTo((const uint8_t *)&ArtIpProg, sizeof(struct TArtIpProg), nRemoteIp, ARTNET_UDP_PORT);
}

int ArtNetController::Run(void) {
	const char *packet = (char *)(&m_pArtNetPacket->ArtPacket);
	uint16_t nForeignPort;
	TOpCodes OpCode;

	SendPoll();

	const int nBytesReceived = Network::Get()->RecvFrom((uint8_t *)packet, (const uint16_t)sizeof(struct TArtNetPacket), &m_pArtNetPacket->IPAddressFrom, &nForeignPort) ;

	if (nBytesReceived == 0) {
		return 0;
	}

	if (nBytesReceived < ARTNET_MIN_HEADER_SIZE) {
		return 0;
	}

	if (memcmp(packet, "Art-Net\0", 8) != 0) {
		return 0;
	}

	OpCode = (TOpCodes) ((uint16_t)(packet[9] << 8) + packet[8]);

	switch (OpCode) {
	case OP_POLLREPLY:
		HandlePollReply();
		break;
	case OP_IPPROGREPLY:
		HandleIpProgReply();
		break;
	default:
		return 0;
		break;
	}

	return nBytesReceived;
}
