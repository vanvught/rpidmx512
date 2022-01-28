/**
 * @file rdmtod.h
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

#ifndef RDMTOD_H_
#define RDMTOD_H_

#include <cstdint>
#include <cstring>
#ifndef NDEBUG
# include <cstdio>
#endif

#include "rdm.h"

namespace rdmtod {
#if !defined (RDM_DISCOVERY_TOD_TABLE_SIZE)
# define RDM_DISCOVERY_TOD_TABLE_SIZE 200U
#endif
static constexpr uint8_t TOD_TABLE_SIZE = RDM_DISCOVERY_TOD_TABLE_SIZE;
struct TRdmTod {
	uint8_t uid[RDM_UID_SIZE];
};
}  // namespace rdmtod

class RDMTod {
public:
	RDMTod() {
		m_pTable = new rdmtod::TRdmTod[rdmtod::TOD_TABLE_SIZE];

		for (uint32_t i = 0; i < rdmtod::TOD_TABLE_SIZE; i++) {
			memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
		}
	}

	~RDMTod() {
		m_nEntries = 0;
		delete[] m_pTable;
	}

	void Reset() {
		for (uint32_t i = 0; i < m_nEntries; i++) {
			memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
		}

		m_nEntries = 0;
	}

	bool AddUid(const uint8_t *pUid) {
		if (m_nEntries == rdmtod::TOD_TABLE_SIZE) {
			return false;
		}

		if (Exist(pUid)) {
			return false;
		}

		memcpy(&m_pTable[m_nEntries++], pUid, RDM_UID_SIZE);

		return true;
	}

	uint32_t GetUidCount() const {
		return m_nEntries;
	}

	void Copy(uint8_t *pTable) {
		const auto *pSrc = reinterpret_cast<const uint8_t*>(m_pTable);
		uint8_t *pDst = pTable;

		for (uint32_t i = 0; i < (m_nEntries * RDM_UID_SIZE); i++) {
			*pDst++ = *pSrc++;
		}
	}

	bool Delete(const uint8_t *pUid) {
		bool found = false;
		uint32_t i;

		for (i = 0; i < m_nEntries; i++) {
			if (memcmp(&m_pTable[i], pUid, RDM_UID_SIZE) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}

		if (i == rdmtod::TOD_TABLE_SIZE - 1) {
			memcpy(&m_pTable[i], UID_ALL, RDM_UID_SIZE);
		} else {
			for (; i < m_nEntries; i++) {
				memcpy(&m_pTable[i], &m_pTable[i + 1], RDM_UID_SIZE);
			}
		}

		m_nEntries--;

		return true;
	}

	bool Exist(const uint8_t *pUid) {
		for (uint32_t i = 0 ; i < m_nEntries; i++) {
			if (memcmp(&m_pTable[i], pUid, RDM_UID_SIZE) == 0) {
				return true;
			}
		}

		return false;
	}

	void Dump(__attribute__((unused)) uint8_t nCount) {
#ifndef NDEBUG
	if (nCount > rdmtod::TOD_TABLE_SIZE) {
		nCount = rdmtod::TOD_TABLE_SIZE;
	}

	for (uint32_t i = 0 ; i < nCount; i++) {
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x\n", m_pTable[i].uid[0], m_pTable[i].uid[1], m_pTable[i].uid[2], m_pTable[i].uid[3], m_pTable[i].uid[4], m_pTable[i].uid[5]);
	}
#endif
	}
	void Dump() {
#ifndef NDEBUG
		Dump(m_nEntries);
#endif
	}

private:
	uint32_t m_nEntries { 0 };
	rdmtod::TRdmTod *m_pTable;
};

#endif /* RDMTOD_H_ */
