/**
 * @file artnetpolltable.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 *
 * Art-Net 3 Protocol Release V1.4 Document Revision 1.4bk 23/1/2016
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "packets.h"

struct TIpProg {
	uint32_t IPAddress;
	uint32_t SubMask;
	uint8_t Status;
};

struct TArtNetNodeEntry {
	uint32_t IPAddress;
	uint8_t  Mac[ARTNET_MAC_SIZE];
	uint8_t  ShortName[ARTNET_SHORT_NAME_LENGTH];
	uint8_t  LongName[ARTNET_LONG_NAME_LENGTH];
	uint8_t  Status1;
	uint8_t  Status2;
	time_t	 LastUpdate;
	struct TIpProg IpProg;
};

class ArtNetPollTable {
public:
	ArtNetPollTable(void);
	~ArtNetPollTable(void);

	bool isChanged(void);
	const uint8_t GetEntries(void);
	bool GetEntry(const uint8_t, struct TArtNetNodeEntry *);

	bool Add(const struct TArtPollReply *);
	bool Add(const struct TArtIpProgReply *);

	void Dump(void);
private:
	bool m_bIsChanged;
	uint8_t m_nEntries;
	TArtNetNodeEntry *m_pPollTable;
	time_t m_nLastUpdate;
};

#endif /* ARTNETPOLLTABLE_H_ */
