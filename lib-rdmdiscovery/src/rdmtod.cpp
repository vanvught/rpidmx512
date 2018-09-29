/**
 * @file rdmtod.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif

#include "rdmtod.h"

#if defined (BARE_METAL)
 #include "util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

RDMTod::RDMTod(void) : m_entries(0) {
	m_pTable = new TRdmTod[TOD_TABLE_SIZE];

	for (unsigned i = 0 ; i < TOD_TABLE_SIZE; i++) {
		memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
	}
}

RDMTod::~RDMTod(void) {
	m_entries = 0;
	delete[] m_pTable;
}

const uint8_t RDMTod::GetUidCount(void) {
	return m_entries;
}

bool RDMTod::Exist(const uint8_t *uid) {
	unsigned i;

	for (i = 0 ; i < m_entries; i++) {
		if (memcmp(&m_pTable[i], uid, RDM_UID_SIZE) == 0) {
			return true;
		}
	}

	return false;
}

void RDMTod::Dump(uint8_t count) {
#ifndef NDEBUG
	if (count > TOD_TABLE_SIZE) {
		count = TOD_TABLE_SIZE;
	}

	for (unsigned i = 0 ; i < count; i++) {
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x\n", m_pTable[i].uid[0], m_pTable[i].uid[1], m_pTable[i].uid[2], m_pTable[i].uid[3], m_pTable[i].uid[4], m_pTable[i].uid[5]);
	}
#endif
}

void RDMTod::Dump(void) {
#ifndef NDEBUG
	Dump(m_entries);
#endif
}

bool RDMTod::AddUid(const uint8_t *uid) {
	if (m_entries == TOD_TABLE_SIZE) {
		return false;
	}

	if (Exist(uid)) {
		return false;
	}

	memcpy(&m_pTable[m_entries++], uid, RDM_UID_SIZE);

	return true;
}

bool RDMTod::Delete(const uint8_t *uid) {
	bool found = false;
	unsigned i;

	for (i = 0 ; i < m_entries; i++) {
		if (memcmp(&m_pTable[i], uid, RDM_UID_SIZE) == 0) {
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
		for (; i < m_entries; i++) {
			memcpy(&m_pTable[i], &m_pTable[i + 1], RDM_UID_SIZE);
		}
	}

	m_entries--;

	return true;
}

void RDMTod::Copy(uint8_t *table) {
	uint8_t *src = (uint8_t *) m_pTable;
	uint8_t *dst = (uint8_t *) table;
	for (unsigned i = 0; i < (m_entries * RDM_UID_SIZE); i++) {
		*dst++ = *src++;
	}
}

void RDMTod::Reset(void) {
	for (unsigned i = 0 ; i < m_entries; i++) {
		memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
	}

	m_entries = 0;
}
