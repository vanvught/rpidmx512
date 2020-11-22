/**
 * @file artnetpolltable.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# if __GNUC__ < 9
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wuseless-cast"	//FIXME GCC 8.0.3 Raspbian GNU/Linux 10 (buster)
# endif
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include "artnetpolltable.h"

#include "hardware.h"
#include "network.h"

#include "debug.h"

union uip {
	uint32_t u32;
	uint8_t u8[4];
} static ip;

ArtNetPollTable::ArtNetPollTable() {
	m_pPollTable = new TArtNetNodeEntry[ARTNET_POLL_TABLE_SIZE_ENRIES];
	assert(m_pPollTable != nullptr);

	memset(m_pPollTable, 0, sizeof(TArtNetNodeEntry[ARTNET_POLL_TABLE_SIZE_ENRIES]));

	m_pTableUniverses = new TArtNetPollTableUniverses[ARTNET_POLL_TABLE_SIZE_UNIVERSES];
	assert(m_pTableUniverses != nullptr);

	memset(m_pTableUniverses, 0, sizeof(TArtNetPollTableUniverses[ARTNET_POLL_TABLE_SIZE_UNIVERSES]));

	for (uint32_t nIndex = 0; nIndex < ARTNET_POLL_TABLE_SIZE_UNIVERSES; nIndex++) {
		m_pTableUniverses[nIndex].pIpAddresses = new uint32_t[ARTNET_POLL_TABLE_SIZE_ENRIES];
		assert(m_pTableUniverses[nIndex].pIpAddresses != nullptr);
	}

	DEBUG_PRINTF("TArtNetNodeEntry[%d] = %ld bytes [%ld Kb]", ARTNET_POLL_TABLE_SIZE_ENRIES, (sizeof(TArtNetNodeEntry[ARTNET_POLL_TABLE_SIZE_ENRIES])), (sizeof(TArtNetNodeEntry[ARTNET_POLL_TABLE_SIZE_ENRIES])) / 1024);
	DEBUG_PRINTF("TArtNetPollTableUniverses[%d] = %ld bytes [%ld Kb]", ARTNET_POLL_TABLE_SIZE_UNIVERSES, (sizeof(TArtNetPollTableUniverses[ARTNET_POLL_TABLE_SIZE_UNIVERSES])), (sizeof(TArtNetPollTableUniverses[ARTNET_POLL_TABLE_SIZE_UNIVERSES])) / 1024);

	m_tTableClean.nTableIndex = 0;
	m_tTableClean.nUniverseIndex = 0;
	m_tTableClean.bOffLine = true;
}

ArtNetPollTable::~ArtNetPollTable() {
	for (uint32_t nIndex = 0; nIndex < ARTNET_POLL_TABLE_SIZE_UNIVERSES; nIndex++) {
		delete[] m_pTableUniverses[nIndex].pIpAddresses;
		m_pTableUniverses[nIndex].pIpAddresses = nullptr;
	}

	delete[] m_pTableUniverses;
	m_pTableUniverses = nullptr;

	delete[] m_pPollTable;
	m_pPollTable = nullptr;
}

uint16_t ArtNetPollTable::MakePortAddress(uint8_t nNetSwitch, uint8_t nSubSwitch, uint8_t nUniverse) {
	// PortAddress Bit 15 = 0
	uint16_t nPortAddress = (nNetSwitch & 0x7F) << 8;	// Net : Bits 14-8
	nPortAddress |= (nSubSwitch & 0x0F) << 4;			// Sub-Net : Bits 7-4
	nPortAddress |= nUniverse & 0x0F;					// Universe : Bits 3-0

	return nPortAddress;
}

const struct TArtNetPollTableUniverses *ArtNetPollTable::GetIpAddress(uint16_t nUniverse) {
	if (m_nTableUniversesEntries == 0) {
		return nullptr;
	}

	// FIXME Universe lookup
	for (uint32_t nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			return &m_pTableUniverses[nEntry];
		}
	}

	return nullptr;
}


void ArtNetPollTable::RemoveIpAddress(uint32_t nUniverse, uint32_t nIpAddress) {
	if (m_nTableUniversesEntries == 0) {
		return;
	}

	uint32_t nEntry = 0;

	// FIXME Universe lookup
	for (nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			break;
		}
	}

	if (nEntry == m_nTableUniversesEntries) {
		// Universe not found
		return;
	}

	TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
	assert(pTableUniverses->nCount > 0);

	uint32_t nIpAddressIndex = 0;

	// FIXME IP lookup
	for (nIpAddressIndex = 0; nIpAddressIndex < pTableUniverses->nCount; nIpAddressIndex++) {
		if (pTableUniverses->pIpAddresses[nIpAddressIndex] == nIpAddress) {
			break;
		}
	}

	uint32_t *p32 = pTableUniverses->pIpAddresses;
	int32_t i;

	for (i = static_cast<int32_t>(nIpAddressIndex); i < static_cast<int32_t>(pTableUniverses->nCount) - 1; i++) {
		p32[i] = p32[i + 1];
	}

	p32[i] = 0;

	pTableUniverses->nCount--;

	if (pTableUniverses->nCount == 0) {
		DEBUG_PRINTF("Delete Universe -> m_nTableUniversesEntries=%u, nEntry=%u", m_nTableUniversesEntries, nEntry);

		TArtNetPollTableUniverses *p = m_pTableUniverses;

		for (i = static_cast<int32_t>(nEntry); i < static_cast<int32_t>(m_nTableUniversesEntries) - 1; i++) {
			p[i].nUniverse = p[i + 1].nUniverse;
			p[i].nCount = p[i + 1].nCount;
			p[i].pIpAddresses = p[i + 1].pIpAddresses;
		}

		p[i].nUniverse = 0;
		p[i].nCount = 0;

		m_nTableUniversesEntries--;
	}
}

void ArtNetPollTable::ProcessUniverse(uint32_t nIpAddress, uint16_t nUniverse) {
	DEBUG_ENTRY

	if (ARTNET_POLL_TABLE_SIZE_UNIVERSES == m_nTableUniversesEntries) {
		DEBUG_PUTS("m_pTableUniverses is full");
		DEBUG_EXIT
		return;
	}

	// FIXME Universe lookup
	bool bFoundUniverse = false;
	uint32_t nEntry = 0;

	for (nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			bFoundUniverse = true;
			DEBUG_PRINTF("Universe found %u", nUniverse);
			break;
		}
	}

	TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];

	bool bFoundIp = false;
	uint32_t nCount = 0;

	if (bFoundUniverse) {
		// FIXME IP lookup
		for (nCount = 0; nCount < pTableUniverses->nCount; nCount++) {
			if (pTableUniverses->pIpAddresses[nCount] == nIpAddress) {
				bFoundIp = true;
				DEBUG_PUTS("IP found");
				break;
			}
		}
	} else {
		// New universe
		pTableUniverses->nUniverse = nUniverse;
		m_nTableUniversesEntries++;
		DEBUG_PRINTF("New Universe %d", static_cast<int>(nUniverse));
	}

	if (!bFoundIp) {
		if (pTableUniverses->nCount < ARTNET_POLL_TABLE_SIZE_ENRIES) {
			pTableUniverses->pIpAddresses[pTableUniverses->nCount] = nIpAddress;
			pTableUniverses->nCount++;
			DEBUG_PUTS("It is a new IP for the Universe");
		} else {
			DEBUG_PUTS("New IP does not fit");
		}
	}

	DEBUG_EXIT
}

void ArtNetPollTable::Add(const struct TArtPollReply *ptArtPollReply) {
	DEBUG_ENTRY

	bool bFound = false;

	memcpy(ip.u8, ptArtPollReply->IPAddress, 4);

	const auto nIpSwap = __builtin_bswap32(ip.u32);

	int32_t i;
	int32_t nLow = 0;
	int32_t nMid;
	auto nHigh = static_cast<int32_t>(m_nPollTableEntries);

	while (nLow <= nHigh) {
		nMid = nLow + ((nHigh - nLow) / 2);
		const auto nMidValue = __builtin_bswap32(m_pPollTable[nMid].IPAddress);

		if (nMidValue < nIpSwap) {
			nLow = nMid + 1;
		} else if (nMidValue > nIpSwap) {
			nHigh = nMid - 1;
		} else {
			i = nMid;
			bFound = true;
			break;;
		}
	}

	if (!bFound) {
		if (m_nPollTableEntries == ARTNET_POLL_TABLE_SIZE_ENRIES) {
			DEBUG_PUTS("Full");
			return;
		}

		if (m_nPollTableEntries != static_cast<uint32_t>(nHigh)) {
			DEBUG_PUTS("Move");

			struct TArtNetNodeEntry *pArtNetNodeEntry = m_pPollTable; // TODO

			assert(m_nPollTableEntries >= 1);
			assert(nLow >= 0);

			for (int32_t i = static_cast<int32_t>(m_nPollTableEntries) - 1;i >= nLow; i--) {
				const struct TArtNetNodeEntry *pSrc = &pArtNetNodeEntry[i];
				struct TArtNetNodeEntry *pDst = &pArtNetNodeEntry[i + 1];
				memcpy(pDst, pSrc, sizeof(struct TArtNetNodeEntry));
			}

			struct TArtNetNodeEntry *pDst = &pArtNetNodeEntry[nLow];
			memset(pDst, 0, sizeof(struct TArtNetNodeEntry));

			i = nLow;
		} else {
			i = static_cast<int32_t>(m_nPollTableEntries);
			DEBUG_PRINTF("Add -> i=%d", i);
		}

		m_pPollTable[i].IPAddress = ip.u32;
		m_nPollTableEntries++;
	}

#ifndef NDEBUG
	if (ptArtPollReply->BindIndex <= 1) {
		memcpy(m_pPollTable[i].Mac, ptArtPollReply->MAC, ArtNet::MAC_SIZE);

		const uint8_t *pSrc = ptArtPollReply->ShortName;
		uint8_t *pDst = m_pPollTable[i].ShortName;
		memcpy(pDst, pSrc, ArtNet::SHORT_NAME_LENGTH + ArtNet::LONG_NAME_LENGTH);
	}
#endif

	const uint32_t nMillis = Hardware::Get()->Millis();

	for (uint32_t nIndex = 0; nIndex < ArtNet::MAX_PORTS; nIndex++) {
		const uint8_t nPortAddress = ptArtPollReply->SwOut[nIndex];

		if (ptArtPollReply->PortTypes[nIndex] == ARTNET_ENABLE_OUTPUT) {
			const uint16_t nUniverse = MakePortAddress(ptArtPollReply->NetSwitch, ptArtPollReply->SubSwitch, nPortAddress);

			uint32_t nIndexUniverse;

			for (nIndexUniverse = 0; nIndexUniverse < m_pPollTable[i].nUniversesCount; nIndexUniverse++) {
				if (m_pPollTable[i].Universe[nIndexUniverse].nUniverse == nUniverse) {
					break;
				}
			}

			if (nIndexUniverse == m_pPollTable[i].nUniversesCount) {
				// Not found
				if (m_pPollTable[i].nUniversesCount < ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES) {
					m_pPollTable[i].nUniversesCount++;
					m_pPollTable[i].Universe[nIndexUniverse].nUniverse = nUniverse;
					ProcessUniverse(ip.u32, nUniverse);
				} else {
					// No room
					continue;
				}
			}

			m_pPollTable[i].Universe[nIndexUniverse].nLastUpdateMillis = nMillis;
		}
	}

	DEBUG_EXIT;
}

void ArtNetPollTable::Clean() {
	if (m_nPollTableEntries == 0) {
		return;
	}

	assert(m_tTableClean.nTableIndex < m_nPollTableEntries);
	assert(m_tTableClean.nUniverseIndex < ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES);

	if (m_tTableClean.nUniverseIndex == 0) {
		m_tTableClean.bOffLine = true;
	}

	struct TArtNetNodeEntryUniverse *pArtNetNodeEntryBind = &m_pPollTable[m_tTableClean.nTableIndex].Universe[m_tTableClean.nUniverseIndex];

	if (pArtNetNodeEntryBind->nLastUpdateMillis != 0) {
		if ((Hardware::Get()->Millis() - pArtNetNodeEntryBind->nLastUpdateMillis) > (1.5 * ARTNET_POLL_INTERVAL_MILLIS)) {
			pArtNetNodeEntryBind->nLastUpdateMillis = 0;
			RemoveIpAddress(pArtNetNodeEntryBind->nUniverse, m_pPollTable[m_tTableClean.nTableIndex].IPAddress);
		} else {
			m_tTableClean.bOffLine = false;
		}
	}

	m_tTableClean.nUniverseIndex++;

	if (m_tTableClean.nUniverseIndex == ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES) {
		if (m_tTableClean.bOffLine) {
			DEBUG_PUTS("Node is off-line");

			struct TArtNetNodeEntry *pArtNetNodeEntry = m_pPollTable;
			// Move
			for (uint32_t i = m_tTableClean.nTableIndex; i < (m_nPollTableEntries - 1); i++) {
				const struct TArtNetNodeEntry *pSrc = &pArtNetNodeEntry[i + 1];
				struct TArtNetNodeEntry *pDst = &pArtNetNodeEntry[i];

				memcpy(pDst, pSrc, sizeof(struct TArtNetNodeEntry));
			}

			m_nPollTableEntries--;

			struct TArtNetNodeEntry *pDst = &pArtNetNodeEntry[m_nPollTableEntries];
			pDst->IPAddress = 0;
			pDst->nUniversesCount = 0;
			memset(pDst->Universe, 0, sizeof(struct TArtNetNodeEntryUniverse[ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES]));
#ifndef NDEBUG
			memset(pDst->Mac, 0, ArtNet::MAC_SIZE + ArtNet::SHORT_NAME_LENGTH + ArtNet::LONG_NAME_LENGTH);
#endif
		}

		m_tTableClean.nUniverseIndex = 0;
		m_tTableClean.bOffLine = true;
		m_tTableClean.nTableIndex++;

		if (m_tTableClean.nTableIndex >= m_nPollTableEntries) {
			m_tTableClean.nTableIndex = 0;
		}
	}
}

void ArtNetPollTable::Dump() {
#ifndef NDEBUG
	printf("Entries : %d\n", m_nPollTableEntries);

	for (uint32_t i = 0; i < m_nPollTableEntries; i++) {
		printf("\t" IPSTR " [" MACSTR "] |%-18s|%-64s|\n", IP2STR(m_pPollTable[i].IPAddress), MAC2STR(m_pPollTable[i].Mac), m_pPollTable[i].ShortName, m_pPollTable[i].LongName);

		for (uint32_t nUniverse = 0; nUniverse < m_pPollTable[i].nUniversesCount; nUniverse++) {
			struct TArtNetNodeEntryUniverse *pArtNetNodeEntryUniverse = &m_pPollTable[i].Universe[nUniverse];
			printf("\t %u [%u]\n", pArtNetNodeEntryUniverse->nUniverse, (Hardware::Get()->Millis() - pArtNetNodeEntryUniverse->nLastUpdateMillis) / 1000);
		}
		puts("");
	}
#endif
}

void ArtNetPollTable::DumpTableUniverses() {
#ifndef NDEBUG
	printf("Entries : %d\n", m_nTableUniversesEntries);

	for (uint32_t nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		const TArtNetPollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		printf("%3d |%4u | %d ", nEntry, pTableUniverses->nUniverse, pTableUniverses->nCount);

		const uint32_t *pIpAddresses = pTableUniverses->pIpAddresses;
		assert(pIpAddresses != nullptr);

		for (uint32_t nCount = 0; nCount < pTableUniverses->nCount; nCount++) {
			printf(" " IPSTR, IP2STR(pIpAddresses[nCount]));
		}

		puts("");
	}

	puts("");
#endif
}
