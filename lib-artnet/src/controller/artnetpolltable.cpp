/**
 * @file artnetpolltable.cpp
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cstring>
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
	DEBUG_ENTRY

	m_pPollTable = new artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES];
	assert(m_pPollTable != nullptr);

	memset(m_pPollTable, 0, sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES]));

	m_pTableUniverses = new artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES];
	assert(m_pTableUniverses != nullptr);

	memset(m_pTableUniverses, 0, sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES]));

	for (uint32_t nIndex = 0; nIndex < artnet::POLL_TABLE_SIZE_UNIVERSES; nIndex++) {
		m_pTableUniverses[nIndex].pIpAddresses = new uint32_t[artnet::POLL_TABLE_SIZE_ENRIES];
		assert(m_pTableUniverses[nIndex].pIpAddresses != nullptr);
	}

	m_PollTableClean.nTableIndex = 0;
	m_PollTableClean.nUniverseIndex = 0;
	m_PollTableClean.bOffLine = true;

	DEBUG_PRINTF("NodeEntry[%d] = %u bytes [%u Kb]", artnet::POLL_TABLE_SIZE_ENRIES, static_cast<unsigned>(sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES])), static_cast<unsigned>(sizeof(artnet::NodeEntry[artnet::POLL_TABLE_SIZE_ENRIES])) / 1024U);
	DEBUG_PRINTF("PollTableUniverses[%d] = %u bytes [%u Kb]", artnet::POLL_TABLE_SIZE_UNIVERSES, static_cast<unsigned>(sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES])), static_cast<unsigned>(sizeof(artnet::PollTableUniverses[artnet::POLL_TABLE_SIZE_UNIVERSES])) / 1024U);
	DEBUG_EXIT
}

ArtNetPollTable::~ArtNetPollTable() {
	for (uint32_t nIndex = 0; nIndex < artnet::POLL_TABLE_SIZE_UNIVERSES; nIndex++) {
		delete[] m_pTableUniverses[nIndex].pIpAddresses;
		m_pTableUniverses[nIndex].pIpAddresses = nullptr;
	}

	delete[] m_pTableUniverses;
	m_pTableUniverses = nullptr;

	delete[] m_pPollTable;
	m_pPollTable = nullptr;
}

const struct artnet::PollTableUniverses *ArtNetPollTable::GetIpAddress(uint16_t nUniverse) const {
	if (m_nTableUniversesEntries == 0) {
		return nullptr;
	}

	// FIXME Universe lookup
	for (uint32_t nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		artnet::PollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			return &m_pTableUniverses[nEntry];
		}
	}

	return nullptr;
}


void ArtNetPollTable::RemoveIpAddress(const uint16_t nUniverse, const uint32_t nIpAddress) {
	if (m_nTableUniversesEntries == 0) {
		return;
	}

	uint32_t nEntry = 0;

	// FIXME Universe lookup
	for (nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		artnet::PollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			break;
		}
	}

	if (nEntry == m_nTableUniversesEntries) {
		// Universe not found
		return;
	}

	artnet::PollTableUniverses *pTableUniverses = &m_pTableUniverses[nEntry];
	assert(pTableUniverses->nCount > 0);

	uint32_t nIpAddressIndex = 0;

	// FIXME IP lookup
	for (nIpAddressIndex = 0; nIpAddressIndex < pTableUniverses->nCount; nIpAddressIndex++) {
		if (pTableUniverses->pIpAddresses[nIpAddressIndex] == nIpAddress) {
			break;
		}
	}

	auto *p32 = pTableUniverses->pIpAddresses;
	int32_t i;

	for (i = static_cast<int32_t>(nIpAddressIndex); i < static_cast<int32_t>(pTableUniverses->nCount) - 1; i++) {
		p32[i] = p32[i + 1];
	}

	p32[i] = 0;

	pTableUniverses->nCount--;

	if (pTableUniverses->nCount == 0) {
		DEBUG_PRINTF("Delete Universe -> m_nTableUniversesEntries=%u, nEntry=%u", m_nTableUniversesEntries, nEntry);

		artnet::PollTableUniverses *p = m_pTableUniverses;

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

void ArtNetPollTable::ProcessUniverse(const uint32_t nIpAddress, const uint16_t nUniverse) {
	DEBUG_ENTRY

	if (artnet::POLL_TABLE_SIZE_UNIVERSES == m_nTableUniversesEntries) {
		DEBUG_PUTS("m_pTableUniverses is full");
		DEBUG_EXIT
		return;
	}

	// FIXME Universe lookup
	auto bFoundUniverse = false;
	uint32_t nEntry = 0;

	for (nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		auto *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		if (pTableUniverses->nUniverse == nUniverse) {
			bFoundUniverse = true;
			DEBUG_PRINTF("Universe found %u", nUniverse);
			break;
		}
	}

	auto *pTableUniverses = &m_pTableUniverses[nEntry];
	auto bFoundIp = false;
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
		if (pTableUniverses->nCount < artnet::POLL_TABLE_SIZE_ENRIES) {
			pTableUniverses->pIpAddresses[pTableUniverses->nCount] = nIpAddress;
			pTableUniverses->nCount++;
			DEBUG_PUTS("It is a new IP for the Universe");
		} else {
			DEBUG_PUTS("New IP does not fit");
		}
	}

	DEBUG_EXIT
}

void ArtNetPollTable::Add(const struct artnet::ArtPollReply *ptArtPollReply) {
	DEBUG_ENTRY

	auto bFound = false;

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
			break;
		}
	}

	if (!bFound) {
		if (m_nPollTableEntries == artnet::POLL_TABLE_SIZE_ENRIES) {
			DEBUG_PUTS("Full");
			return;
		}

		if (m_nPollTableEntries != static_cast<uint32_t>(nHigh)) {
			DEBUG_PUTS("Move");

			auto *pArtNetNodeEntry = m_pPollTable; // TODO

			assert(m_nPollTableEntries >= 1);
			assert(nLow >= 0);

			for (int32_t i = static_cast<int32_t>(m_nPollTableEntries) - 1;i >= nLow; i--) {
				const struct artnet::NodeEntry *pSrc = &pArtNetNodeEntry[i];
				struct artnet::NodeEntry *pDst = &pArtNetNodeEntry[i + 1];
				memcpy(pDst, pSrc, sizeof(struct artnet::NodeEntry));
			}

			auto *pDst = &pArtNetNodeEntry[nLow];
			memset(pDst, 0, sizeof(struct artnet::NodeEntry));

			i = nLow;
		} else {
			i = static_cast<int32_t>(m_nPollTableEntries);
			DEBUG_PRINTF("Add -> i=%d", i);
		}

		m_pPollTable[i].IPAddress = ip.u32;
		m_nPollTableEntries++;
	}

	if (ptArtPollReply->BindIndex <= 1) {
		memcpy(m_pPollTable[i].Mac, ptArtPollReply->MAC, artnet::MAC_SIZE);
		const uint8_t *pSrc = ptArtPollReply->LongName;
		uint8_t *pDst = m_pPollTable[i].LongName;
		memcpy(pDst, pSrc, artnet::LONG_NAME_LENGTH);
	}

	const auto nMillis = Hardware::Get()->Millis();

	for (uint32_t nIndex = 0; nIndex < artnet::PORTS; nIndex++) {
		const auto nPortAddress = ptArtPollReply->SwOut[nIndex];

		if (ptArtPollReply->PortTypes[nIndex] == static_cast<uint8_t>(artnet::PortType::OUTPUT_ARTNET)) {
			const auto nUniverse = artnet::make_port_address(ptArtPollReply->NetSwitch, ptArtPollReply->SubSwitch, nPortAddress);

			uint32_t nIndexUniverse;

			for (nIndexUniverse = 0; nIndexUniverse < m_pPollTable[i].nUniversesCount; nIndexUniverse++) {
				if (m_pPollTable[i].Universe[nIndexUniverse].nUniverse == nUniverse) {
					break;
				}
			}

			if (nIndexUniverse == m_pPollTable[i].nUniversesCount) {
				// Not found
				if (m_pPollTable[i].nUniversesCount < artnet::POLL_TABLE_SIZE_NODE_UNIVERSES) {
					m_pPollTable[i].nUniversesCount++;
					m_pPollTable[i].Universe[nIndexUniverse].nUniverse = nUniverse;
					const uint8_t *pSrc = ptArtPollReply->ShortName;
					uint8_t *pDst = m_pPollTable[i].Universe[nIndexUniverse].ShortName;
					memcpy(pDst, pSrc, artnet::SHORT_NAME_LENGTH);
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

	assert(m_PollTableClean.nTableIndex < m_nPollTableEntries);
	assert(m_PollTableClean.nUniverseIndex < artnet::POLL_TABLE_SIZE_NODE_UNIVERSES);

	if (m_PollTableClean.nUniverseIndex == 0) {
		m_PollTableClean.bOffLine = true;
	}

	auto *pArtNetNodeEntryBind = &m_pPollTable[m_PollTableClean.nTableIndex].Universe[m_PollTableClean.nUniverseIndex];

	if (pArtNetNodeEntryBind->nLastUpdateMillis != 0) {
		if ((Hardware::Get()->Millis() - pArtNetNodeEntryBind->nLastUpdateMillis) > (1.5 * artnet::POLL_INTERVAL_MILLIS)) {
			pArtNetNodeEntryBind->nLastUpdateMillis = 0;
			RemoveIpAddress(pArtNetNodeEntryBind->nUniverse, m_pPollTable[m_PollTableClean.nTableIndex].IPAddress);
		} else {
			m_PollTableClean.bOffLine = false;
		}
	}

	m_PollTableClean.nUniverseIndex++;

	if (m_PollTableClean.nUniverseIndex == artnet::POLL_TABLE_SIZE_NODE_UNIVERSES) {
		if (m_PollTableClean.bOffLine) {
			DEBUG_PUTS("Node is off-line");

			auto *pArtNetNodeEntry = m_pPollTable;
			// Move
			for (uint32_t i = m_PollTableClean.nTableIndex; i < (m_nPollTableEntries - 1); i++) {
				const auto *pSrc = &pArtNetNodeEntry[i + 1];
				auto *pDst = &pArtNetNodeEntry[i];
				memcpy(pDst, pSrc, sizeof(struct artnet::NodeEntry));
			}

			m_nPollTableEntries--;

			auto *pDst = &pArtNetNodeEntry[m_nPollTableEntries];
			pDst->IPAddress = 0;
			pDst->nUniversesCount = 0;
			memset(pDst->Universe, 0, sizeof(struct artnet::NodeEntryUniverse[artnet::POLL_TABLE_SIZE_NODE_UNIVERSES]));
#ifndef NDEBUG
			memset(pDst->Mac, 0, artnet::MAC_SIZE + artnet::LONG_NAME_LENGTH);
#endif
		}

		m_PollTableClean.nUniverseIndex = 0;
		m_PollTableClean.bOffLine = true;
		m_PollTableClean.nTableIndex++;

		if (m_PollTableClean.nTableIndex >= m_nPollTableEntries) {
			m_PollTableClean.nTableIndex = 0;
		}
	}
}

void ArtNetPollTable::Dump() {
#ifndef NDEBUG
	printf("Entries : %d\n", m_nPollTableEntries);

	for (uint32_t i = 0; i < m_nPollTableEntries; i++) {
		printf("\t" IPSTR " [" MACSTR "] |%-64s|\n", IP2STR(m_pPollTable[i].IPAddress), MAC2STR(m_pPollTable[i].Mac), m_pPollTable[i].LongName);

		for (uint32_t nUniverse = 0; nUniverse < m_pPollTable[i].nUniversesCount; nUniverse++) {
			auto *pArtNetNodeEntryUniverse = &m_pPollTable[i].Universe[nUniverse];
			printf("\t %u [%u] |%-18s|\n", pArtNetNodeEntryUniverse->nUniverse, (Hardware::Get()->Millis() - pArtNetNodeEntryUniverse->nLastUpdateMillis) / 1000U, pArtNetNodeEntryUniverse->ShortName);
		}
		puts("");
	}
#endif
}

void ArtNetPollTable::DumpTableUniverses() {
#ifndef NDEBUG
	printf("Entries : %d\n", m_nTableUniversesEntries);

	for (uint32_t nEntry = 0; nEntry < m_nTableUniversesEntries; nEntry++) {
		const auto *pTableUniverses = &m_pTableUniverses[nEntry];
		assert(pTableUniverses != nullptr);

		printf("%3d |%4u | %d ", nEntry, pTableUniverses->nUniverse, pTableUniverses->nCount);

		const auto *pIpAddresses = pTableUniverses->pIpAddresses;
		assert(pIpAddresses != nullptr);

		for (uint32_t nCount = 0; nCount < pTableUniverses->nCount; nCount++) {
			printf(" " IPSTR, IP2STR(pIpAddresses[nCount]));
		}

		puts("");
	}

	puts("");
#endif
}
