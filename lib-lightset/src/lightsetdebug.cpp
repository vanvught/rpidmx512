/**
 * @file lightsetdebug.cpp
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
#include <stdio.h>
#include <assert.h>

#include "lightsetdebug.h"

#define DMX_FOOTPRINT	16

LightSetDebug::LightSetDebug(void): m_bIsStarted(false), m_nDmxStartAddress(1) {
}

LightSetDebug::~LightSetDebug(void) {
}

uint16_t LightSetDebug::GetDmxFootprint(void) {
	printf("LightSetDebug::GetDmxFootprint(void)\n");
	return DMX_FOOTPRINT;
}

bool LightSetDebug::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	printf("LightSetDebug::SetDmxStartAddress(%d)\n", (int) nDmxStartAddress);

	if (nDmxStartAddress  > (512 - DMX_FOOTPRINT)) {
		return false;
	}

	m_nDmxStartAddress = nDmxStartAddress;
	return true;
}

uint16_t LightSetDebug::GetDmxStartAddress(void) {
	printf("LightSetDebug::GetDmxStartAddress(void)\n");
	return m_nDmxStartAddress;
}

bool LightSetDebug::GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) {
	if (nSlotOffset > DMX_FOOTPRINT) {
		return false;
	}

	tSlotInfo.nType = 0x00; // ST_PRIMARY
	tSlotInfo.nCategory = 0x0001; // SD_INTENSITY

	return true;
}

void LightSetDebug::Start(uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	printf("LightSetDebug::Start(void)\n");
}

void LightSetDebug::Stop(uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;

	printf("LightSetDebug::Stop(void)\n");
}

void LightSetDebug::SetData(uint8_t nPort, const uint8_t* pData, uint16_t nLength) {
	assert(pData != 0);
	assert(nLength <= 512);

	printf("LightSetDebug::SetData(nPort:%d, *pData:%p, nLength:%d)\n", (int) nPort, (void *) pData, (int) nLength);
	printf("%d:%d:%d: ", (int) nLength, (int) DMX_FOOTPRINT, (int) m_nDmxStartAddress);

	for (uint16_t i = m_nDmxStartAddress - 1, j = 0; (i < nLength) && (j < DMX_FOOTPRINT); i++, j++) {
		printf("%.2x ", pData[i]);
	}

	printf("\n");
}

