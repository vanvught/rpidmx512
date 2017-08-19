/*
 * artnetcontroller.cpp
 *
 *  Created on: Aug 11, 2017
 *      Author: pi
 */

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#if defined (__linux__) || defined (__CYGWIN__)
#include <string.h>
#else
#include "util.h"
#endif

#include "artnetcontroller.h"
#include "artnetpolltable.h"

#include "network.h"

#define ARTNET_UDP_PORT				0x1936
#define ARTNET_MIN_HEADER_SIZE		12
#define ARTNET_PROTOCOL_REVISION	14							///< Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
#define ARTNET_ID					"Art-Net"

ArtNetController::ArtNetController(void) : m_nLastPollTime(0), m_nPollInterVal(8) {
	m_pArtNetPacket = new (struct TArtNetPacket);

	memset((void *) &m_ArtNetPoll, 0, sizeof(struct TArtPoll));
	memcpy((void *) &m_ArtNetPoll, (const char *) ARTNET_ID, 8);
	m_ArtNetPoll.OpCode = OP_POLL;
	m_ArtNetPoll.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;
	m_ArtNetPoll.TalkToMe = (1 << 1);///< Bit 1 set : Send ArtPollReply whenever Node conditions change.

	memset((void *) &m_ArtIpProg, 0, sizeof(struct TArtIpProg));
	memcpy((void *) &m_ArtIpProg, (const char *) ARTNET_ID, 8);
	m_ArtIpProg.OpCode = OP_IPPROG;
	m_ArtIpProg.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;

	m_IPAddressLocal = network_get_ip();
	m_IPAddressBroadcast = m_IPAddressLocal | ~network_get_netmask();
}

ArtNetController::~ArtNetController(void) {
	delete m_pArtNetPacket;
}

void ArtNetController::Start(void) {
	network_begin(ARTNET_UDP_PORT);
}

void ArtNetController::Stop(void) {
}

void ArtNetController::SetPollInterval(const uint8_t nPollInterval) {
	if (nPollInterval < 8) {
		return;
	}

	m_nPollInterVal = nPollInterval;
}

const uint8_t ArtNetController::GetPollInterval(void) {
	return m_nPollInterVal;
}

void ArtNetController::SendPoll(void) {
	const time_t nTime = time(NULL);

	if (nTime - m_nLastPollTime >= 8) {
		network_sendto((const uint8_t *)&m_ArtNetPoll, sizeof(struct TArtPoll), m_IPAddressBroadcast, ARTNET_UDP_PORT);
		m_nLastPollTime= nTime;
	}
}

void ArtNetController::HandlePollReply(void) {
	time_t ltime = time(NULL);
	struct tm tm = *localtime(&ltime);

	printf("%.2d-%.2d-%.4d %.2d:%.2d:%.2d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (!Add(&m_pArtNetPacket->ArtPacket.ArtPollReply)) {
		SendIpProg();
	}
}

void ArtNetController::SendIpProg(void) {
	network_sendto((const uint8_t *)&m_ArtIpProg, sizeof(struct TArtIpProg), m_pArtNetPacket->IPAddressFrom, ARTNET_UDP_PORT);
}

void ArtNetController::HandleIpProgReply(void) {
	Add(&m_pArtNetPacket->ArtPacket.ArtIpProgReply);
}

void ArtNetController::SendIpProg(const uint32_t nIp, const struct TArtNetIpProg *pArtNetIpProg) {
	struct TArtIpProg ArtIpProg;

	memset((void *) &ArtIpProg, 0, sizeof(struct TArtIpProg));
	memcpy((void *) &ArtIpProg, (const char *) ARTNET_ID, 8);
	ArtIpProg.OpCode = OP_IPPROG;
	ArtIpProg.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;

	memcpy(&ArtIpProg.Command, pArtNetIpProg, sizeof (struct TArtNetIpProg));

	network_sendto((const uint8_t *)&ArtIpProg, sizeof(struct TArtIpProg), nIp, ARTNET_UDP_PORT);
}

int ArtNetController::Run(void) {
	const char *packet = (char *)(&m_pArtNetPacket->ArtPacket);
	uint16_t nForeignPort;
	TOpCodes OpCode;

	SendPoll();

	const int nBytesReceived = network_recvfrom((const uint8_t *)packet, (const uint16_t)sizeof(struct TArtNetPacket), &m_pArtNetPacket->IPAddressFrom, &nForeignPort) ;

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


