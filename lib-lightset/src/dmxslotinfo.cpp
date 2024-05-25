/**
 * @file dmxslotinfo.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cassert>

#include "dmxslotinfo.h"

#include "lightset.h"

#include "debug.h"

using namespace lightset;

#define TO_HEX(i)	static_cast<char>(((i) < 10) ? '0' + (i) : 'A' + ((i) - 10))

DmxSlotInfo::DmxSlotInfo(SlotInfo *pSlotInfo, uint32_t nSize): m_pSlotInfo(pSlotInfo), m_nSize(nSize) {
	DEBUG_ENTRY

	assert(m_pSlotInfo != nullptr);
	assert(m_nSize != 0);

	for (uint32_t i = 0; i < m_nSize; i++) {
		m_pSlotInfo[i].nType = 0x00;		// ST_PRIMARY
		m_pSlotInfo[i].nCategory = 0xFFFF;	// SD_UNDEFINED
	}

	DEBUG_EXIT
}

DmxSlotInfo::~DmxSlotInfo() {
	DEBUG_ENTRY

	if (m_pSlotInfo != nullptr) {
		delete[] m_pSlotInfo;
		m_pSlotInfo = nullptr;
	}

	if (m_pToString != nullptr) {
		delete[] m_pToString;
		m_pToString = nullptr;
	}

	DEBUG_EXIT
}

void  DmxSlotInfo::FromString(const char *pString, uint32_t &nMask) {
	assert(pString != nullptr);

	auto *pSlotInfoRaw = const_cast<char*>(pString);
	nMask = 0;


	for (uint32_t i = 0; i < m_nSize; i++) {
		bool isSet = false;
		SlotInfo tLightSetSlotInfo;

		if (pSlotInfoRaw == nullptr) {
			break;
		}

		pSlotInfoRaw = Parse(pSlotInfoRaw, isSet, tLightSetSlotInfo);

		if (isSet) {
			m_pSlotInfo[i].nType = tLightSetSlotInfo.nType;
			m_pSlotInfo[i].nCategory = tLightSetSlotInfo.nCategory;
			nMask |= (1U << i);
		}
	}

	DEBUG_PRINTF("nMask=0x%x", static_cast<int>(nMask));
}

const char *DmxSlotInfo::ToString(uint32_t nMask) {
	if (m_pToString == nullptr) {
		m_pToString = new char[m_nSize * 7];
		assert(m_pToString != nullptr);

		m_pToString[0] = '\0';
	}

	DEBUG_PRINTF("nMask=0x%x", nMask);

	if (nMask == 0) {
		m_pToString[0] = '\0';
		return m_pToString;
	}

	auto *p = m_pToString;

	for (uint32_t i = 0; i < m_nSize; i++) {
		if ((nMask & 0x1) == 0x1) {
			const auto nType = m_pSlotInfo[i].nType;
			const auto nCategory = m_pSlotInfo[i].nCategory;

			*p++ = TO_HEX((nType & 0xF0) >> 4);
			*p++ = TO_HEX(nType & 0x0F);
			*p++ = ':';
			*p++ = TO_HEX((nCategory & 0xF000) >> 12);
			*p++ = TO_HEX((nCategory & 0x0F00) >> 8);
			*p++ = TO_HEX((nCategory & 0x00F0) >> 4);
			*p++ = TO_HEX(nCategory & 0x000F);
			*p++ = ',';
		}

		nMask = nMask >> 1;
	}

	p--;
	*p = '\0';

	assert(static_cast<uint32_t>(p - m_pToString) <= (m_nSize * 7U));

	return m_pToString;
}

void DmxSlotInfo::Dump() {
	for (uint32_t i = 0; i < m_nSize; i++) {
		printf("  Slot:%u %.2X:%.4X\n", static_cast<unsigned int>(i), m_pSlotInfo[i].nType, m_pSlotInfo[i].nCategory);
	}
}

char *DmxSlotInfo::Parse(char *s, bool &isValid, SlotInfo &tLightSetSlotInfo) {
	assert(s != nullptr);

	auto *b = s;
	uint8_t i = 0;

	uint16_t nTmp = 0;

	while ((i < 2) && (*b != ':')) {
		if (isxdigit(*b) == 0) {
			isValid = false;
			return nullptr;
		}

		const auto nibble = *b > '9' ? static_cast<uint8_t>((*b | 0x20) - 'a' + 10) : static_cast<uint8_t>(*b - '0');
		nTmp = static_cast<uint16_t>((nTmp << 4) | nibble);
		b++;
		i++;
	}

	if ((i != 2) && (*b != ':')) {
		isValid = false;
		return nullptr;
	}

	tLightSetSlotInfo.nType = static_cast<uint8_t>(nTmp);

	i = 0;
	nTmp = 0;

	b++;

	while ((i < 4) && (*b != ',') && (*b != '\0')) {
		if (isxdigit(*b) == 0) {
			isValid = false;
			return nullptr;
		}

		const auto nibble = *b > '9' ? static_cast<uint8_t>((*b | 0x20) - 'a' + 10) : static_cast<uint8_t>(*b - '0');
		nTmp = static_cast<uint16_t>((nTmp << 4) | nibble);
		b++;
		i++;
	}

	if (i != 4) {
		isValid = false;
		return nullptr;
	}

	if ((*b != ',') && (*b != ' ') && (*b != '\0')) {
		isValid = false;
		return nullptr;
	}

	tLightSetSlotInfo.nCategory = nTmp;

	isValid = true;

	if (*b == '\0') {
		return nullptr;
	}

	return ++b;
}
