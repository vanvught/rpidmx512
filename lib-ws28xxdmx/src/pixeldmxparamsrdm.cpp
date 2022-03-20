/**
 * @file pixeldmxparamsdmx.cpp
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <cassert>

#include "pixeltype.h"
#include "lightset.h"

#include "pixeldmxparamsrdm.h"

#include "ws28xxdmxparams.h"
#include "ws28xxdmxstore.h"

#include "debug.h"

using namespace pixeldmx::paramsdmx;

WS28xxDmxStore *PixelDmxParamsRdm::s_pWS28xxDmxStore;
uint8_t PixelDmxParamsRdm::s_Data;

PixelDmxParamsRdm::PixelDmxParamsRdm(WS28xxDmxStore *pWS28xxDmxStore) {
	DEBUG_ENTRY

	s_pWS28xxDmxStore = pWS28xxDmxStore;

	DEBUG_EXIT
}

void PixelDmxParamsRdm::Start(__attribute__((unused)) uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex == 0);

	DEBUG_EXIT
}

void PixelDmxParamsRdm::Stop(__attribute__((unused)) uint32_t nPortIndex) {
	DEBUG_ENTRY
	assert(nPortIndex == 0);

	DEBUG_EXIT
}

void PixelDmxParamsRdm::SetData(__attribute__((unused)) uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
	assert(nPortIndex == 0);

	if (nLength < DMX_FOOTPRINT) {
		return;
	}

	/*
	 * Slot 1: nType
	 * Slot 2: nCount;
	 * Slot 3: nGroupingCount
	 * Slot 4: nMap;
	 * Slot 5: nTestPattern
	 * Slot 6: Program;
	 */

	assert(DMX_FOOTPRINT == 6);

	const auto nLastIndex = DMX_FOOTPRINT - 1U;

	if (pData[nLastIndex] == 0x00) {
		s_Data = 0x00;
	} else {
		if ((pData[nLastIndex] == 0xFF) && (s_Data == 0x00)) {
			DEBUG_PUTS("Program");
			s_Data = 0xFF;

			auto nData = pData[static_cast<uint32_t>(SlotInfo::TYPE)];
			auto nUndefined = static_cast<uint8_t>(pixel::Type::UNDEFINED);
			s_pWS28xxDmxStore->SaveType(nData < nUndefined ? nData : nUndefined);

			// Validation takes place in class PixelDmxConfiguration
			s_pWS28xxDmxStore->SaveCount(pData[static_cast<uint32_t>(SlotInfo::COUNT)]);
			s_pWS28xxDmxStore->SaveGroupingCount(pData[static_cast<uint32_t>(SlotInfo::GROUPING_COUNT)]);

			nData = pData[static_cast<uint32_t>(SlotInfo::MAP)];
			nUndefined = static_cast<uint8_t>(pixel::Map::UNDEFINED);
			s_pWS28xxDmxStore->SaveMap(nData < nUndefined ? nData : nUndefined);

			s_pWS28xxDmxStore->SaveTestPattern(pData[static_cast<uint32_t>(SlotInfo::TEST_PATTERN)]);
		}
	}

	if ((pData[nLastIndex] == 0x00) || (pData[nLastIndex] == 0xFF)) {
		Display(pData);
	}
}

bool PixelDmxParamsRdm::GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &slotInfo) {
	if (nSlotOffset >= DMX_FOOTPRINT) {
		return false;
	}

	slotInfo.nType = 0x00;			// ST_PRIMARY
	slotInfo.nCategory = 0xFFFF;	// SD_UNDEFINED;

	return true;
}

void PixelDmxParamsRdm::Display(__attribute__((unused)) const uint8_t *pData) {
	// Weak
}
