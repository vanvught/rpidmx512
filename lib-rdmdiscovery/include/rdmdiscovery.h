/**
 * @file rdmddiscovery.h
 *
 */
/* Copyright (C) 2017-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMDISCOVERY_H_
#define RDMDISCOVERY_H_

#include <cstdint>

#include "rdm.h"

#include "rdmmessage.h"
#include "rdmtod.h"

class RDMDiscovery {
public:
	RDMDiscovery(const uint8_t *pUid);

	void Full(uint32_t m_nPortIndex, RDMTod *pRDMTod);

private:
	bool FindDevices(uint64_t LowerBound, uint64_t UpperBound);
	bool QuickFind(const uint8_t *pUid);

	bool IsValidDiscoveryResponse(const uint8_t *pDiscResponse, uint8_t *pUid);

	void PrintUid(uint64_t nUid);
	void PrintUid(const uint8_t *pUid);
	const uint8_t *ConvertUid(uint64_t nUid);
	uint64_t ConvertUid(const uint8_t *pUid);

private:
	RDMMessage m_Message;
	uint32_t m_nPortIndex;
	RDMTod *m_pRDMTod;
	uint8_t m_Uid[RDM_UID_SIZE];
	uint8_t m_Pdl[2][RDM_UID_SIZE];
};

#endif /* RDMDISCOVERY_H_ */
