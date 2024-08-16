/**
 * @file display.cpp
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

#include "pixeltype.h"
#include "pixelpatterns.h"
#include "pixeldmxparamsrdm.h"

#include "displayudf.h"

using namespace pixeldmx::paramsdmx;

static constexpr auto nLastIndex = DMX_FOOTPRINT - 1U;
static bool s_IsProgrammed;

void PixelDmxParamsRdm::Display(const uint8_t *pData)  {
	if (pData[static_cast<uint32_t>(SlotInfo::TEST_PATTERN)] == 0x00) {
		DisplayUdf::Get()->ClearLine(6);
	} else {
		DisplayUdf::Get()->Printf(6, "%-20s",
				PixelPatterns::GetName(static_cast<pixelpatterns::Pattern>(pData[static_cast<uint32_t>(SlotInfo::TEST_PATTERN)])));
	}

	DisplayUdf::Get()->Printf(7, "%-8s %-2d G%-2d %-5s",
			pixel::pixel_get_type(static_cast<pixel::Type>(pData[static_cast<uint32_t>(SlotInfo::TYPE)])),
			pData[static_cast<uint32_t>(SlotInfo::COUNT)],
			pData[static_cast<uint32_t>(SlotInfo::GROUPING_COUNT)],
			pixel::pixel_get_map(static_cast<pixel::Map>(pData[static_cast<uint32_t>(SlotInfo::MAP)])));

	if (pData[nLastIndex] == 0xFF) {
		if (!s_IsProgrammed) {
			s_IsProgrammed = true;
			DisplayUdf::Get()->TextStatus("Programmed");
		}
	} else {
		s_IsProgrammed = false	;
		DisplayUdf::Get()->ClearLine(8);
	}
}
