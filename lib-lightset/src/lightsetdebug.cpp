/**
 * @file lightsetdebug.cpp
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
#include <stdio.h>
#include <cassert>

#include "lightsetdebug.h"

#include "debug.h"

#define DMX_FOOTPRINT	16

LightSetDebug::LightSetDebug() {
}

uint16_t LightSetDebug::GetDmxFootprint() {
	DEBUG_PRINTF("DMX_FOOTPRINT=%d", DMX_FOOTPRINT);
	return DMX_FOOTPRINT;
}

bool LightSetDebug::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	DEBUG_PRINTF("nDmxStartAddress=%d", static_cast<int>(nDmxStartAddress));

	if (nDmxStartAddress > (512 - DMX_FOOTPRINT)) {
		return false;
	}

	m_nDmxStartAddress = nDmxStartAddress;
	return true;
}

uint16_t LightSetDebug::GetDmxStartAddress() {
	DEBUG_PRINTF("m_nDmxStartAddress=%d", m_nDmxStartAddress);
	return m_nDmxStartAddress;
}

bool LightSetDebug::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) {
	DEBUG_ENTRY

	if (nSlotOffset > DMX_FOOTPRINT) {
		DEBUG_EXIT
		return false;
	}

	tSlotInfo.nType = 0x00; // ST_PRIMARY
	tSlotInfo.nCategory = 0x0001; // SD_INTENSITY

	DEBUG_EXIT
	return true;
}

void LightSetDebug::Start(__attribute__((unused)) uint8_t nPort) {
	printf("%s:%s\n", __FILE__, __FUNCTION__);

	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;
}

void LightSetDebug::Stop(__attribute__((unused)) uint8_t nPort) {
	printf("%s:%s\n", __FILE__, __FUNCTION__);

	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void LightSetDebug::SetData(uint8_t nPort, const uint8_t* pData, uint16_t nLength) {
	assert(pData != nullptr);
	assert(nLength <= 512);

	printf("%s:%s(%d, %p, %d)\n", __FILE__, __FUNCTION__, nPort, reinterpret_cast<const void *>(pData), nLength);
	printf("%d:%d: ", DMX_FOOTPRINT, m_nDmxStartAddress);

	for (uint32_t i = static_cast<uint32_t>(m_nDmxStartAddress - 1), j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		printf("%.2x ", pData[i]);
	}

	printf("\n");
}
