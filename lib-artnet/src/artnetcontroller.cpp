/**
 * @file artnetcontroller.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "artnetcontroller.h"
#include "artnet.h"
#include "artnetconst.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

#define ARTNET_MIN_HEADER_SIZE		12

static uint16_t s_ActiveUniverses[512] __attribute__ ((aligned (4)));

ArtNetController *ArtNetController::s_pThis = 0;

ArtNetController::ArtNetController(void):
	m_bSynchronization(true),
	m_bUnicast(true),
	m_nHandle(-1),
	m_pArtNetTrigger(0),
	m_nLastPollMillis(0),
	m_bDoTableCleanup(true),
	m_bDmxHandled(false),
	m_nActiveUniverses(0)
{
	DEBUG_ENTRY

	s_pThis = this;

	m_pArtNetPacket = new struct TArtNetPacket;
	assert(m_pArtNetPacket != 0);

	memset((void *) &m_ArtNetPoll, 0, sizeof(struct TArtPoll));
	memcpy((void *) &m_ArtNetPoll, (const char *) NODE_ID, 8);
	m_ArtNetPoll.OpCode = OP_POLL;
	m_ArtNetPoll.ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;
	m_ArtNetPoll.TalkToMe = TTM_SEND_ARTP_ON_CHANGE;

	m_pArtDmx = new struct TArtDmx;
	assert(m_pArtDmx != 0);

	memset((void *) m_pArtDmx, 0, sizeof(struct TArtDmx));
	memcpy((void *) m_pArtDmx, (const char *) NODE_ID, 8);
	m_pArtDmx->OpCode = OP_DMX;
	m_pArtDmx->ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;

	m_pArtSync = new struct TArtSync;
	assert(m_pArtSync != 0);

	memset((void *) m_pArtSync, 0, sizeof(struct TArtSync));
	memcpy((void *) m_pArtSync, (const char *) NODE_ID, 8);
	m_pArtSync->OpCode = OP_SYNC;
	m_pArtSync->ProtVerLo = (uint8_t) ARTNET_PROTOCOL_REVISION;

	m_tArtNetController.Oem[0] = ArtNetConst::OEM_ID[0];
	m_tArtNetController.Oem[1] = ArtNetConst::OEM_ID[1];

	ActiveUniversesClear();

	DEBUG_EXIT
}

ArtNetController::~ArtNetController(void) {
	DEBUG_ENTRY

	delete m_pArtNetPacket;
	m_pArtNetPacket = 0;

	DEBUG_EXIT
}

void ArtNetController::Start(void) {
	DEBUG_ENTRY

	m_tArtNetController.nIPAddressLocal = Network::Get()->GetIp();
	m_tArtNetController.nIPAddressBroadcast = Network::Get()->GetBroadcastIp();

	m_nHandle = Network::Get()->Begin(ARTNET_UDP_PORT);
	assert(m_nHandle != -1);

	HandlePoll();

	DEBUG_EXIT
}

void ArtNetController::Stop(void) {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void ArtNetController::HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint16_t nLength, uint8_t nPortIndex) {
	DEBUG_ENTRY

	ActiveUniversesAdd(nUniverse);

	m_pArtDmx->Physical = nPortIndex;

	m_pArtDmx->PortAddress = nUniverse;
	m_pArtDmx->LengthHi = (nLength & 0xFF00) >> 8;
	m_pArtDmx->Length = (nLength & 0xFF);

	// The sequence number is used to ensure that ArtDmx packets are used in the correct order.
	// This field is incremented in the range 0x01 to 0xff to allow the receiving node to resequence packets.
	m_pArtDmx->Sequence++;
	if (m_pArtDmx->Sequence == 0) {
		m_pArtDmx->Sequence = 1;
	}

	memcpy(m_pArtDmx->Data, pDmxData, nLength);

	struct TArtNetPollTableUniverses *IpAddresses;
	uint32_t nCount;

	if (m_bUnicast) {
		IpAddresses = (struct TArtNetPollTableUniverses*) GetIpAddress(nUniverse);
		if (IpAddresses != 0) {
			nCount = IpAddresses->nCount;
		} else {
			nCount = 0;
		}
	}

	// If the number of universe subscribers exceeds 40 for a given universe, the transmitting device may broadcast.
	if (m_bUnicast && (nCount != 0) && (nCount <= 40)) {
		for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
			Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pArtDmx, (uint16_t) sizeof(struct TArtDmx), IpAddresses->pIpAddresses[nIndex], ARTNET_UDP_PORT);
		}

		m_bDmxHandled = true;

		DEBUG_EXIT
		return;
	}

	if (!m_bUnicast) {
		Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pArtDmx, (uint16_t) sizeof(struct TArtDmx), m_tArtNetController.nIPAddressBroadcast, ARTNET_UDP_PORT);

		m_bDmxHandled = true;
	}

	DEBUG_EXIT
}

void ArtNetController::HandleSync(void) {
	if (m_bSynchronization && m_bDmxHandled) {
		m_bDmxHandled = false;
		Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pArtSync, (uint16_t) sizeof(struct TArtSync), m_tArtNetController.nIPAddressBroadcast, ARTNET_UDP_PORT);
	}
}

void ArtNetController::HandleBlackout(void) {
	m_pArtDmx->LengthHi = (512 & 0xFF00) >> 8;
	m_pArtDmx->Length = (512 & 0xFF);

	memset(m_pArtDmx->Data, 0, 512);

	for (uint32_t nIndex = 0; nIndex < m_nActiveUniverses; nIndex++) {
		m_pArtDmx->PortAddress = s_ActiveUniverses[nIndex];

		if (m_bUnicast) {
			const struct TArtNetPollTableUniverses *IpAddresses = GetIpAddress(s_ActiveUniverses[nIndex]);
			if (IpAddresses != 0) {
				for (uint32_t nIndex = 0; nIndex < IpAddresses->nCount; nIndex++) {
					m_pArtDmx->Sequence++;
					Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pArtDmx, (uint16_t) sizeof(struct TArtDmx), IpAddresses->pIpAddresses[nIndex], ARTNET_UDP_PORT);
				}
			}
		} else  {
			m_pArtDmx->Sequence++;
			Network::Get()->SendTo(m_nHandle, (const uint8_t *) m_pArtDmx, (uint16_t) sizeof(struct TArtDmx), m_tArtNetController.nIPAddressBroadcast, ARTNET_UDP_PORT);
		}

		DEBUG_PRINTF("s_ActiveUniverses[%d]=%u", nIndex, s_ActiveUniverses[nIndex]);
	}

	m_bDmxHandled = true;
	HandleSync();
}

void ArtNetController::HandleTrigger(void) {
	DEBUG_ENTRY
	const TArtTrigger *pArtTrigger = (TArtTrigger *) &m_pArtNetPacket->ArtPacket.ArtTrigger;

	if ((pArtTrigger->OemCodeHi == 0xFF && pArtTrigger->OemCodeLo == 0xFF) || (pArtTrigger->OemCodeHi == m_tArtNetController.Oem[0] && pArtTrigger->OemCodeLo == m_tArtNetController.Oem[1])) {
		DEBUG_PRINTF("Key=%d, SubKey=%d, Data[0]=%d", pArtTrigger->Key, pArtTrigger->SubKey, pArtTrigger->Data[0]);

		m_pArtNetTrigger->Handler((const struct TArtNetTrigger *)&pArtTrigger->Key);
	}

	DEBUG_EXIT
}

void ArtNetController::HandlePoll(void) {
	const uint32_t nCurrentMillis = Hardware::Get()->Millis();

	if (__builtin_expect((nCurrentMillis - m_nLastPollMillis > ARTNET_POLL_INTERVAL_MILLIS), 0)) {
#ifndef NDEBUG
		time_t ltime = Hardware::Get()->GetTime();
		struct tm tm = *localtime(&ltime);

		DEBUG_PRINTF("SendPoll - %.2d:%.2d:%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif

		DEBUG_PUTS("");

		Network::Get()->SendTo(m_nHandle, (const uint8_t *)&m_ArtNetPoll, sizeof(struct TArtPoll), m_tArtNetController.nIPAddressBroadcast, ARTNET_UDP_PORT);
		m_nLastPollMillis= nCurrentMillis;

		DEBUG_PUTS("");

#ifndef NDEBUG
		Dump();
		DumpTableUniverses();
#endif
	}

	if (m_bDoTableCleanup && (__builtin_expect((nCurrentMillis - m_nLastPollMillis > ARTNET_POLL_INTERVAL_MILLIS/4), 0))) {
		Clean();
	}
}

void ArtNetController::HandlePollReply(void) {
	DEBUG_ENTRY

#ifndef NDEBUG
	time_t ltime = Hardware::Get()->GetTime();;
	struct tm tm = *localtime(&ltime);

	printf("ArtPollReply - %.2d:%.2d:%.2d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif

	const TArtPollReply *pArtPollReply = (TArtPollReply *)(&m_pArtNetPacket->ArtPacket.ArtIpProgReply);

	Add(pArtPollReply);

	DEBUG_EXIT
}

void ArtNetController::Run(void) {
	const char *pArtPacket = (char *)(&m_pArtNetPacket->ArtPacket);
	uint16_t nForeignPort;

	if (m_bUnicast) {
		HandlePoll();
	}

	const int nBytesReceived = Network::Get()->RecvFrom(m_nHandle, (uint8_t *)pArtPacket, (const uint16_t)sizeof(struct TArtNetPacket), &m_pArtNetPacket->IPAddressFrom, &nForeignPort) ;

	if (__builtin_expect((nBytesReceived < ARTNET_MIN_HEADER_SIZE), 1)) {
		return;
	}

	if (memcmp(pArtPacket, "Art-Net\0", 8) != 0) {
		return;
	}

	const TOpCodes OpCode = (TOpCodes) ((uint16_t)(pArtPacket[9] << 8) + pArtPacket[8]);

	switch (OpCode) {
	case OP_POLLREPLY:
		HandlePollReply();
		break;
	case OP_TRIGGER:
		if (m_pArtNetTrigger != 0) {
			HandleTrigger();
		}
		break;
	default:
		break;
	}
}

void ArtNetController::ActiveUniversesClear(void) {
	memset(s_ActiveUniverses, 0, sizeof(s_ActiveUniverses));
	m_nActiveUniverses = 0;
}

void ArtNetController::ActiveUniversesAdd(uint16_t nUniverse) {
	DEBUG_ENTRY
	DEBUG_PRINTF("nUniverse=%d", (int) nUniverse);

	int32_t nLow = 0;
	int32_t nMid = 0;
	int32_t nHigh = m_nActiveUniverses;

	if (m_nActiveUniverses == (sizeof(s_ActiveUniverses) / sizeof(s_ActiveUniverses[0]))) {
		assert(0);
		return;
	}

	while (nLow <= nHigh) {

		nMid = nLow + ((nHigh - nLow) / 2);
		const uint32_t nMidValue = (const uint32_t) s_ActiveUniverses[nMid];

		if (nMidValue < nUniverse) {
			nLow = nMid + 1;
		} else if (nMidValue > nUniverse) {
			nHigh = nMid - 1;
		} else {
			DEBUG_EXIT
			return;
		}
	}

	DEBUG_PRINTF("nLow=%d, nMid=%d, nHigh=%d", (int) nLow, (int) nMid, (int) nHigh);

	if ((nHigh != -1) && (m_nActiveUniverses != (uint32_t) nHigh)) {

		uint16_t *p16 = (uint16_t *)s_ActiveUniverses;

		assert(nLow >= 0);

		for (uint32_t i = m_nActiveUniverses; i >= (uint32_t) nLow; i--) {
			p16[i + 1] = p16[i];
		}

		s_ActiveUniverses[nLow] = nUniverse;

		DEBUG_PRINTF(">m< nUniverse=%u, nLow=%d", nUniverse, nLow);
	} else {
		s_ActiveUniverses[m_nActiveUniverses] = nUniverse;

		DEBUG_PRINTF(">a< nUniverse=%u, nMid=%d", nUniverse, nMid);
	}

	m_nActiveUniverses++;

	DEBUG_EXIT
}

void ArtNetController::Print(void) {
	printf("Art-Net Controller\n");
	printf(" Max Node's    : %u\n", ARTNET_POLL_TABLE_SIZE_ENRIES);
	printf(" Max Universes : %u\n", ARTNET_POLL_TABLE_SIZE_UNIVERSES);
	if (!m_bSynchronization) {
		puts(" Synchronization is disabled");
	}
}
