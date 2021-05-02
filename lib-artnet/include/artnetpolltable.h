/**
 * @file artnetpolltable.h
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

#ifndef ARTNETPOLLTABLE_H_
#define ARTNETPOLLTABLE_H_

#include <stdint.h>

#include "packets.h"

enum TArtNetPollInterval {
	ARTNET_POLL_INTERVAL_SECONDS = 8,
	ARTNET_POLL_INTERVAL_MILLIS = (ARTNET_POLL_INTERVAL_SECONDS * 1000)
};

enum TArtNetPollTableSizes {
	ARTNET_POLL_TABLE_SIZE_ENRIES = 255,
	ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES = 64,
	ARTNET_POLL_TABLE_SIZE_UNIVERSES = 512
};

struct TArtNetNodeEntryUniverse {
	uint32_t nLastUpdateMillis;
	uint16_t nUniverse;
};

struct TArtNetNodeEntry {
	uint32_t IPAddress;
	uint8_t Mac[ArtNet::MAC_SIZE];
	uint8_t ShortName[ArtNet::SHORT_NAME_LENGTH];
	uint8_t LongName[ArtNet::LONG_NAME_LENGTH];
	uint32_t nUniversesCount;
	struct TArtNetNodeEntryUniverse Universe[ARTNET_POLL_TABLE_SIZE_NODE_UNIVERSES];
};

struct TArtNetPollTableUniverses {
	uint16_t nUniverse;
	uint16_t nCount;
	uint32_t *pIpAddresses;
};

struct TArtNetPollTableClean {
	uint32_t nTableIndex;
	uint32_t nUniverseIndex;
	bool bOffLine;
};

class ArtNetPollTable {
public:
	ArtNetPollTable();
	~ArtNetPollTable();

	uint32_t GetEntries() {
		return m_nPollTableEntries;
	}

	void Add(const struct TArtPollReply *ptArtPollReply);
	void Clean();

	const struct TArtNetPollTableUniverses *GetIpAddress(uint16_t nUniverse);

	void Dump();
	void DumpTableUniverses();

private:
	uint16_t MakePortAddress(uint8_t nNetSwitch, uint8_t nSubSwitch, uint8_t nUniverse);
	void ProcessUniverse(uint32_t nIpAddress, uint16_t nUniverse);
	void RemoveIpAddress(uint32_t nEntry, uint32_t nIpAddressIndex);

private:
	TArtNetNodeEntry *m_pPollTable;
	uint32_t m_nPollTableEntries{0};
	TArtNetPollTableUniverses *m_pTableUniverses;
	uint32_t m_nTableUniversesEntries{0};
	TArtNetPollTableClean m_tTableClean;
};

#endif /* ARTNETPOLLTABLE_H_ */
