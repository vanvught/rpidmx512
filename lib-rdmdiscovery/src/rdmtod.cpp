/**
 * @file rdmtod.cpp
 *
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
#ifndef NDEBUG
# include <stdio.h>
#endif

#include "rdmtod.h"

RDMTod::RDMTod()  {
	m_pTable = new TRdmTod[TOD_TABLE_SIZE];

	for (uint32_t i = 0 ; i < TOD_TABLE_SIZE; i++) {
		memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
	}
}

RDMTod::~RDMTod() {
	m_nEntries = 0;
	delete[] m_pTable;
}

uint8_t RDMTod::GetUidCount() const {
	return m_nEntries;
}

bool RDMTod::Exist(const uint8_t *pUid) {
	for (uint32_t i = 0 ; i < m_nEntries; i++) {
		if (memcmp(&m_pTable[i], pUid, RDM_UID_SIZE) == 0) {
			return true;
		}
	}

	return false;
}

void RDMTod::Dump(__attribute__((unused)) uint8_t nCount) {
#ifndef NDEBUG
	if (nCount > TOD_TABLE_SIZE) {
		nCount = TOD_TABLE_SIZE;
	}

	for (uint32_t i = 0 ; i < nCount; i++) {
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x\n", m_pTable[i].uid[0], m_pTable[i].uid[1], m_pTable[i].uid[2], m_pTable[i].uid[3], m_pTable[i].uid[4], m_pTable[i].uid[5]);
	}
#endif
}

void RDMTod::Dump() {
#ifndef NDEBUG
	Dump(m_nEntries);
#endif
}

bool RDMTod::AddUid(const uint8_t *pUid) {
	if (m_nEntries == TOD_TABLE_SIZE) {
		return false;
	}

	if (Exist(pUid)) {
		return false;
	}

	memcpy(&m_pTable[m_nEntries++], pUid, RDM_UID_SIZE);

	return true;
}

bool RDMTod::Delete(const uint8_t *pUid) {
	bool found = false;
	uint32_t i;

	for (i = 0 ; i < m_nEntries; i++) {
		if (memcmp(&m_pTable[i], pUid, RDM_UID_SIZE) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		return false;
	}

	if (i == TOD_TABLE_SIZE - 1) {
		memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
	} else {
		for (; i < m_nEntries; i++) {
			memcpy(&m_pTable[i], &m_pTable[i + 1], RDM_UID_SIZE);
		}
	}

	m_nEntries--;

	return true;
}

void RDMTod::Copy(uint8_t *pTable) {
	const auto *pSrc = reinterpret_cast<const uint8_t*>(m_pTable);
	uint8_t *pDst = pTable;

	for (uint32_t i = 0; i < (m_nEntries * RDM_UID_SIZE); i++) {
		*pDst++ = *pSrc++;
	}
}

void RDMTod::Reset() {
	for (uint32_t i = 0 ; i < m_nEntries; i++) {
		memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
	}

	m_nEntries = 0;
}
