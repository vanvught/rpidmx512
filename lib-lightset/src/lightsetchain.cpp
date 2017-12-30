/**
 * @file lightsetchain.cpp
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

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "lightsetchain.h"

#define LIGHTSET_CHAIN_MAX_ENTRIES	16

LightSetChain::LightSetChain(void): m_nSize(0) {
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

void LightSetChain::Start(void) {
	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->Start();
	}
}

void LightSetChain::Stop(void) {
	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->Stop();
	}
}

void LightSetChain::SetData(uint8_t nPort, const uint8_t *pData, uint16_t nSize) {
	assert(pData != 0);

	for (unsigned i = 0; i < m_nSize; i++) {
		m_pTable[i].pLightSet->SetData(nPort, pData, nSize);
	}
}

bool LightSetChain::Add(LightSet *pLightSet, int nType) {
	if (m_nSize == LIGHTSET_CHAIN_MAX_ENTRIES) {
		return false;
	}

	if (pLightSet != 0) {
		m_pTable[m_nSize].pLightSet = pLightSet;
		m_pTable[m_nSize].nType = nType;
		m_nSize++;
		return true;
	}

	return false;
}

void LightSetChain::Clear(void) {
	for (unsigned i = 0; i < m_nSize ; i++) {
		m_pTable[i].pLightSet = 0;
		m_pTable[i].nType = LIGHTSET_TYPE_UNDEFINED;
	}

	m_nSize = 0;
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

bool LightSetChain::Remove(LightSet *pLightSet) {
	return Remove(pLightSet, LIGHTSET_TYPE_UNDEFINED, true);
}

bool LightSetChain::Remove(LightSet *pLightSet, int nType, bool DoIgnoreType) {
	bool found = false;
	unsigned i;

	for (i = 0; i < m_nSize; i++) {
		if ((m_pTable[i].pLightSet == pLightSet) && (DoIgnoreType || (m_pTable[i].nType == nType))) {
			found = true;
			break;
		}
	}

	if (!found) {
		return false;
	}

	if (i == LIGHTSET_CHAIN_MAX_ENTRIES - 1) {
		m_pTable[i].pLightSet = 0;
		m_pTable[i].nType = LIGHTSET_TYPE_UNDEFINED;
	} else {
		for (; i < m_nSize; i++) {
			m_pTable[i].pLightSet = m_pTable[i + 1].pLightSet;
			m_pTable[i].nType = m_pTable[i + 1].nType;
		}
	}

	m_nSize--;

	return true;
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


#ifndef NDEBUG
void LightSetChain::Dump(uint8_t nEntries) {
	if (nEntries > LIGHTSET_CHAIN_MAX_ENTRIES) {
		nEntries = LIGHTSET_CHAIN_MAX_ENTRIES;
	}

	printf("Max size = %d, Current size = %d\n\n", (int) LIGHTSET_CHAIN_MAX_ENTRIES, (int) m_nSize);

	printf("Index\tPointer\tType\n");

	for (unsigned i = 0; i < nEntries ; i++) {
		printf("%d\t%p\t%d\n", (int) i, m_pTable[i].pLightSet, (int) m_pTable[i].nType);
	}
}

void LightSetChain::Dump(void) {
	Dump(m_nSize);
}

#endif
