/**
 * @file rdmslotinfo.cpp
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

#include "rdmslotinfo.h"
#include "pixeldmxparamsrdm.h"

using namespace pixeldmx::paramsdmx;

const char *RDMSlotInfo::GetCategoryTextUndefined(uint16_t nSlotOffset, uint32_t& nLength) {
	switch (static_cast<SlotInfo>(nSlotOffset)) {
	case SlotInfo::TYPE:
		nLength = 4;
		return "Type";
		break;
	case SlotInfo::COUNT:
		nLength = 5;
		return "Count";
		break;
	case SlotInfo::GROUPING_COUNT:
		nLength = 14;
		return "Grouping Count";
		break;
	case SlotInfo::MAP:
		nLength = 3;
		return "Map";
		break;
	case SlotInfo::TEST_PATTERN:
		nLength = 12;
		return "Test Pattern";
		break;
	case SlotInfo::PROGRAM:
		nLength = 7;
		return "Program";
		break;
	default:
		break;
	}

	nLength = 9;
	return "Undefined";
}
