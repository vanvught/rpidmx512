/**
 * @file artnetpolltable.h
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

#ifndef ARTNETPOLLTABLE_H_
#define ARTNETPOLLTABLE_H_

#include <cstdint>

#include "artnet.h"

namespace artnet {
static constexpr uint32_t POLL_INTERVAL_SECONDS = 8;
static constexpr uint32_t POLL_INTERVAL_MILLIS = (POLL_INTERVAL_SECONDS * 1000U);
static constexpr uint32_t POLL_TABLE_SIZE_ENRIES = 255;
static constexpr uint32_t POLL_TABLE_SIZE_NODE_UNIVERSES = 64;
static constexpr uint32_t POLL_TABLE_SIZE_UNIVERSES = 512;

struct NodeEntryUniverse {
	uint8_t ShortName[artnet::SHORT_NAME_LENGTH];
	uint32_t nLastUpdateMillis;
	uint16_t nUniverse;
};

struct NodeEntry {
	uint32_t IPAddress;
	uint8_t Mac[artnet::MAC_SIZE];
	uint8_t LongName[artnet::LONG_NAME_LENGTH];
	uint16_t nUniversesCount;
	struct NodeEntryUniverse Universe[artnet::POLL_TABLE_SIZE_NODE_UNIVERSES];
};

struct PollTableUniverses {
	uint16_t nUniverse;
	uint16_t nCount;
	uint32_t *pIpAddresses;
};

struct PollTableClean {
	uint32_t nTableIndex;
	uint16_t nUniverseIndex;
	bool bOffLine;
};
}  // namespace artnet

class ArtNetPollTable {
public:
	ArtNetPollTable();
	~ArtNetPollTable();

	const artnet::NodeEntry *GetPollTable() const {
		return m_pPollTable;
	}

	uint32_t GetPollTableEntries() const {
		return m_nPollTableEntries;
	}

	void Add(const struct artnet::ArtPollReply *ptArtPollReply);
	void Clean();

	const struct artnet::PollTableUniverses *GetIpAddress(uint16_t nUniverse) const;

	void Dump();
	void DumpTableUniverses();

private:
	void ProcessUniverse(const uint32_t nIpAddress, const uint16_t nUniverse);
	void RemoveIpAddress(const uint16_t nUniverse, const uint32_t nIpAddress);

private:
	artnet::NodeEntry *m_pPollTable;
	artnet::PollTableUniverses *m_pTableUniverses;
	uint32_t m_nPollTableEntries { 0 };
	uint32_t m_nTableUniversesEntries { 0 };
	artnet::PollTableClean m_PollTableClean;
};

#endif /* ARTNETPOLLTABLE_H_ */
