/**
 * @file rdmddiscovery.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdiscovery.h"

#include "hardware.h"

typedef union cast {
	uint64_t uint;
	uint8_t uid[RDM_UID_SIZE];
} _cast;

static _cast uuid_cast;

static constexpr auto RECEIVE_TIME_OUT = 2800U;

RDMDiscovery::RDMDiscovery(const uint8_t *pUid) {
	memcpy(m_Uid, pUid, RDM_UID_SIZE);
	m_Message.SetSrcUid(pUid);

#ifndef NDEBUG
	printf("Uid : ");
	PrintUid(m_Uid);
	printf("\n");
#endif
}

void RDMDiscovery::Full(uint32_t nPortIndex, RDMTod *pRDMTod){
	DEBUG_ENTRY

	m_nPortIndex = nPortIndex;

	m_pRDMTod = pRDMTod;
	m_pRDMTod->Reset();

	Hardware::Get()->WatchdogFeed();

	m_Message.SetPortID(static_cast<uint8_t>(1 + nPortIndex));
	m_Message.SetDstUid(UID_ALL);
	m_Message.SetCc(E120_DISCOVERY_COMMAND);
	m_Message.SetPid(E120_DISC_UN_MUTE);
	m_Message.SetPd(nullptr, 0);
	m_Message.Send(m_nPortIndex);
	m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT);

	Hardware::Get()->WatchdogFeed();
	udelay(RECEIVE_TIME_OUT);
	Hardware::Get()->WatchdogFeed();

	m_Message.Send(m_nPortIndex);
	m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT);

	Hardware::Get()->WatchdogFeed();
	udelay(RECEIVE_TIME_OUT);
	Hardware::Get()->WatchdogFeed();

	FindDevices(0x000000000000, 0xfffffffffffe);

	m_pRDMTod->Dump();
	DEBUG_EXIT
}

bool RDMDiscovery::FindDevices(uint64_t LowerBound, uint64_t UpperBound) {
	Hardware::Get()->WatchdogFeed();

	auto *pLateResponse __attribute__((unused)) = m_Message.Receive(m_nPortIndex);
#ifndef	NDEBUG
	if (pLateResponse != nullptr) {
		printf("%d ", __LINE__);
		RDMMessage::Print(pLateResponse);
	}
#endif

#ifndef NDEBUG
	printf("FindDevices : ");
	PrintUid(LowerBound);
	printf(" - ");
	PrintUid(UpperBound);
	printf("\n");
#endif

	uint8_t uid[RDM_UID_SIZE];
	struct TRdmMessage *pResponse;

	if (LowerBound == UpperBound) {
		memcpy(uid, ConvertUid(LowerBound), RDM_UID_SIZE);

		m_Message.SetCc(E120_DISCOVERY_COMMAND);
		m_Message.SetPid(E120_DISC_MUTE);
		m_Message.SetDstUid(uid);
		m_Message.SetPd(nullptr, 0);

		auto nRetry = 0;

		do {
			if (++nRetry == 10) {
				break;
			}

			Hardware::Get()->WatchdogFeed();

			m_Message.Send(m_nPortIndex, 5800);
			pResponse = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT)));

			if (pResponse != nullptr) {
				if ((pResponse->command_class != E120_DISCOVERY_COMMAND_RESPONSE) || ((static_cast<uint16_t>((pResponse->param_id[0] << 8) + pResponse->param_id[1])) != E120_DISC_MUTE)) {
					continue;
				}
			}
		} while (pResponse == nullptr);

		if (pResponse != nullptr) {
#ifndef NDEBUG
			RDMMessage::Print(reinterpret_cast<const uint8_t*>(pResponse));
#endif
			if ((pResponse->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(uid, pResponse->source_uid, RDM_UID_SIZE) == 0)) {
				m_pRDMTod->AddUid(uid);
			} else {
	#ifndef NDEBUG
				printf("%d ", __LINE__);
				PrintUid(uid);
				printf("\n");
	#endif
			}
		} else {
			return true;
		}
	} else {
		memcpy(m_Pdl[0], ConvertUid(LowerBound), RDM_UID_SIZE);
		memcpy(m_Pdl[1], ConvertUid(UpperBound), RDM_UID_SIZE);

		m_Message.SetDstUid(UID_ALL);
		m_Message.SetCc(E120_DISCOVERY_COMMAND);
		m_Message.SetPid(E120_DISC_UNIQUE_BRANCH);
		m_Message.SetPd(reinterpret_cast<const uint8_t*>(m_Pdl), 2 * RDM_UID_SIZE);

		auto nRetry = 0;

		do {
			if (++nRetry == 3) {
				break;
			}

			Hardware::Get()->WatchdogFeed();

			m_Message.Send(m_nPortIndex, 5800);
			pResponse = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT)));

		} while (pResponse == nullptr);

		if (pResponse != nullptr) {
			auto bDeviceFound = true;

			if (IsValidDiscoveryResponse(reinterpret_cast<const uint8_t*>(pResponse), uid)) {
				bDeviceFound = QuickFind(uid);
			}

			if (bDeviceFound) {
				const uint64_t MidPosition = ((LowerBound & (0x0000800000000000 - 1)) + (UpperBound & (0x0000800000000000 - 1))) / 2
						+ (UpperBound & (0x0000800000000000) ? 0x0000400000000000 : 0 )
						+ (LowerBound & (0x0000800000000000) ? 0x0000400000000000 : 0 );

				bDeviceFound = FindDevices(LowerBound, MidPosition);
				bDeviceFound |= FindDevices(MidPosition + 1, UpperBound);

				if (bDeviceFound) {
					return true;
				}
			}
		}
	}

	return false;
}

const uint8_t *RDMDiscovery::ConvertUid(uint64_t nUid) {
	uuid_cast.uint = __builtin_bswap64(nUid << 16);
	return uuid_cast.uid;
}

uint64_t RDMDiscovery::ConvertUid(const uint8_t *pUid) {
	memcpy(uuid_cast.uid, pUid, RDM_UID_SIZE);
	return __builtin_bswap64(uuid_cast.uint << 16);
}

void RDMDiscovery::PrintUid(__attribute__((unused)) uint64_t nUid) {
#ifndef NDEBUG
	PrintUid(ConvertUid(nUid));
#endif
}

void RDMDiscovery::PrintUid(__attribute__((unused)) const uint8_t *pUid) {
#ifndef NDEBUG
	printf("%.2x%.2x:%.2x%.2x%.2x%.2x", pUid[0], pUid[1], pUid[2], pUid[3], pUid[4], pUid[5]);
#endif
}

bool RDMDiscovery::IsValidDiscoveryResponse(const uint8_t *pDiscResponse, uint8_t *pUid) {
	uint8_t checksum[2];
	uint16_t nRdmChecksum = 6 * 0xFF;
	auto bIsValid = false;

	if (pDiscResponse[0] == 0xFE) {
		pUid[0] = pDiscResponse[8] & pDiscResponse[9];
		pUid[1] = pDiscResponse[10] & pDiscResponse[11];

		pUid[2] = pDiscResponse[12] & pDiscResponse[13];
		pUid[3] = pDiscResponse[14] & pDiscResponse[15];
		pUid[4] = pDiscResponse[16] & pDiscResponse[17];
		pUid[5] = pDiscResponse[18] & pDiscResponse[19];

		checksum[0] = pDiscResponse[22] & pDiscResponse[23];
		checksum[1] = pDiscResponse[20] & pDiscResponse[21];

		for (uint32_t i = 0; i < 6; i++) {
			nRdmChecksum = static_cast<uint16_t>(nRdmChecksum + pUid[i]);
		}

		if (((nRdmChecksum >> 8) == checksum[1]) && ((nRdmChecksum & 0xFF) == checksum[0])) {
			bIsValid = true;
		}

#ifndef NDEBUG
		PrintUid(pUid);
		printf(", checksum %.2x%.2x -> %.4x {%c}\n", checksum[1], checksum[0], nRdmChecksum, bIsValid ? 'Y' : 'N');
#endif

	} else {
#ifndef NDEBUG
		printf("%d ", __LINE__);
		RDMMessage::Print(pDiscResponse);
#endif
	}

	return bIsValid;
}

bool RDMDiscovery::QuickFind(const uint8_t *uid) {
	uint8_t r_uid[RDM_UID_SIZE];

#ifndef NDEBUG
	printf("QuickFind : ");
	PrintUid(uid);
	printf("\n");
#endif

	m_Message.SetCc(E120_DISCOVERY_COMMAND);
	m_Message.SetPid(E120_DISC_MUTE);
	m_Message.SetDstUid(uid);
	m_Message.SetPd(nullptr, 0);

	TRdmMessage *pResponse;

	auto nRetry = 0;

	do {
		if (++nRetry == 10) {
			break;
		}

		Hardware::Get()->WatchdogFeed();

		m_Message.Send(m_nPortIndex, 5800);
		pResponse = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT)));

		if (pResponse != nullptr) {
			if ((pResponse->command_class != E120_DISCOVERY_COMMAND_RESPONSE) || ((static_cast<uint16_t>((pResponse->param_id[0] << 8) + pResponse->param_id[1])) != E120_DISC_MUTE)) {
				continue;
			}
		}
	} while (pResponse == nullptr);

	if (pResponse != nullptr) {
#ifndef NDEBUG
		RDMMessage::Print(reinterpret_cast<const uint8_t *>(pResponse));
#endif
		if ((pResponse->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(uid, pResponse->source_uid, RDM_UID_SIZE) == 0)) {
			m_pRDMTod->AddUid(uid);
		} else {
#ifndef NDEBUG
			printf("%d ", __LINE__);
			PrintUid(uid);
			printf("\n");
#endif
		}
	}

	m_Message.SetDstUid(UID_ALL);
	m_Message.SetCc(E120_DISCOVERY_COMMAND);
	m_Message.SetPid(E120_DISC_UNIQUE_BRANCH);
	m_Message.SetPd(reinterpret_cast<const uint8_t*>(m_Pdl), 2 * RDM_UID_SIZE);

	nRetry = 0;

	do {
		if (++nRetry == 3) {
			break;
		}

		Hardware::Get()->WatchdogFeed();

		m_Message.Send(m_nPortIndex, 5800);
		pResponse = reinterpret_cast<struct TRdmMessage*>(const_cast<uint8_t*>(m_Message.ReceiveTimeOut(m_nPortIndex, RECEIVE_TIME_OUT)));

	} while (pResponse == nullptr);


	if ((pResponse != nullptr) && (IsValidDiscoveryResponse(reinterpret_cast<uint8_t *>(pResponse), r_uid))) {
		QuickFind(r_uid);
	} else if ((pResponse != nullptr) && (!IsValidDiscoveryResponse(reinterpret_cast<uint8_t *>(pResponse), r_uid))) {
		return true;
	}

	return false;
}
