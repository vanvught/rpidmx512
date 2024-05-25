/**
 * @file rdmtod.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>
#ifndef NDEBUG
# include <cstdio>
#endif

#include "rdmconst.h"

#include "debug.h"

namespace rdmtod {
#if !defined (RDM_DISCOVERY_TOD_TABLE_SIZE)
# define RDM_DISCOVERY_TOD_TABLE_SIZE 200U
#endif
static constexpr uint32_t TOD_TABLE_SIZE = RDM_DISCOVERY_TOD_TABLE_SIZE;
static constexpr uint32_t MUTES_TABLE_SIZE = (TOD_TABLE_SIZE + 32) / 32;
static constexpr uint32_t INVALID_ENTRY = static_cast<uint32_t>(~0);
struct Tod {
	uint8_t uid[RDM_UID_SIZE];
};
}  // namespace rdmtod

class RDMTod {
public:
	RDMTod() {
		for (uint32_t i = 0; i < rdmtod::TOD_TABLE_SIZE; i++) {
			memcpy(&m_Tod[i], UID_ALL, RDM_UID_SIZE);
		}

		for (uint32_t i = 0; i < rdmtod::MUTES_TABLE_SIZE; i++) {
			m_nMutes[i] = 0;
		}
	}

	~RDMTod() = default;

	void Reset() {
		for (uint32_t i = 0; i < m_nEntries; i++) {
			memcpy(&m_Tod[i], UID_ALL, RDM_UID_SIZE);
		}

		m_nEntries = 0;

		for (uint32_t i = 0; i < rdmtod::MUTES_TABLE_SIZE; i++) {
			m_nMutes[i] = 0;
		}
	}

	bool AddUid(const uint8_t *pUid) {
		if (m_nEntries == rdmtod::TOD_TABLE_SIZE) {
			return false;
		}

		if (Exist(pUid)) {
			return false;
		}

		memcpy(&m_Tod[m_nEntries++], pUid, RDM_UID_SIZE);

		return true;
	}

	uint32_t GetUidCount() const {
		return m_nEntries;
	}

	bool CopyUidEntry(uint32_t nIndex, uint8_t uid[RDM_UID_SIZE]) {
		if (nIndex > m_nEntries) {
			memcpy(uid, UID_ALL, RDM_UID_SIZE);
			return false;
		}

		memcpy(uid, &m_Tod[nIndex], RDM_UID_SIZE);
		return true;
	}

	void Copy(uint8_t *pTable) {
		DEBUG_ENTRY
		DEBUG_PRINTF("m_nEntries=%u", static_cast<unsigned int>(m_nEntries));
		assert(pTable != nullptr);

		const auto *pSrc = reinterpret_cast<const uint8_t*>(m_Tod);
		auto *pDst = pTable;

		for (uint32_t i = 0; i < (m_nEntries * RDM_UID_SIZE); i++) {
			*pDst++ = *pSrc++;
		}

		DEBUG_EXIT
	}

	bool Delete(const uint8_t *pUid) {
		bool found = false;
		uint32_t i;

		for (i = 0; i < m_nEntries; i++) {
			if (memcmp(&m_Tod[i], pUid, RDM_UID_SIZE) == 0) {
				found = true;
				break;
			}
		}

		if (!found) {
			return false;
		}

		if (i == rdmtod::TOD_TABLE_SIZE - 1) {
			memcpy(&m_Tod[i], UID_ALL, RDM_UID_SIZE);
		} else {
			for (; i < m_nEntries; i++) {
				memcpy(&m_Tod[i], &m_Tod[i + 1], RDM_UID_SIZE);
			}
		}

		m_nEntries--;

		return true;
	}

	bool Exist(const uint8_t *pUid) {
		for (uint32_t nIndex = 0 ; nIndex < m_nEntries; nIndex++) {
			if (memcmp(&m_Tod[nIndex], pUid, RDM_UID_SIZE) == 0) {
				m_nSavedIndex = nIndex;
				return true;
			}
		}

		m_nSavedIndex = rdmtod::INVALID_ENTRY;
		return false;
	}

	const uint8_t *Next() {
		m_nSavedIndex++;

		if (m_nSavedIndex == m_nEntries) {
			m_nSavedIndex = 0;
		}

		return m_Tod[m_nSavedIndex].uid;
	}

	void Mute() {
		if (m_nSavedIndex == rdmtod::INVALID_ENTRY) {
			return;
		}

		const auto i = m_nSavedIndex / 32;
		const auto shift = m_nSavedIndex - (i * 32);

		m_nMutes[i] |= (1U << shift);
	}

	void UnMute() {
		if (m_nSavedIndex == rdmtod::INVALID_ENTRY) {
			return;
		}

		const auto i = m_nSavedIndex / 32;
		const auto shift = m_nSavedIndex - (i * 32);

		m_nMutes[i] &= ~(1U << shift);
	}

	void UnMuteAll() {
		for (uint32_t i = 0; i < rdmtod::MUTES_TABLE_SIZE; i++) {
			m_nMutes[i] = 0;
		}
	}

	bool IsMuted() {
		if (m_nSavedIndex == rdmtod::INVALID_ENTRY) {
			return true;
		}

		const auto i = m_nSavedIndex / 32;
		const auto mutes = m_nMutes[i];
		const auto shift = m_nSavedIndex - (i * 32);

		return (mutes & (1U << shift)) == (1U << shift);
	}

	void Dump([[maybe_unused]] uint32_t nCount) {
#ifndef NDEBUG
	if (nCount > rdmtod::TOD_TABLE_SIZE) {
		nCount = rdmtod::TOD_TABLE_SIZE;
	}

	printf("[%u]\n", static_cast<unsigned int>(nCount));
	for (uint32_t i = 0 ; i < nCount; i++) {
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x\n", m_Tod[i].uid[0], m_Tod[i].uid[1], m_Tod[i].uid[2], m_Tod[i].uid[3], m_Tod[i].uid[4], m_Tod[i].uid[5]);
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
	uint32_t m_nSavedIndex { rdmtod::INVALID_ENTRY };
	uint32_t m_nMutes[rdmtod::MUTES_TABLE_SIZE];
	rdmtod::Tod m_Tod[rdmtod::TOD_TABLE_SIZE];
};

#endif /* RDMTOD_H_ */
