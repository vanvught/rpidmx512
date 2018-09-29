/**
 * @file spiflashstore.cpp
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#include "spiflashstore.h"

#include "spi_flash.h"

#include "debug.h"

//#define SPI_FLASH_STORE_PAGE_SIZE	128

static uint8_t s_aSignature[] = {'A', 'v', 'V', 0x11};
static uint32_t s_aStorSize[STORE_LAST] = {96, 96, 48, 128};
static uint32_t s_aStorUsedSize[STORE_LAST] = {0, };

SpiFlashStore *SpiFlashStore::s_pThis = 0;

SpiFlashStore::SpiFlashStore(void): m_bHaveFlashChip(false), m_nStartAddress(0), m_tState(STATE_IDLE) {
	DEBUG_ENTRY

	s_pThis = this;

	if (spi_flash_probe(0, 0, 0) < 0) {
		DEBUG_PUTS("No SPI flash chip");
	} else {
		m_bHaveFlashChip = Init();
	}

	DEBUG_EXIT
}

SpiFlashStore::~SpiFlashStore(void) {
	DEBUG_ENTRY

	while (Flash())
		;

	DEBUG_EXIT
}

bool SpiFlashStore::Init(void) {
	const uint32_t nEraseSize = spi_flash_get_sector_size();
	assert(SPI_FLASH_STORE_DATA == nEraseSize);

	if (SPI_FLASH_STORE_DATA != nEraseSize) {
		return false;
	}

	m_nStartAddress = spi_flash_get_size() - nEraseSize;
	assert(!(m_nStartAddress % nEraseSize));

	if (m_nStartAddress % nEraseSize) {
		return false;
	}

	spi_flash_cmd_read_fast(m_nStartAddress, (size_t) SPI_FLASH_STORE_DATA, (void *) &m_aSpiFlashData);

	uint32_t i = sizeof(s_aSignature);
	bool bSignatureOK = true;

	while (i > 0) {
		if (s_aSignature[sizeof(s_aSignature) - i] != m_aSpiFlashData[SPI_FLASH_STORE_DATA - i]) {
			bSignatureOK = false;
		}
		m_aSpiFlashData[SPI_FLASH_STORE_DATA - i] = s_aSignature[sizeof(s_aSignature) - i];
		i--;
	}

	if (__builtin_expect(!bSignatureOK, 0)) {
		DEBUG_PUTS("No signature");

		// Clear bSetList
		for (uint32_t j = 0; j < STORE_LAST; j++) {
			const uint32_t nOffset = GetStoreOffset((enum TStore) j);
			uint32_t k = nOffset;
			m_aSpiFlashData[k++] = 0x00;
			m_aSpiFlashData[k++] = 0x00;
			m_aSpiFlashData[k++] = 0x00;
			m_aSpiFlashData[k++] = 0x00;

			// Clear rest of data
			for (; k < nOffset + s_aStorSize[j]; k++) {
				m_aSpiFlashData[k] = 0xFF;
			}
		}

		m_tState = STATE_CHANGED;

		while (Flash())
			;
	}

	return true;
}

uint32_t SpiFlashStore::GetStoreOffset(enum TStore tStore) {
	assert(tStore < STORE_LAST);

	uint32_t nOffset= 0;

	for (uint32_t i = 0 ; i < tStore; i++) {
		nOffset += s_aStorSize[i];
	}

	DEBUG_PRINTF("nOffset=%d", nOffset);

	return nOffset;
}

bool SpiFlashStore::HaveFlashChip(void) const {
	return m_bHaveFlashChip;
}

void SpiFlashStore::Update(enum TStore tStore, uint32_t nOffset, void* pData, uint32_t nDataLength, uint32_t bSetList) {
	DEBUG_ENTRY

	if (__builtin_expect((!m_bHaveFlashChip),0)) {
		return;
	}

	assert(tStore < STORE_LAST);
	assert(pData != 0);
	assert((nOffset + nDataLength) <= s_aStorSize[tStore]);

	DEBUG_PRINTF("%d:%p[%d]:%d", tStore, pData, nOffset, nDataLength);

	uint32_t nBase = nOffset + GetStoreOffset(tStore);

	bool bIsChanged = false;

	uint8_t *src = (uint8_t *) pData;
	uint8_t *dst = (uint8_t *) &m_aSpiFlashData[nBase];

	for (uint32_t i = 0; i < nDataLength; i++) {
		if (*src != *dst) {
			bIsChanged = true;
			*dst = *src;
		}
		dst++;
		src++;
	}

	if(bIsChanged && (m_tState != STATE_ERASED)) {
		m_tState = STATE_CHANGED;
	}

	if (0 == nOffset) {
		s_aStorUsedSize[tStore] = nDataLength;
	} else if (bIsChanged) {
		assert(bSetList != 0);

		uint32_t nOffsetSetList = GetStoreOffset(tStore);
		uint32_t *p = (uint32_t *) &m_aSpiFlashData[nOffsetSetList];
		*p |= bSetList;
	}

	DEBUG_EXIT
}

void SpiFlashStore::Copy(enum TStore tStore, void* pData, uint32_t nDataLength) {
	DEBUG_ENTRY

	if (__builtin_expect((!m_bHaveFlashChip),0)) {
		return;
	}

	assert(tStore < STORE_LAST);
	assert(pData != 0);
	assert(nDataLength <= s_aStorSize[tStore]);

	uint32_t nOffset = GetStoreOffset(tStore);
	uint8_t *src = (uint8_t *) &m_aSpiFlashData[nOffset];
	uint8_t *dst = (uint8_t *) pData;

	for (uint32_t i = 0; i < nDataLength; i++) {
		*dst = *src;
		dst++;
		src++;
	}

	DEBUG_EXIT
}

void SpiFlashStore::Dump(void) {
#ifndef NDEBUG
	for (uint32_t j = 0; j < STORE_LAST; j++) {
		printf("Store %d:%d\n", j, s_aStorUsedSize[j]);

		uint8_t *p = (uint8_t *) (&m_aSpiFlashData[GetStoreOffset((enum TStore) j)]);
		debug_dump(p, s_aStorSize[j]);

		printf("\n");
	}

	printf("m_tState=%d\n", m_tState);
#endif
}

bool SpiFlashStore::Flash(void) {
	assert(m_nStartAddress != 0);

	if (__builtin_expect((m_tState == STATE_IDLE),1)) {
		return false;
	}

	DEBUG_PRINTF("m_tState=%d", m_tState);

	switch (m_tState) {
		case STATE_CHANGED:
			spi_flash_cmd_erase(m_nStartAddress, (size_t) SPI_FLASH_STORE_DATA);
			m_tState = STATE_ERASED;
			return true;
			break;
		case STATE_ERASED:
			spi_flash_cmd_write_multi(m_nStartAddress, (size_t) SPI_FLASH_STORE_DATA, (const void *)&m_aSpiFlashData);
			m_tState = STATE_IDLE;
			break;
		default:
			break;
	}

	return false;
}
