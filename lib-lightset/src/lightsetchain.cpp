/**
 * @file lightsetchain.cpp
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
#include <assert.h>

#include "lightsetchain.h"
#include "lightset.h"

#include "debug.h"

#ifndef MAX
 #define MAX(a,b)	(((a) > (b)) ? (a) : (b))
 #define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#define LIGHTSET_CHAIN_MAX_ENTRIES	16

LightSetChain::LightSetChain(void): m_nSize(0), m_nDmxStartAddress(DMX_ADDRESS_INVALID), m_nDmxFootprint(0) { // Invalidate DMX Start Address and DMX Footprint
	m_pTable = new TLightSetEntry[LIGHTSET_CHAIN_MAX_ENTRIES];
	assert(m_pTable != 0);

	for (unsigned i = 0; i < LIGHTSET_CHAIN_MAX_ENTRIES ; i++) {
		m_pTable[i].pLightSet = 0;
		m_pTable[i].nType = LIGHTSET_TYPE_UNDEFINED;
	}
}

LightSetChain::~LightSetChain(void) {
	delete[] m_pTable;
	m_pTable = 0;
	m_nSize = 0;
}

void LightSetChain::Start(uint8_t nPort) {
	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->Start(nPort);
	}
}

void LightSetChain::Stop(uint8_t nPort) {
	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->Stop(nPort);
	}
}

void LightSetChain::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nSize) {
	assert(pData != 0);

	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->SetData(nPort, pData, nSize);
	}
}

bool LightSetChain::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG1_ENTRY

	if (nDmxStartAddress == m_nDmxStartAddress) {
		DEBUG1_EXIT
		return true;
	}

	for (unsigned i = 0; i < m_nSize; i++) {
		const uint16_t nCurrentDmxStartAddress = m_pTable[i].pLightSet->GetDmxStartAddress();
		const uint16_t nNewDmxStartAddress =  (nCurrentDmxStartAddress - m_nDmxStartAddress) + nDmxStartAddress;

		m_pTable[i].pLightSet->SetDmxStartAddress(nNewDmxStartAddress);
	}

	m_nDmxStartAddress = nDmxStartAddress;

	DEBUG1_EXIT
	return true;;
}

bool LightSetChain::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) {
	DEBUG1_ENTRY

	if (nSlotOffset > m_nDmxFootprint) {
		return false;
	}

	for (unsigned i = 0; i < m_nSize; i++) {
		const uint16_t nDmxAddress = m_nDmxStartAddress + nSlotOffset;
		const int16_t nOffset = nDmxAddress - m_pTable[i].pLightSet->GetDmxStartAddress();

#ifndef NDEBUG
		printf("\tnSlotOffset=%d, m_nDmxStartAddress=%d, m_pTable[%d].pLightSet->GetDmxStartAddress()=%d, m_pTable[%d].pLightSet->GetDmxFootprint()=%d\n",
				(int) nSlotOffset,
				(int) m_nDmxStartAddress,
				(int) i,
				(int) m_pTable[i].pLightSet->GetDmxStartAddress(),
				(int) i,
				(int) m_pTable[i].pLightSet->GetDmxFootprint()) ;

		printf("\tnOffset=%d\n", nOffset);
#endif

		if ((m_pTable[i].pLightSet->GetDmxStartAddress() + m_pTable[i].pLightSet->GetDmxFootprint() <= nDmxAddress) || (nOffset < 0)){
#ifndef NDEBUG
			printf("\t[continue]\n");
#endif
			continue;
		}

		if (m_pTable[i].pLightSet->GetSlotInfo(nOffset, tSlotInfo)) {
			DEBUG1_EXIT
			return true;
		}
	}

	DEBUG1_EXIT
	return false;
}

bool LightSetChain::Add(LightSet *pLightSet, int nType) {
	DEBUG1_ENTRY

	if (m_nSize == LIGHTSET_CHAIN_MAX_ENTRIES) {
		DEBUG1_EXIT
		return false;
	}

	if (pLightSet != 0) {
#ifndef NDEBUG
		printf("m_nDmxStartAddress=%d, m_nDmxFootprint=%d\n", (int) m_nDmxStartAddress,(int) m_nDmxFootprint);
#endif
		const bool IsValidDmxStartAddress = (pLightSet->GetDmxStartAddress() > 0) && (pLightSet->GetDmxFootprint() - pLightSet->GetDmxStartAddress() < 512);

		if (IsValidDmxStartAddress) {
			if (m_nDmxStartAddress == DMX_ADDRESS_INVALID) {

				m_pTable[0].pLightSet = pLightSet;
				m_pTable[0].nType = nType;
				m_nSize = 1;

				m_nDmxStartAddress = pLightSet->GetDmxStartAddress();
				m_nDmxFootprint = pLightSet->GetDmxFootprint();

#ifndef NDEBUG
				printf("m_nDmxStartAddress=%d, m_nDmxFootprint=%d\n", (int) m_nDmxStartAddress, (int) m_nDmxFootprint);
#endif
				DEBUG1_EXIT
				return true;
			}

			m_pTable[m_nSize].pLightSet = pLightSet;
			m_pTable[m_nSize].nType = nType;
			m_nSize++;

#ifndef NDEBUG
			printf("pLightSet->GetDmxStartAddress()=%d, pLightSet->GetDmxFootprint()=%d\n", (int) pLightSet->GetDmxStartAddress(),(int) pLightSet->GetDmxFootprint());
#endif
			const uint16_t nDmxChannelLastCurrent = m_nDmxStartAddress + m_nDmxFootprint;
			m_nDmxStartAddress = MIN(m_nDmxStartAddress, pLightSet->GetDmxStartAddress());

			const uint16_t nDmxChannelLast = pLightSet->GetDmxStartAddress() + pLightSet->GetDmxFootprint();
			m_nDmxFootprint = MAX(nDmxChannelLastCurrent, nDmxChannelLast) - m_nDmxStartAddress;

#ifndef NDEBUG
			printf("m_nDmxStartAddress=%d, m_nDmxFootprint=%d\n", (int) m_nDmxStartAddress,(int) m_nDmxFootprint);
#endif
			DEBUG1_EXIT
			return true;
		} else {
			DEBUG1_EXIT
			return false;
		}
	}

	DEBUG1_EXIT
	return false;
}

uint8_t LightSetChain::GetSize(void) const {
	return m_nSize;
}

int LightSetChain::GetType(uint8_t nEntry) const {
	if (nEntry > LIGHTSET_CHAIN_MAX_ENTRIES) {
		return LIGHTSET_TYPE_UNDEFINED;
	}

	return m_pTable[nEntry].nType;
}

const LightSet* LightSetChain::GetLightSet(uint8_t nEntry) {
	if (nEntry > LIGHTSET_CHAIN_MAX_ENTRIES) {
		return 0;
	}

	return m_pTable[nEntry].pLightSet;
}

bool LightSetChain::IsEmpty(void) const {
	return (m_nSize == 0);
}

bool LightSetChain::Exist(LightSet *pLightSet) {
	return Exist(pLightSet, LIGHTSET_TYPE_UNDEFINED, true);
}

bool LightSetChain::Exist(LightSet *pLightSet , int nType, bool DoIgnoreType) {
	for (unsigned i = 0; i < m_nSize; i++) {
		if ((m_pTable[i].pLightSet == pLightSet) && (DoIgnoreType || (m_pTable[i].nType == nType))) {
			return true;
		}
	}

	return false;
}

void LightSetChain::Dump(uint8_t nEntries) {
#ifndef NDEBUG
	if (nEntries > LIGHTSET_CHAIN_MAX_ENTRIES) {
		nEntries = LIGHTSET_CHAIN_MAX_ENTRIES;
	}

	printf("Max size = %d, Current size = %d\n\n", (int) LIGHTSET_CHAIN_MAX_ENTRIES, (int) m_nSize);
	printf("Index\tPointer\t\tType\n");

	for (unsigned i = 0; i < nEntries ; i++) {
		printf("%d\t%p\t%d\n", (int) i, m_pTable[i].pLightSet, (int) m_pTable[i].nType);
	}

	printf("\n");
#endif
}

void LightSetChain::Dump(void) {
#ifndef NDEBUG
	Dump(m_nSize);
#endif
}
