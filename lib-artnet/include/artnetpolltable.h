/*
 * artnetpolltable.h
 *
 *  Created on: Aug 11, 2017
 *      Author: pi
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
