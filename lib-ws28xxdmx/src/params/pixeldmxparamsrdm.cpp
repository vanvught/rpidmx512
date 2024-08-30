/**
 * @file pixeldmxparamsdmx.cpp
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#if defined (DEBUG_PIXELDMX)
# undef NDEBUG
#endif

#include <cstdint>
#include <cassert>

#include "pixeldmxparamsrdm.h"
#include "pixeldmxstore.h"
#include "pixeltype.h"

#include "lightset.h"

#include "debug.h"

void PixelDmxParamsRdm::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, [[maybe_unused]] const bool doUpdate) {
	assert(nPortIndex == 0);

	if (nLength < pixeldmx::paramsdmx::DMX_FOOTPRINT) {
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

	assert(pixeldmx::paramsdmx::DMX_FOOTPRINT == 6);

	const auto nLastIndex = pixeldmx::paramsdmx::DMX_FOOTPRINT - 1U;

	if (pData[nLastIndex] == 0x00) {
		m_Data = 0x00;
	} else {
		if ((pData[nLastIndex] == 0xFF) && (m_Data == 0x00)) {
			DEBUG_PUTS("Program");
			m_Data = 0xFF;

			auto nData = pData[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotInfo::TYPE)];
			auto nUndefined = static_cast<uint8_t>(pixel::Type::UNDEFINED);
			const auto nType = nData < nUndefined ? nData : nUndefined;
			PixelDmxStore::SaveType(nType);

			// Validation takes place in class PixelDmxConfiguration
			PixelDmxStore::SaveCount(pData[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotInfo::COUNT)]);
			PixelDmxStore::SaveGroupingCount(pData[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotInfo::GROUPING_COUNT)]);

			nData = pData[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotInfo::MAP)];
			nUndefined = static_cast<uint8_t>(pixel::Map::UNDEFINED);
			const auto nMap = nData < nUndefined ? nData : nUndefined;
			PixelDmxStore::SaveMap(nMap);

			PixelDmxStore::SaveTestPattern(pData[static_cast<uint32_t>(pixeldmx::paramsdmx::SlotInfo::TEST_PATTERN)]);
		}
	}

	if ((pData[nLastIndex] == 0x00) || (pData[nLastIndex] == 0xFF)) {
		Display(pData);
	}
}

void PixelDmxParamsRdm::Display([[maybe_unused]] const uint8_t *pData) {
	// Weak
}
