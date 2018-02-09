/**
 * @file lightset.cpp
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "lightset.h"

LightSet::~LightSet(void) {

}

uint16_t LightSet::GetDmxStartAddress(void) {
	return 1;
}

uint16_t LightSet::GetDmxFootprint(void) {
	return 512;
}

bool LightSet::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	return false;
}

bool LightSet::GetSlotInfo(uint16_t nSlot, struct TLightSetSlotInfo &tSlotInfo) {
	tSlotInfo.nType = 0x00; // ST_PRIMARY
	tSlotInfo.nCategory = 0x0001; // SD_INTENSITY

	return true;
}
